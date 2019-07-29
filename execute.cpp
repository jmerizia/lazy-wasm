#ifndef _EXECUTE_
#define _EXECUTE_

#include <vector>
#include <string>
#include <map>

struct ExecutionResult {
    bool is_nil;
    int value;
};

struct Variable {
    bool is_nil;
    int value;
    std::string name;
};


struct ExecutionResult
reserved_add(struct ExecutionResult a, struct ExecutionResult b) {
    if (a.is_nil || b.is_nil) return {true, 0};
    else return {false, a.value + b.value};
}

struct ExecutionResult
reserved_sub(struct ExecutionResult a, struct ExecutionResult b) {
    if (a.is_nil || b.is_nil) return {true, 0};
    else return {false, a.value - b.value};
}

struct ExecutionResult
reserved_mul(struct ExecutionResult a, struct ExecutionResult b) {
    if (a.is_nil || b.is_nil) return {true, 0};
    else return {false, a.value * b.value};
}

struct ExecutionResult
reserved_div(struct ExecutionResult a, struct ExecutionResult b) {
    if (a.is_nil || b.is_nil) return {true, 0};
    else return {false, a.value / b.value};
}

struct ExecutionResult
reserved_equals(struct ExecutionResult a, struct ExecutionResult b) {
    if (a.is_nil && b.is_nil) return {false, 1};
    if ((a.is_nil && !b.is_nil) || (!a.is_nil && b.is_nil)) return {false, 0};
    return {false, a.value == b.value};
}

struct ExecutionResult
reserved_print_int(struct ExecutionResult a) {
    std::cout << "printing!" << std::endl;
    if (a.is_nil) std::cout << "nil" << std::endl;
    else std::cout << a.value;
    return {true, 0};
}

struct ExecutionResult
reserved_print_char(struct ExecutionResult a) {
    std::cout << "printing!" << std::endl;
    if (a.is_nil) std::cout << "nil" << std::endl;
    else std::cout << char(a.value);
    return {true, 0};
}

std::map<std::string, std::function<struct ExecutionResult(struct ExecutionResult)>>
ReservedFunctionsOneInput = {
    {"print_int", reserved_print_int},
    {"print_char", reserved_print_char},
};

std::map<std::string, std::function<struct ExecutionResult(struct ExecutionResult, struct ExecutionResult)>>
ReservedFunctionsTwoInput = {
    {"add", reserved_add},
    {"sub", reserved_sub},
    {"mul", reserved_mul},
    {"div", reserved_div},
    {"equals", reserved_equals},
};

struct ExecutionResult execute_expression(
    struct Expression& expression,
    std::vector<struct Function>& functions,
    std::vector<struct Variable>& variables,
    int datapair_side) {
    /*
    ternary,  // (if () then () else ())
    function_call,  // (fname p1 p2 (p3))
    datapair,  // [(), ()]
    base,  // no sub-expressions
    */
    std::cout << "parsing.. " << ExpressionType_map[expression.type] << " : " << expression.base_string << std::endl;
    struct ExecutionResult result;
    switch (expression.type) {
        case ExpressionType::ternary:
            {
                struct Expression condition = expression.expressions[0];
                struct Expression first = expression.expressions[1];
                struct Expression second = expression.expressions[2];
                struct ExecutionResult result = execute_expression(condition, functions, variables, 0);
                if (result.is_nil || result.value == 0) {
                    result = execute_expression(second, functions, variables, 0);
                } else {
                    result = execute_expression(first, functions, variables, 0);
                }
                break;
            }
        case ExpressionType::function_call:
            {
                std::string name = expression.expressions[0].base_string;
                std::cout << "function call " << name << std::endl;
                bool matched = false;
                for (struct Function function : functions) {
                    if (function.name == name) {
                        matched = true;
                        // execute parameters, put results into variables
                        for (int i = 1; i < expression.expressions.size(); i++) {
                            print_expression_tree(expression.expressions[i], 1);
                            struct ExecutionResult res =
                                execute_expression(expression.expressions[i], functions, variables, 0);
                            std::string param_name = function.parameters[i-1]; /* name from defn*/
                            variables.push_back({res.is_nil, res.value, param_name});
                        }
                        result = execute_expression(function.expression, functions, variables, 0);
                        break;
                    }
                }
                if (!matched) {
                    if (CONTAINS(ReservedFunctionsOneInput, name)) {
                        if (expression.expressions.size() != 2) {
                            std::string msg = "Incorrect number of arguments for function ";
                            msg += name;
                            error(msg);
                        } else {
                            auto f = ReservedFunctionsOneInput[name];
                            struct ExecutionResult a = execute_expression(expression.expressions[1], functions, variables, 0);
                            result = f(a);
                        }
                    } else if (CONTAINS(ReservedFunctionsTwoInput, name)) {
                        if (expression.expressions.size() != 3) {
                            std::string msg = "Incorrect number of arguments for function ";
                            msg += name;
                            error(msg);
                        } else {
                            auto f = ReservedFunctionsTwoInput[name];
                            struct ExecutionResult a = execute_expression(expression.expressions[1], functions, variables, 0);
                            struct ExecutionResult b = execute_expression(expression.expressions[2], functions, variables, 0);
                            result = f(a, b);
                        }
                    }
                }
                break;
            }
        case ExpressionType::datapair:
            break;
        case ExpressionType::base:
            {
                // int ^\s*-?\d+\s*$
                // nil ^\s*nil\s*$
                std::regex nil_re ("^\\s*nil\\s*$");
                std::regex int_re ("^\\s*-?\\d+\\s*$");
                std::smatch match;
                if (std::regex_search(expression.base_string, match, nil_re)) {
                    result = {true, 0};
                } else if (std::regex_search(expression.base_string, match, int_re)) {
                    result = {false, std::stoi(expression.base_string)};
                } else {
                    // variable name
                    std::cout << "variable name: " << expression.base_string << std::endl;
                    bool matched = false;
                    for (struct Variable variable : variables) {
                        if (variable.name == expression.base_string) {
                            matched = true;
                            result = {variable.is_nil, variable.value};
                            break;
                        }
                    }
                    if (!matched) {
                        result = {true, 0};
                    }
                }
            }
            break;
    }
    return result;
}

#endif
