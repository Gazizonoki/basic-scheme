#include <tokenizer.h>
#include <cctype>
#include <error.h>

bool SymbolToken::operator==(const SymbolToken &other) const {
    return name == other.name;
}

bool QuoteToken::operator==(const QuoteToken &) const {
    return true;
}

bool DotToken::operator==(const DotToken &) const {
    return true;
}

bool ConstantToken::operator==(const ConstantToken &other) const {
    return value == other.value;
}

Tokenizer::Tokenizer(std::istream *in) {
    current_stream_ = in;
    Next();
}

bool Tokenizer::IsEnd() {
    return is_eof_;
}

Token Tokenizer::GetToken() {
    return current_token_;
}

void Tokenizer::Next() {
    if (IsEnd()) {
        return;
    }
    std::string buf;
    SkipSpaces();
    buf += current_stream_->get();
    if (current_stream_->eof()) {
        is_eof_ = true;
        return;
    }
    if (buf.back() == '.') {
        current_token_ = DotToken();
        return;
    }
    if (buf.back() == '\'') {
        current_token_ = QuoteToken();
        return;
    }
    if (buf.back() == '(') {
        current_token_ = BracketToken::OPEN;
        return;
    }
    if (buf.back() == ')') {
        current_token_ = BracketToken::CLOSE;
        return;
    }
    bool is_symbol = false;
    if ((buf.back() == '+' || buf.back() == '-') && !std::isdigit(current_stream_->peek())) {
        current_token_ = SymbolToken();
        get<SymbolToken>(current_token_).name = buf;
        return;
    }
    if (!IsBeginSymbol(buf.back()) && !std::isdigit(buf.back()) && buf.back() != '+' &&
        buf.back() != '-') {
        throw SyntaxError("Wrong syntax");
    }
    if (IsBeginSymbol(buf.back())) {
        is_symbol = true;
    }
    if (is_symbol) {
        while (IsSymbol(current_stream_->peek())) {
            buf += current_stream_->get();
        }
        current_token_ = SymbolToken();
        get<SymbolToken>(current_token_).name = buf;
    } else {
        while (std::isdigit(current_stream_->peek())) {
            buf += current_stream_->get();
        }
        current_token_ = ConstantToken();
        get<ConstantToken>(current_token_).value = std::stoi(buf);
    }
}

void Tokenizer::SkipSpaces() {
    while (std::isspace(current_stream_->peek())) {
        current_stream_->get();
    }
}

bool Tokenizer::IsBeginSymbol(char c) {
    return std::isalpha(c) || c == '<' || c == '=' || c == '>' || c == '*' || c == '/' || c == '#';
}

bool Tokenizer::IsSymbol(char c) {
    return IsBeginSymbol(c) || std::isdigit(c) || c == '?' || c == '!' || c == '-';
}