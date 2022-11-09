#include "parser.h"
#include "error.h"

void AddListElement(std::shared_ptr<Object>& vertex, std::shared_ptr<Object> son) {
    if (vertex == nullptr) {
        vertex = std::make_shared<Cell>(son, nullptr);
        return;
    }
    AddListElement(As<Cell>(vertex)->GetSecond(), son);
}

void AddBadListElement(std::shared_ptr<Object>& vertex, std::shared_ptr<Object> son) {
    if (vertex == nullptr) {
        throw SyntaxError("Wrong syntax");
    }
    if (As<Cell>(vertex)->GetSecond() == nullptr) {
        vertex = std::make_shared<Cell>(As<Cell>(vertex)->GetFirst(), son);
        return;
    }
    AddBadListElement(As<Cell>(vertex)->GetSecond(), son);
}

std::shared_ptr<Object> ReadList(Tokenizer* tokenizer) {
    std::shared_ptr<Object> root = nullptr;
    tokenizer->Next();
    bool bad_list = false;
    while (!(std::holds_alternative<BracketToken>(tokenizer->GetToken()) &&
             std::get<BracketToken>(tokenizer->GetToken()) == BracketToken::CLOSE)) {
        if (tokenizer->IsEnd()) {
            throw SyntaxError("wrong syntax");
        }
        if (bad_list) {
            throw SyntaxError("wrong syntax");
        }
        if (std::holds_alternative<DotToken>(tokenizer->GetToken())) {
            tokenizer->Next();
            AddBadListElement(root, Read(tokenizer));
            if (tokenizer->IsEnd()) {
                throw SyntaxError("wrong syntax");
            }
            bad_list = true;
            continue;
        }

        AddListElement(root, Read(tokenizer));
        if (tokenizer->IsEnd()) {
            throw SyntaxError("wrong syntax");
        }
    }
    tokenizer->Next();
    return root;
}

std::shared_ptr<Object> Read(Tokenizer* tokenizer) {
    if (tokenizer->IsEnd()) {
        throw SyntaxError("wrong syntax");
    }
    if (std::holds_alternative<BracketToken>(tokenizer->GetToken()) &&
        std::get<BracketToken>(tokenizer->GetToken()) == BracketToken::OPEN) {
        return ReadList(tokenizer);
    }
    if (std::holds_alternative<QuoteToken>(tokenizer->GetToken())) {
        tokenizer->Next();
        return std::make_shared<Cell>(std::make_shared<Symbol>("quote"),
                                      std::make_shared<Cell>(Read(tokenizer), nullptr));
    }
    std::shared_ptr<Object> root;
    if (std::holds_alternative<ConstantToken>(tokenizer->GetToken())) {
        root = std::make_shared<Number>(std::get<ConstantToken>(tokenizer->GetToken()).value);
    } else {
        if (std::holds_alternative<SymbolToken>(tokenizer->GetToken())) {
            root = std::make_shared<Symbol>(std::get<SymbolToken>(tokenizer->GetToken()).name);
        } else {
            throw SyntaxError("Wrong syntax");
        }
    }
    tokenizer->Next();
    return root;
}