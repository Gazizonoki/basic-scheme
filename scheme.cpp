#include "scheme.h"
#include "parser.h"
#include <sstream>

std::string Interpreter::Run(const std::string& input) {
    visited_.clear();
    std::stringstream in;
    in << input;
    Tokenizer tokenizer(&in);
    std::string ans;
    auto root = Read(&tokenizer);
    while (!tokenizer.IsEnd()) {
        Read(&tokenizer);
    }
    if (scope_ == nullptr) {
        scope_ = std::make_shared<Scope>();
        scope_->CreateGlobalScope();
    }
    return Serialize(Evaluate(root, scope_));
}

std::string Interpreter::Serialize(std::shared_ptr<Object> object) {
    if (visited_.contains(object)) {
        return "(...)";
    }
    visited_.insert(object);
    if (!object) {
        return "()";
    }
    if (Is<Number>(object)) {
        return std::to_string(As<Number>(object)->GetValue());
    }
    if (Is<Symbol>(object)) {
        return As<Symbol>(object)->GetName();
    }
    std::string ans;
    ans += "(";
    while (object && Is<Cell>(object)) {
        ans += Serialize(As<Cell>(object)->GetFirst());
        object = As<Cell>(object)->GetSecond();
        if (object) {
            ans += " ";
        }
        if (Is<Cell>(object)) {
            if (visited_.contains(object)) {
                object = nullptr;
                ans += "(...)";
                break;
            }
            visited_.insert(object);
        }
    }
    if (object) {
        ans += ". " + Serialize(object);
    }
    ans += ")";

    return ans;
}
