#pragma once

#include <string>
#include <set>

#include "object.h"

class Interpreter {
public:
    std::string Run(const std::string& input);

private:
    std::string Serialize(std::shared_ptr<Object> object);
    std::shared_ptr<Scope> scope_;
    std::set<std::shared_ptr<Object>> visited_;
};
