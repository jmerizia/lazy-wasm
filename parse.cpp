#ifndef _PARSE_
#define _PARSE_

#include <vector>
#include "tokenize.cpp"
#include <string>
#include <vector>
#include <fstream>
#include <map>

enum class ExpressionType {
    ternary,  // (if () then () else ())
    function_call,  // (fname p1 p2 (p3))
    datapair,  // [(), ()]
    base,  // no sub-expressions
};

std::map<ExpressionType, std::string> ExpressionType_map = {
    {ExpressionType::ternary, "ternary"},
    {ExpressionType::function_call, "function_call"},
    {ExpressionType::datapair, "datapair"},
    {ExpressionType::base, "base"},
};

/*
 * A parsed expression that holds reference to
 * its sub-expressions.
 */
struct Expression {
    ExpressionType type;
    std::vector<struct Expression> expressions;
    std::string base_string;
};

/*
 * A function object containing a name and an expression.
 */
struct Function {
    std::string name;
    std::vector<std::string> parameters;
    struct Expression expression;
};

/*
 * Parse a string into an expression tree.
 */
struct Expression
parse_expression_string(std::string expression_str) {
    std::vector<struct ExpressionToken> tokens = tokenize_expression(expression_str);
    struct Expression expression;
    if (tokens[0].type == ExpressionTokenType::ternary_if) {
        // ternary
        std::vector<struct Expression> sub_expressions = {
            parse_expression_string(tokens[1].value),
            parse_expression_string(tokens[3].value),
            parse_expression_string(tokens[5].value),
        };
        expression = {ExpressionType::ternary, sub_expressions, ""};

    } else if (tokens[0].type == ExpressionTokenType::function_call_name) {
        // function call
        std::vector<struct Expression> sub_expressions = { parse_expression_string(tokens[0].value) };
        for (int i = 1; i < tokens.size(); i++) {
            sub_expressions.push_back(parse_expression_string(tokens[i].value));
        }
        expression = {ExpressionType::function_call, sub_expressions, ""};

    } else if (tokens[0].type == ExpressionTokenType::pair_first) {
        // data pair
        std::vector<struct Expression> sub_expressions = {
            parse_expression_string(tokens[0].value),
            parse_expression_string(tokens[1].value),
        };
        expression = {ExpressionType::datapair, sub_expressions, ""};

    } else if (tokens[0].type == ExpressionTokenType::base) {
        // base
        std::vector<struct Expression> sub_expressions;
        expression = {ExpressionType::base, sub_expressions, tokens[0].value};

    } else {
        error("Cannot parse tokenized expression.");
    }

    return expression;
}


/*
 * Parse a file into a set of expression trees and functions.
 */
std::pair<
    std::vector<struct Expression>,
    std::vector<struct Function>
>
parse_file(std::ifstream& f) {
    std::vector<struct FileToken> tokens = tokenize_file(f);
    std::vector<struct Expression> expressions;
    std::vector<struct Function> functions;

    for (int i = 0; i < tokens.size(); ) {
        if (tokens[i].type == FileTokenType::function_declaration_name) {
            std::string name = tokens[i].value;
            std::vector<std::string> parameters;
            i++;
            for (; i < tokens.size() && tokens[i].type == FileTokenType::function_declaration_parameter; i++) {
                parameters.push_back(tokens[i].value);
            }
            i++; // skip the arrow
            if (i == tokens.size() || tokens[i].type != FileTokenType::function_declaration_expression) {
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
    if (expression.type == ExpressionType::base) {
        for (int i = 0; i < depth; i++) {
            std::cout << "  ";
        }
        std::cout << ExpressionType_map[expression.type] << " : ";
        std::cout << expression.base_string << std::endl;
    } else {
        for (int i = 0; i < depth; i++) {
            std::cout << "  ";
        }
        std::cout << ExpressionType_map[expression.type] << std::endl;
        for (struct Expression sub_expression : expression.expressions) {
            print_expression_tree(sub_expression, depth + 1);
        }
    }
}

#endif
