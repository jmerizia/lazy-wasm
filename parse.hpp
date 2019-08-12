#ifndef _PARSE_H_
#define _PARSE_H_

#include <vector>
#include <string>
#include <map>

#include "tokenize.hpp"

enum class ExpressionType {
    function_call,  // (fname p1 p2 (p3))
    variable,  // (a) a
    base,  // (nil) nil (42) 42
};

/*
 * A parsed expression that holds reference to
 * its sub-expressions.
 */
struct Expression {
    ExpressionType type;
    std::vector<struct Expression> expressions;
    std::string base_string;
    int int_value;
    bool is_nil;
};

/*
 * A function object containing a name and an expression.
 */
struct Function {
    std::string name;
    std::vector<std::string> parameters;
    struct Expression expression;
};

struct Expression parse_expression_string(std::string);
std::pair<std::vector<struct Expression>, std::vector<struct Function>> parse_file(std::ifstream&);
void print_expression_tree(struct Expression&, int);

extern std::map<ExpressionType, std::string> ExpressionType_map;

#endif
