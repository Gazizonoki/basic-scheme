#include "object.h"

#include <cmath>
#include <random>

void Scope::CreateGlobalScope() {
    functions_["+"] = std::make_shared<SumFunction>();
    functions_["-"] = std::make_shared<SubtractFunction>();
    functions_["*"] = std::make_shared<MultiplyFunction>();
    functions_["/"] = std::make_shared<DivideFunction>();
    functions_["max"] = std::make_shared<MaxFunction>();
    functions_["min"] = std::make_shared<MinFunction>();
    functions_["abs"] = std::make_shared<AbsFunction>();
    functions_["number?"] = std::make_shared<IsNumberFunction>();
    functions_["quote"] = std::make_shared<QuoteFunction>();
    functions_["="] = std::make_shared<EqualFunction>();
    functions_["<"] = std::make_shared<LessFunction>();
    functions_[">"] = std::make_shared<GreaterFunction>();
    functions_["<="] = std::make_shared<LessOrEqualFunction>();
    functions_[">="] = std::make_shared<GreaterOrEqualFunction>();
    functions_["boolean?"] = std::make_shared<IsBoolFunction>();
    functions_["and"] = std::make_shared<AndFunction>();
    functions_["or"] = std::make_shared<OrFunction>();
    functions_["not"] = std::make_shared<NotFunction>();
    functions_["null?"] = std::make_shared<IsNullFunction>();
    functions_["pair?"] = std::make_shared<IsPairFunction>();
    functions_["list?"] = std::make_shared<IsListFunction>();
    functions_["symbol?"] = std::make_shared<IsSymbolFunction>();
    functions_["cons"] = std::make_shared<ConstructPairFunction>();
    functions_["car"] = std::make_shared<GetFirstElementFunction>();
    functions_["cdr"] = std::make_shared<GetSecondElementFunction>();
    functions_["list"] = std::make_shared<ConstructListFunction>();
    functions_["list-ref"] = std::make_shared<GetElementFunction>();
    functions_["list-tail"] = std::make_shared<GetTailFunction>();
    functions_["if"] = std::make_shared<IfFunction>();
    functions_["define"] = std::make_shared<DefineFunction>();
    functions_["set!"] = std::make_shared<SetFunction>();
    functions_["set-car!"] = std::make_shared<SetFirstFunction>();
    functions_["set-cdr!"] = std::make_shared<SetSecondFunction>();
    functions_["lambda"] = std::make_shared<LambdaFunction>();
    global_scope_ = this->shared_from_this();
}

std::shared_ptr<Object> Scope::CallFunction(std::shared_ptr<Object> func,
                                            std::shared_ptr<Object> object,
                                            std::shared_ptr<Scope> scope) {
    if (!Is<Symbol>(func) && !Is<FunctionObject>(func)) {
        throw RuntimeError("Invalid function name");
    }
    if (Is<FunctionObject>(func)) {
        if (object && object->object_scope) {
            scope = object->object_scope;
        }
        return As<FunctionObject>(func)->GetFunction()->Execute(object, scope);
    }
    if (functions_.contains(As<Symbol>(func)->GetName())) {
        if (object && object->object_scope) {
            scope = object->object_scope;
        }
        return functions_[As<Symbol>(func)->GetName()]->Execute(object, scope);
    }
    if (!parent_scope_) {
        throw NameError("Unknown function: " + As<Symbol>(func)->GetName());
    }
    return parent_scope_->CallFunction(func, object, scope);
}

std::shared_ptr<Object> Scope::GetVariable(const std::string& name) {
    if (variables_.contains(name)) {
        return variables_[name];
    }
    if (!parent_scope_) {
        throw NameError("Unknown variable: " + name);
    }
    return parent_scope_->GetVariable(name);
}

std::shared_ptr<Object> Scope::AddVariable(const std::string& name,
                                           std::shared_ptr<Object> variable) {
    return variables_[name] = variable;
}

std::shared_ptr<Object> Scope::UpdateVariable(const std::string& name,
                                              std::shared_ptr<Object> variable) {
    if (variables_.contains(name)) {
        return variables_[name] = variable;
    }
    if (!parent_scope_) {
        throw NameError("Unknown variable: " + name);
    }
    return parent_scope_->UpdateVariable(name, variable);
}

std::shared_ptr<Object> Scope::AddFunction(const std::string& name,
                                           std::shared_ptr<IFunction> func) {
    functions_[name] = func;
    global_scope_->all_functions_.insert(name);
    return std::make_shared<Symbol>(name);
}

void Scope::AddParentScope(std::shared_ptr<Scope> parent_scope) {
    parent_scope_ = parent_scope;
    global_scope_ = parent_scope->global_scope_;
}

std::shared_ptr<IFunction> Scope::GetFunction(const std::string& name) {
    if (functions_.contains(name)) {
        return functions_[name];
    }
    if (!parent_scope_) {
        throw NameError("Unknown function: " + name);
    }
    return parent_scope_->GetFunction(name);
}

bool Scope::IsFunctionExists(const std::string& name) {
    return global_scope_->all_functions_.contains(name);
}

std::shared_ptr<Symbol> BoolToSymbol(bool statement) {
    if (statement) {
        return std::make_shared<Symbol>("#t");
    }
    return std::make_shared<Symbol>("#f");
}

bool ObjectToBool(std::shared_ptr<Object> object) {
    if (!Is<Symbol>(object) || As<Symbol>(object)->GetName() != "#f") {
        return true;
    }
    return false;
};

bool IsBool(std::shared_ptr<Object> object) {
    return Is<Symbol>(object) &&
           (As<Symbol>(object)->GetName() == "#t" || As<Symbol>(object)->GetName() == "#f");
}

std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> object, std::shared_ptr<Scope> scope) {
    if (object) {
        if (object->object_scope) {
            scope = object->object_scope;
        }
        return object->Evaluate(scope);
    }
    throw RuntimeError("Bad list");
}

std::shared_ptr<Object> UserFunction::Execute(std::shared_ptr<Object> object,
                                              std::shared_ptr<Scope> scope) {
    std::vector<std::shared_ptr<Object>> input_args;
    while (object) {
        if (!Is<Cell>(object)) {
            throw RuntimeError("Bad list");
        }
        input_args.push_back(As<Cell>(object)->GetFirst());
        object = As<Cell>(object)->GetSecond();
    }
    if (input_args.size() != args_.size()) {
        throw RuntimeError("Invalid argument count");
    }

    auto new_scope = std::make_shared<Scope>();
    new_scope->AddParentScope(parent_scope_);

    for (size_t i = 0; i < args_.size(); ++i) {
        new_scope->AddVariable(args_[i], Evaluate(input_args[i], scope));
    }
    for (size_t i = 0; i + 1 < executables_.size(); ++i) {
        Evaluate(executables_[i], new_scope);
    }
    auto ans = Evaluate(executables_.back(), new_scope);
    ans->object_scope = new_scope;
    return ans;
}

std::shared_ptr<Object> ArithmeticFunction::Execute(std::shared_ptr<Object> object,
                                                    std::shared_ptr<Scope> scope) {
    int64_t ans = 0;
    bool is_first = true;
    std::shared_ptr<Object> lhs;
    while (Is<Cell>(object)) {
        lhs = Evaluate(As<Cell>(object)->GetFirst(), scope);
        if (!Is<Number>(lhs)) {
            throw RuntimeError("Bad list");
        }
        if (is_first) {
            ans = As<Number>(lhs)->GetValue();
        } else {
            ans = Operation(ans, As<Number>(lhs)->GetValue());
        }
        is_first = false;
        object = As<Cell>(object)->GetSecond();
    }
    if (object) {
        throw RuntimeError("Bad list");
    }
    if (is_first) {
        return std::make_shared<Number>(GetDefaultValue());
    }
    return std::make_shared<Number>(ans);
}

std::shared_ptr<Object> BooleanFunction::Execute(std::shared_ptr<Object> object,
                                                 std::shared_ptr<Scope> scope) {
    std::shared_ptr<Object> lhs, prev = BoolToSymbol(GetDefaultValue());
    while (Is<Cell>(object)) {
        lhs = Evaluate(As<Cell>(object)->GetFirst(), scope);
        if (ObjectToBool(lhs) != GetDefaultValue()) {
            return lhs;
        }
        prev = lhs;
        object = As<Cell>(object)->GetSecond();
    }
    if (object) {
        throw RuntimeError("Bad list");
    }
    return prev;
}

std::shared_ptr<Object> ComparisonFunction::Execute(std::shared_ptr<Object> object,
                                                    std::shared_ptr<Scope> scope) {
    bool ans = true, is_first = true;
    int64_t prev;
    std::shared_ptr<Object> lhs;
    while (Is<Cell>(object)) {
        lhs = Evaluate(As<Cell>(object)->GetFirst(), scope);
        if (!Is<Number>(lhs)) {
            throw RuntimeError("Bad list");
        }
        if (!is_first) {
            ans = (ans && Compare(prev, As<Number>(lhs)->GetValue()));
        }
        prev = As<Number>(lhs)->GetValue();
        is_first = false;
        object = As<Cell>(object)->GetSecond();
    }
    if (object) {
        throw RuntimeError("Bad list");
    }
    return BoolToSymbol(ans);
}

std::shared_ptr<Object> OneArgumentFunction::Execute(std::shared_ptr<Object> object,
                                                     std::shared_ptr<Scope> scope) {
    if (!Is<Cell>(object) || As<Cell>(object)->GetSecond()) {
        throw RuntimeError("Invalid argument");
    }
    return Function(As<Cell>(object)->GetFirst(), scope);
}

std::shared_ptr<Object> AbsFunction::Function(std::shared_ptr<Object> object,
                                              std::shared_ptr<Scope> scope) {
    object = Evaluate(object, scope);
    if (!Is<Number>(object)) {
        throw RuntimeError("Invalid argument");
    }
    return std::make_shared<Number>(std::abs(As<Number>(object)->GetValue()));
}

std::shared_ptr<Object> IsNumberFunction::Function(std::shared_ptr<Object> object,
                                                   std::shared_ptr<Scope> scope) {
    return BoolToSymbol(Is<Number>(Evaluate(object, scope)));
}

std::shared_ptr<Object> IsBoolFunction::Function(std::shared_ptr<Object> object,
                                                 std::shared_ptr<Scope> scope) {
    return BoolToSymbol(IsBool(Evaluate(object, scope)));
}

std::shared_ptr<Object> IsNullFunction::Function(std::shared_ptr<Object> object,
                                                 std::shared_ptr<Scope> scope) {
    return BoolToSymbol(!Evaluate(object, scope));
}

std::shared_ptr<Object> IsPairFunction::Function(std::shared_ptr<Object> object,
                                                 std::shared_ptr<Scope> scope) {
    object = Evaluate(object, scope);
    return BoolToSymbol(Is<Cell>(object));
}

std::shared_ptr<Object> IsListFunction::Function(std::shared_ptr<Object> object,
                                                 std::shared_ptr<Scope> scope) {
    object = Evaluate(object, scope);
    while (Is<Cell>(object)) {
        object = As<Cell>(object)->GetSecond();
    }
    return BoolToSymbol(!object);
}

std::shared_ptr<Object> IsSymbolFunction::Function(std::shared_ptr<Object> object,
                                                   std::shared_ptr<Scope> scope) {
    object = Evaluate(object, scope);
    return BoolToSymbol(Is<Symbol>(object) && !IsBool(object));
}

std::shared_ptr<Object> QuoteFunction::Function(std::shared_ptr<Object> object,
                                                std::shared_ptr<Scope>) {
    return object;
}

std::shared_ptr<Object> NotFunction::Function(std::shared_ptr<Object> object,
                                              std::shared_ptr<Scope> scope) {
    return BoolToSymbol(!ObjectToBool(Evaluate(object, scope)));
}

std::shared_ptr<Object> GetFirstElementFunction::Function(std::shared_ptr<Object> object,
                                                          std::shared_ptr<Scope> scope) {
    object = Evaluate(object, scope);
    if (!Is<Cell>(object)) {
        throw RuntimeError("Invalid argument");
    }
    auto ans = As<Cell>(object)->GetFirst();
    if (ans && object->object_scope) {
        ans->object_scope = object->object_scope;
    }
    return ans;
}

std::shared_ptr<Object> GetSecondElementFunction::Function(std::shared_ptr<Object> object,
                                                           std::shared_ptr<Scope> scope) {
    object = Evaluate(object, scope);
    if (!Is<Cell>(object)) {
        throw RuntimeError("Invalid argument");
    }
    auto ans = As<Cell>(object)->GetSecond();
    if (ans && object->object_scope) {
        ans->object_scope = object->object_scope;
    }
    return ans;
}

std::shared_ptr<Object> GetElementFunction::Execute(std::shared_ptr<Object> object,
                                                    std::shared_ptr<Scope> scope) {
    if (!Is<Cell>(object) || !Is<Cell>(As<Cell>(object)->GetSecond()) ||
        As<Cell>(As<Cell>(object)->GetSecond())->GetSecond() ||
        !Is<Number>(As<Cell>(As<Cell>(object)->GetSecond())->GetFirst())) {
        throw RuntimeError("Invalid argument");
    }
    std::shared_ptr<Object> list = Evaluate(As<Cell>(object)->GetFirst(), scope);
    size_t index = As<Number>(As<Cell>(As<Cell>(object)->GetSecond())->GetFirst())->GetValue();
    while (Is<Cell>(list) && index > 0) {
        list = As<Cell>(list)->GetSecond();
        --index;
    }
    if (!Is<Cell>(list)) {
        throw RuntimeError("Out of bounds");
    }
    return As<Cell>(list)->GetFirst();
}

std::shared_ptr<Object> GetTailFunction::Execute(std::shared_ptr<Object> object,
                                                 std::shared_ptr<Scope> scope) {
    if (!Is<Cell>(object) || !Is<Cell>(As<Cell>(object)->GetSecond()) ||
        As<Cell>(As<Cell>(object)->GetSecond())->GetSecond() ||
        !Is<Number>(As<Cell>(As<Cell>(object)->GetSecond())->GetFirst())) {
        throw RuntimeError("Invalid argument");
    }
    std::shared_ptr<Object> list = Evaluate(As<Cell>(object)->GetFirst(), scope);
    size_t index = As<Number>(As<Cell>(As<Cell>(object)->GetSecond())->GetFirst())->GetValue();
    while (Is<Cell>(list) && index > 0) {
        list = As<Cell>(list)->GetSecond();
        --index;
    }
    if (index > 0) {
        throw RuntimeError("Out of bounds");
    }
    return list;
}

std::shared_ptr<Object> ConstructPairFunction::Execute(std::shared_ptr<Object> object,
                                                       std::shared_ptr<Scope>) {
    if (!Is<Cell>(object) || !Is<Cell>(As<Cell>(object)->GetSecond()) ||
        As<Cell>(As<Cell>(object)->GetSecond())->GetSecond()) {
        throw RuntimeError("Invalid argument");
    }
    return std::make_shared<Cell>(As<Cell>(object)->GetFirst(),
                                  As<Cell>(As<Cell>(object)->GetSecond())->GetFirst());
}

std::shared_ptr<Object> ConstructListFunction::Execute(std::shared_ptr<Object> object,
                                                       std::shared_ptr<Scope>) {
    return object;
}

std::shared_ptr<Object> IfFunction::Execute(std::shared_ptr<Object> object,
                                            std::shared_ptr<Scope> scope) {
    if (!Is<Cell>(object) || !Is<Cell>(As<Cell>(object)->GetSecond()) ||
        (As<Cell>(As<Cell>(object)->GetSecond())->GetSecond() &&
         (!Is<Cell>(As<Cell>(As<Cell>(object)->GetSecond())->GetSecond()) ||
          As<Cell>(As<Cell>(As<Cell>(object)->GetSecond())->GetSecond())->GetSecond()))) {
        throw SyntaxError("Invalid argument");
    }
    if (ObjectToBool(Evaluate(As<Cell>(object)->GetFirst(), scope))) {
        return Evaluate(As<Cell>(As<Cell>(object)->GetSecond())->GetFirst(), scope);
    }
    if (!As<Cell>(As<Cell>(object)->GetSecond())->GetSecond()) {
        return nullptr;
    }
    return Evaluate(As<Cell>(As<Cell>(As<Cell>(object)->GetSecond())->GetSecond())->GetFirst(),
                    scope);
}

std::shared_ptr<Object> DefineFunction::Execute(std::shared_ptr<Object> object,
                                                std::shared_ptr<Scope> scope) {
    if (!Is<Cell>(object)) {
        throw SyntaxError("Invalid argument");
    }
    if (!Is<Symbol>(As<Cell>(object)->GetFirst())) {
        std::shared_ptr<Object> list = As<Cell>(object)->GetFirst();
        if (!Is<Cell>(list) || !Is<Symbol>(As<Cell>(list)->GetFirst())) {
            throw SyntaxError("Invalid argument");
        }
        std::string name = As<Symbol>(As<Cell>(list)->GetFirst())->GetName();
        std::vector<std::string> args;
        list = As<Cell>(list)->GetSecond();
        while (list) {
            if (!Is<Cell>(list) || !Is<Symbol>(As<Cell>(list)->GetFirst())) {
                throw SyntaxError("Invalid argument");
            }
            args.push_back(As<Symbol>(As<Cell>(list)->GetFirst())->GetName());
            list = As<Cell>(list)->GetSecond();
        }
        object = As<Cell>(object)->GetSecond();
        std::vector<std::shared_ptr<Object>> executables;
        while (object) {
            if (!Is<Cell>(object)) {
                throw SyntaxError("Invalid argument");
            }
            executables.push_back(As<Cell>(object)->GetFirst());
            object = As<Cell>(object)->GetSecond();
        }
        if (executables.empty()) {
            throw SyntaxError("Lambda should have at least 1 expression");
        }
        return scope->AddFunction(name, std::make_shared<UserFunction>(args, executables, scope));
    }
    if (!Is<Cell>(As<Cell>(object)->GetSecond()) ||
        As<Cell>(As<Cell>(object)->GetSecond())->GetSecond()) {
        throw SyntaxError("Invalid argument");
    }
    std::shared_ptr<Object> second =
        Evaluate(As<Cell>(As<Cell>(object)->GetSecond())->GetFirst(), scope);
    if (Is<FunctionObject>(second)) {
        return scope->AddFunction(As<Symbol>(As<Cell>(object)->GetFirst())->GetName(),
                                  As<FunctionObject>(second)->GetFunction());
    }
    return scope->AddVariable(As<Symbol>(As<Cell>(object)->GetFirst())->GetName(),
                              Evaluate(As<Cell>(As<Cell>(object)->GetSecond())->GetFirst(), scope));
}

std::shared_ptr<Object> SetFunction::Execute(std::shared_ptr<Object> object,
                                             std::shared_ptr<Scope> scope) {
    if (!Is<Cell>(object) || !Is<Cell>(As<Cell>(object)->GetSecond()) ||
        As<Cell>(As<Cell>(object)->GetSecond())->GetSecond() ||
        !Is<Symbol>(As<Cell>(object)->GetFirst())) {
        throw SyntaxError("Invalid argument");
    }
    return scope->UpdateVariable(
        As<Symbol>(As<Cell>(object)->GetFirst())->GetName(),
        Evaluate(As<Cell>(As<Cell>(object)->GetSecond())->GetFirst(), scope));
}

std::shared_ptr<Object> SetFirstFunction::Execute(std::shared_ptr<Object> object,
                                                  std::shared_ptr<Scope> scope) {
    if (!Is<Cell>(object) || !Is<Cell>(As<Cell>(object)->GetSecond()) ||
        As<Cell>(As<Cell>(object)->GetSecond())->GetSecond()) {
        throw SyntaxError("Invalid argument");
    }
    auto pair = Evaluate(As<Cell>(object)->GetFirst(), scope);
    if (!Is<Cell>(pair)) {
        throw SyntaxError("Invalid argument");
    }
    As<Cell>(pair)->GetFirst() =
        Evaluate(As<Cell>(As<Cell>(object)->GetSecond())->GetFirst(), scope);
    return pair;
}

std::shared_ptr<Object> SetSecondFunction::Execute(std::shared_ptr<Object> object,
                                                   std::shared_ptr<Scope> scope) {
    if (!Is<Cell>(object) || !Is<Cell>(As<Cell>(object)->GetSecond()) ||
        As<Cell>(As<Cell>(object)->GetSecond())->GetSecond()) {
        throw SyntaxError("Invalid argument");
    }
    auto pair = Evaluate(As<Cell>(object)->GetFirst(), scope);
    if (!Is<Cell>(pair)) {
        throw SyntaxError("Invalid argument");
    }
    As<Cell>(pair)->GetSecond() =
        Evaluate(As<Cell>(As<Cell>(object)->GetSecond())->GetFirst(), scope);
    return pair;
}

std::shared_ptr<Object> LambdaFunction::Execute(std::shared_ptr<Object> object,
                                                std::shared_ptr<Scope> scope) {
    if (!Is<Cell>(object)) {
        throw SyntaxError("Invalid argument");
    }
    std::shared_ptr<Object> list = As<Cell>(object)->GetFirst();
    std::vector<std::string> args;
    while (list) {
        if (!Is<Cell>(list) || !Is<Symbol>(As<Cell>(list)->GetFirst())) {
            throw SyntaxError("Invalid argument");
        }
        args.push_back(As<Symbol>(As<Cell>(list)->GetFirst())->GetName());
        list = As<Cell>(list)->GetSecond();
    }
    object = As<Cell>(object)->GetSecond();
    std::vector<std::shared_ptr<Object>> executables;
    while (object) {
        if (!Is<Cell>(object)) {
            throw SyntaxError("Invalid argument");
        }
        executables.push_back(As<Cell>(object)->GetFirst());
        object = As<Cell>(object)->GetSecond();
    }
    if (executables.empty()) {
        throw SyntaxError("Lambda should have at least 1 expression");
    }
    return std::make_shared<FunctionObject>(
        std::make_shared<UserFunction>(args, executables, scope));
}

Cell::Cell(std::shared_ptr<Object> first, std::shared_ptr<Object> second)
    : first_(first), second_(second) {
}

std::shared_ptr<Object> Cell::Evaluate(std::shared_ptr<Scope> scope) {
    auto lhs = first_;
    while (!Is<Number>(lhs) && !Is<Symbol>(lhs) && !Is<FunctionObject>(lhs)) {
        lhs = ::Evaluate(lhs, scope);
    }
    return scope->CallFunction(lhs, second_, scope);
}