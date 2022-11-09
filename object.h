#pragma once

#include <memory>
#include <utility>
#include <string>
#include <set>
#include <map>
#include <vector>

#include "error.h"

class Scope;

class Object : public std::enable_shared_from_this<Object> {
public:
    virtual ~Object() = default;
    virtual std::shared_ptr<Object> Evaluate(std::shared_ptr<Scope> scope) = 0;
    std::shared_ptr<Scope> object_scope;
};

template <class T>
std::shared_ptr<T> As(const std::shared_ptr<Object>& obj);

template <class T>
bool Is(const std::shared_ptr<Object>& obj);

std::shared_ptr<Object> Evaluate(std::shared_ptr<Object> object, std::shared_ptr<Scope> scope);

class IFunction : public std::enable_shared_from_this<IFunction> {
public:
    virtual std::shared_ptr<Object> Execute(std::shared_ptr<Object> object,
                                            std::shared_ptr<Scope> scope) = 0;
    virtual ~IFunction() = default;
};

class UserFunction : public IFunction {
public:
    UserFunction(const std::vector<std::string>& args,
                 const std::vector<std::shared_ptr<Object>>& executables,
                 std::shared_ptr<Scope> parent_scope)
        : args_(args), executables_(executables), parent_scope_(parent_scope) {
    }
    std::shared_ptr<Object> Execute(std::shared_ptr<Object> object,
                                    std::shared_ptr<Scope> scope) override;

private:
    std::vector<std::string> args_;
    std::vector<std::shared_ptr<Object>> executables_;
    std::shared_ptr<Scope> parent_scope_;
};

class Scope : public std::enable_shared_from_this<Scope> {
public:
    std::shared_ptr<Object> AddFunction(const std::string& name, std::shared_ptr<IFunction> func);
    std::shared_ptr<Object> AddVariable(const std::string& name, std::shared_ptr<Object> variable);
    std::shared_ptr<Object> UpdateVariable(const std::string& name,
                                           std::shared_ptr<Object> variable);
    void AddParentScope(std::shared_ptr<Scope> parent_scope);
    std::shared_ptr<Object> CallFunction(std::shared_ptr<Object> func,
                                         std::shared_ptr<Object> object,
                                         std::shared_ptr<Scope> scope);
    std::shared_ptr<Object> GetVariable(const std::string& name);
    std::shared_ptr<IFunction> GetFunction(const std::string& name);
    bool IsFunctionExists(const std::string& name);
    void CreateGlobalScope();

private:
    std::map<std::string, std::shared_ptr<Object>> variables_;
    std::map<std::string, std::shared_ptr<IFunction>> functions_;
    std::shared_ptr<Scope> parent_scope_, global_scope_;
    std::set<std::string> all_functions_;
};

class ArithmeticFunction : public IFunction {
public:
    std::shared_ptr<Object> Execute(std::shared_ptr<Object> object,
                                    std::shared_ptr<Scope> scope) override;

protected:
    virtual int64_t Operation(int64_t lhs, int64_t rhs) = 0;
    virtual int64_t GetDefaultValue() = 0;
};

class ComparisonFunction : public IFunction {
public:
    std::shared_ptr<Object> Execute(std::shared_ptr<Object> object,
                                    std::shared_ptr<Scope> scope) override;

protected:
    virtual int64_t Compare(int64_t lhs, int64_t rhs) = 0;
};

class BooleanFunction : public IFunction {
public:
    std::shared_ptr<Object> Execute(std::shared_ptr<Object> object,
                                    std::shared_ptr<Scope> scope) override;

protected:
    virtual bool GetDefaultValue() = 0;
};

class OneArgumentFunction : public IFunction {
public:
    std::shared_ptr<Object> Execute(std::shared_ptr<Object> object,
                                    std::shared_ptr<Scope> scope) override;

protected:
    virtual std::shared_ptr<Object> Function(std::shared_ptr<Object> object,
                                             std::shared_ptr<Scope> scope) = 0;
};

class AbsFunction : public OneArgumentFunction {
protected:
    std::shared_ptr<Object> Function(std::shared_ptr<Object> object,
                                     std::shared_ptr<Scope> scope) override;
};

class IsNumberFunction : public OneArgumentFunction {
protected:
    std::shared_ptr<Object> Function(std::shared_ptr<Object> object,
                                     std::shared_ptr<Scope> scope) override;
};

class IsBoolFunction : public OneArgumentFunction {
protected:
    std::shared_ptr<Object> Function(std::shared_ptr<Object> object,
                                     std::shared_ptr<Scope> scope) override;
};

class QuoteFunction : public OneArgumentFunction {
protected:
    std::shared_ptr<Object> Function(std::shared_ptr<Object> object,
                                     std::shared_ptr<Scope> scope) override;
};

class NotFunction : public OneArgumentFunction {
protected:
    std::shared_ptr<Object> Function(std::shared_ptr<Object> object,
                                     std::shared_ptr<Scope> scope) override;
};

class IsNullFunction : public OneArgumentFunction {
protected:
    std::shared_ptr<Object> Function(std::shared_ptr<Object> object,
                                     std::shared_ptr<Scope> scope) override;
};

class IsPairFunction : public OneArgumentFunction {
protected:
    std::shared_ptr<Object> Function(std::shared_ptr<Object> object,
                                     std::shared_ptr<Scope> scope) override;
};

class IsListFunction : public OneArgumentFunction {
protected:
    std::shared_ptr<Object> Function(std::shared_ptr<Object> object,
                                     std::shared_ptr<Scope> scope) override;
};

class IsSymbolFunction : public OneArgumentFunction {
private:
    std::shared_ptr<Object> Function(std::shared_ptr<Object> object,
                                     std::shared_ptr<Scope> scope) override;
};

class GetFirstElementFunction : public OneArgumentFunction {
protected:
    std::shared_ptr<Object> Function(std::shared_ptr<Object> object,
                                     std::shared_ptr<Scope> scope) override;
};

class GetSecondElementFunction : public OneArgumentFunction {
protected:
    std::shared_ptr<Object> Function(std::shared_ptr<Object> object,
                                     std::shared_ptr<Scope> scope) override;
};

class GetElementFunction : public IFunction {
public:
    std::shared_ptr<Object> Execute(std::shared_ptr<Object> object,
                                    std::shared_ptr<Scope> scope) override;
};

class GetTailFunction : public IFunction {
public:
    std::shared_ptr<Object> Execute(std::shared_ptr<Object> object,
                                    std::shared_ptr<Scope> scope) override;
};

class ConstructPairFunction : public IFunction {
public:
    std::shared_ptr<Object> Execute(std::shared_ptr<Object> object,
                                    std::shared_ptr<Scope> scope) override;
};

class ConstructListFunction : public IFunction {
public:
    std::shared_ptr<Object> Execute(std::shared_ptr<Object> object,
                                    std::shared_ptr<Scope> scope) override;
};

class IfFunction : public IFunction {
public:
    std::shared_ptr<Object> Execute(std::shared_ptr<Object> object,
                                    std::shared_ptr<Scope> scope) override;
};

class DefineFunction : public IFunction {
public:
    std::shared_ptr<Object> Execute(std::shared_ptr<Object> object,
                                    std::shared_ptr<Scope> scope) override;
};

class SetFunction : public IFunction {
public:
    std::shared_ptr<Object> Execute(std::shared_ptr<Object> object,
                                    std::shared_ptr<Scope> scope) override;
};

class SetFirstFunction : public IFunction {
public:
    std::shared_ptr<Object> Execute(std::shared_ptr<Object> object,
                                    std::shared_ptr<Scope> scope) override;
};

class SetSecondFunction : public IFunction {
public:
    std::shared_ptr<Object> Execute(std::shared_ptr<Object> object,
                                    std::shared_ptr<Scope> scope) override;
};

class LambdaFunction : public IFunction {
public:
    std::shared_ptr<Object> Execute(std::shared_ptr<Object> object,
                                    std::shared_ptr<Scope> scope) override;
};

class AndFunction : public BooleanFunction {
protected:
    bool GetDefaultValue() override {
        return true;
    }
};

class OrFunction : public BooleanFunction {
protected:
    bool GetDefaultValue() override {
        return false;
    }
};

class SumFunction : public ArithmeticFunction {
protected:
    int64_t Operation(int64_t lhs, int64_t rhs) override {
        return lhs + rhs;
    }
    int64_t GetDefaultValue() override {
        return 0;
    }
};

class SubtractFunction : public ArithmeticFunction {
protected:
    int64_t Operation(int64_t lhs, int64_t rhs) override {
        return lhs - rhs;
    }
    int64_t GetDefaultValue() override {
        throw RuntimeError("Subtract hasn't default value");
    }
};

class MultiplyFunction : public ArithmeticFunction {
protected:
    int64_t Operation(int64_t lhs, int64_t rhs) override {
        return lhs * rhs;
    }
    int64_t GetDefaultValue() override {
        return 1;
    }
};

class DivideFunction : public ArithmeticFunction {
protected:
    int64_t Operation(int64_t lhs, int64_t rhs) override {
        return lhs / rhs;
    }
    int64_t GetDefaultValue() override {
        throw RuntimeError("Divide hasn't default value");
    }
};

class MaxFunction : public ArithmeticFunction {
protected:
    int64_t Operation(int64_t lhs, int64_t rhs) override {
        return std::max(lhs, rhs);
    }
    int64_t GetDefaultValue() override {
        throw RuntimeError("Max hasn't default value");
    }
};

class MinFunction : public ArithmeticFunction {
protected:
    int64_t Operation(int64_t lhs, int64_t rhs) override {
        return std::min(lhs, rhs);
    }
    int64_t GetDefaultValue() override {
        throw RuntimeError("Min hasn't default value");
    }
};

class EqualFunction : public ComparisonFunction {
protected:
    int64_t Compare(int64_t lhs, int64_t rhs) override {
        return lhs == rhs;
    }
};

class LessFunction : public ComparisonFunction {
protected:
    int64_t Compare(int64_t lhs, int64_t rhs) override {
        return lhs < rhs;
    }
};

class GreaterFunction : public ComparisonFunction {
protected:
    int64_t Compare(int64_t lhs, int64_t rhs) override {
        return lhs > rhs;
    }
};

class LessOrEqualFunction : public ComparisonFunction {
protected:
    int64_t Compare(int64_t lhs, int64_t rhs) override {
        return lhs <= rhs;
    }
};

class GreaterOrEqualFunction : public ComparisonFunction {
protected:
    int64_t Compare(int64_t lhs, int64_t rhs) override {
        return lhs >= rhs;
    }
};

class FunctionObject : public Object {
public:
    explicit FunctionObject(std::shared_ptr<IFunction> function) : function_(function) {
    }
    std::shared_ptr<Object> Evaluate(std::shared_ptr<Scope>) override {
        return this->shared_from_this();
    }
    std::shared_ptr<IFunction> GetFunction() {
        return function_;
    }

private:
    std::shared_ptr<IFunction> function_;
};

class Number : public Object {
public:
    explicit Number(int value) : value_(value) {
    }
    int GetValue() const {
        return value_;
    }
    std::shared_ptr<Object> Evaluate(std::shared_ptr<Scope>) override {
        return this->shared_from_this();
    }

private:
    int value_;
};

class Symbol : public Object {
public:
    explicit Symbol(const std::string& name) : name_(name) {
    }
    const std::string& GetName() const {
        return name_;
    }
    std::shared_ptr<Object> Evaluate(std::shared_ptr<Scope> scope) override {
        if (name_ == "#t" || name_ == "#f") {
            return this->shared_from_this();
        }
        if (scope->IsFunctionExists(name_)) {
            return std::make_shared<FunctionObject>(scope->GetFunction(name_));
        }
        return scope->GetVariable(name_);
    }

private:
    std::string name_;
};

class Cell : public Object {
public:
    explicit Cell(std::shared_ptr<Object> first, std::shared_ptr<Object> second);
    std::shared_ptr<Object>& GetFirst() {
        return first_;
    }
    std::shared_ptr<Object>& GetSecond() {
        return second_;
    }

    std::shared_ptr<Object> Evaluate(std::shared_ptr<Scope> scope) override;

private:
    std::shared_ptr<Object> first_, second_;
};

///////////////////////////////////////////////////////////////////////////////

// Runtime type checking and convertion.
// This can be helpful: https://en.cppreference.com/w/cpp/memory/shared_ptr/pointer_cast

template <class T>
std::shared_ptr<T> As(const std::shared_ptr<Object>& obj) {
    return std::dynamic_pointer_cast<T>(obj);
}

template <class T>
bool Is(const std::shared_ptr<Object>& obj) {
    return std::dynamic_pointer_cast<T>(obj) != nullptr;
}
