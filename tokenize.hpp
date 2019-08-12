#ifndef _TOKENIZE_H_
#define _TOKENIZE_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>

#include "helpers.hpp"

enum class FileTokenType {
    function_declaration_name,
    function_declaration_parameter,
    function_declaration_arrow,
    function_declaration_expression,
    expression,
    none,
};

struct FileToken {
    std::string value;
    FileTokenType type;
};

enum class ExpressionTokenType {
    expression,
    function_call_name,
    function_call_parameter,
    variable,
    base,
    none,
};

struct ExpressionToken {
    std::string value;
    ExpressionTokenType type;
};

std::vector<struct ExpressionToken> tokenize_expression(std::string);
std::vector<struct FileToken> tokenize_file(std::ifstream&);

#endif
