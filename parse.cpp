#ifndef _PARSE_
#define _PARSE_

#include <vector>
#include <string>
#include <vector>
#include <fstream>
#include <map>
#include <regex>

#include "parse.hpp"
#include "tokenize.hpp"
#include "helpers.hpp"

std::map<ExpressionType, std::string> ExpressionType_map = {
    {ExpressionType::function_call, "function_call"},
    {ExpressionType::lazy_pair, "lazy_pair"},
    {ExpressionType::eager_pair, "eager_pair"},
    {ExpressionType::variable, "variable"},
    {ExpressionType::base, "base"},
};

/*
 * Parse a string into an expression tree.
 */
struct Expression parse_expression_string(std::string expression_str) {
    std::vector<struct ExpressionToken> tokens = tokenize_expression(expression_str);
    struct Expression expression;
    if (tokens[0].type == ExpressionTokenType::function_call_name) {
        // function call
        std::vector<struct Expression> sub_expressions = { parse_expression_string(tokens[0].value) };
        for (int i = 1; i < (int)tokens.size(); i++) {
            sub_expressions.push_back(parse_expression_string(tokens[i].value));
        }
        expression = {ExpressionType::function_call, sub_expressions, ""};

    } else if (tokens[0].type == ExpressionTokenType::lazy_pair_first) {
        // eager data pair
        std::vector<struct Expression> sub_expressions = {
            parse_expression_string(tokens[0].value),
            parse_expression_string(tokens[1].value),
        };
        expression = {ExpressionType::lazy_pair, sub_expressions, ""};

    } else if (tokens[0].type == ExpressionTokenType::eager_pair_first) {
        // lazy data pair
        std::vector<struct Expression> sub_expressions = {
            parse_expression_string(tokens[0].value),
            parse_expression_string(tokens[1].value),
        };
        expression = {ExpressionType::eager_pair, sub_expressions, ""};

    } else if (tokens[0].type == ExpressionTokenType::variable) {
        // variable
        std::vector<struct Expression> sub_expressions;
        expression = {ExpressionType::variable, sub_expressions, tokens[0].value};

    } else if (tokens[0].type == ExpressionTokenType::base) {
        // base
        std::vector<struct Expression> sub_expressions;
        expression = {ExpressionType::base, sub_expressions, tokens[0].value};

    } else {
        std::string msg = "Cannot parse tokenized expression: \"";
        if (expression_str.size() > 30) {
            msg += expression_str.substr(0, 30);
        } else {
            msg += expression_str;
        }
        msg += "\"";
        error(msg);
    }

    return expression;
}


/*
 * Parse a file into a set of expression trees and functions.
 */
std::pair<std::vector<struct Expression>, std::vector<struct Function>> parse_file(std::ifstream& f) {
    std::vector<struct FileToken> tokens = tokenize_file(f);
    std::vector<struct Expression> expressions;
    std::vector<struct Function> functions;

    for (int i = 0; i < (int)tokens.size(); ) {
        if (tokens[i].type == FileTokenType::function_declaration_name) {
            std::string name = tokens[i].value;
            std::vector<std::string> parameters;
            i++;
            for (; i < (int)tokens.size() && tokens[i].type == FileTokenType::function_declaration_parameter; i++) {
                parameters.push_back(tokens[i].value);
            }
            i++; // skip the arrow
            if (i == (int)tokens.size() || tokens[i].type != FileTokenType::function_declaration_expression) {
                std::string msg = "Error parsing function \"";
                msg += name;
                msg += "\"";
                error(msg);
            } else {
                std::string expression_str = tokens[i].value;
                struct Function function = {name, parameters, parse_expression_string(expression_str)};
                functions.push_back(function);
                i++;
            }

        } else if (tokens[i].type == FileTokenType::expression) {
            struct Expression expression = parse_expression_string(tokens[i].value);
            expressions.push_back(expression);
            i++;

        } else {
            std::string msg = "Unexpected token \"";
            msg += tokens[i].value;
            msg += "\"";
            error(msg);
        }
    }
    return {expressions, functions};
}

void print_expression_tree(struct Expression& expression, int depth) {
    for (int i = 0; i < depth; i++) {
        std::cout << "  ";
    }
    if (expression.type == ExpressionType::base || expression.type == ExpressionType::variable) {
        std::cout << ExpressionType_map[expression.type] << " : ";
        std::cout << expression.base_string << std::endl;
    } else {
        std::cout << ExpressionType_map[expression.type] << std::endl;
        for (struct Expression sub_expression : expression.expressions) {
            print_expression_tree(sub_expression, depth + 1);
        }
    }
}

#endif
