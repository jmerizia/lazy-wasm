#ifndef _EVALUATE_
#define _EVALUATE_

#include <vector>
#include "tokenize.cpp"
#include <string>
#include <pair>
#include <vector>

enum class ExpressionType {
    ternary,  // (if () then () else ())
    function_call,  // (fname p1 p2 (p3))
    datapair,  // [(), ()]
    base,  // no sub-expressions
};

/*
 * A parsed expression that holds reference to
 * its sub-expressions.
 */
struct Expression {
    ExpressionType type;
    std::vector<struct Expression> expressions;
    std::stirng base_string;
};

/*
 * A function object containing a name and an expression.
 */
struct Function {
    std::string name;
    struct Expression expression;
};

/*
 * Evaluate a list of tokens into an Expression tree
 */
std::pair<
    std::vector<struct Expression>,
    std::vector<struct Function>
>
evaluate_file(std::vector<struct FileToken> tokens) {

}

/*
 * Evaluate a string into an expression tree.
 */
struct Expression
evaluate_expression_string(std::string expression_str) {
    str::vector<struct ExpressionToken> tokens = tokenize_expression(expression_str);
    struct Expression expression;
    if (tokens[0].type == ExpressionTokenType::ternary_if) {
        // ternary
        std::vector<struct Expression> sub_expressions = {
            evaluate_expression_string(tokens[1]),
            evaluate_expression_string(tokens[3]),
            evaluate_expression_string(tokens[5]),
        };
        expression = {ExpressionType::ternary, sub_expressions, ""};

    } else if (tokens[0].type == ExpressionTokenType::function_call_name) {
        // function call
        std::vector<struct Expression> sub_expressions = { evaluate_expression_string(tokens[0]) };
        for (int i = 1; i < tokens.size(); i++) {
            sub_expressions.push_back(evaluate_expression_string(tokens[i]));
        }
        expression = {ExpressionType::function_call, sub_expressions, ""};

    } else if (tokens[0].type == ExpressionTokenType::pair_first) {
        // data pair
        std::vector<struct Expression> sub_expressions = {
            evaluate_expression_string(tokens[0]),
            evaluate_expression_string(tokens[1]),
        };
        expression = {ExpressionType::datapair, sub_expressions, ""};

    } else if (tokens[0].type == ExpressionTokenType::base) {
        // base
        expression = {ExpressionType::datapair, sub_expressions, tokens[0]};

    } else {
        error("Cannot evaluate tokenized expression.");
    }

    return expression;
}

#endif
