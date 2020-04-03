#ifndef _HELPERS_
#define _HELPERS_

#include <string>
#include "helpers.hpp"

void error(std::string reason) {
    std::cout << "Error: " << reason << std::endl;
    exit(EXIT_FAILURE);
}

bool IS_WHITESPACE(char c) { return (c == '\n' || c == ' ' || c == '\t'); }

std::string TOKENIZER_BAD_TOKEN_ERROR_MESSAGE(int line_number, int column_number, char c) {
    std::string err = "Bad token \'";
    err += c;
    err += "\' at line ";
    err += std::to_string(line_number);
    err += " column ";
    err += std::to_string(column_number);
    return err;
}

#endif
