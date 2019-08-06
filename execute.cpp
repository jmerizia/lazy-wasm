#ifndef _EXECUTE_
#define _EXECUTE_

#include <vector>
#include <string>
#include <map>
#include <functional>
#include <regex>

#include "execute.hpp"
#include "helpers.hpp"
#include "parse.hpp"

class SystemFunctions {
public:
    static struct ExecutionResult add(Thunk a, Thunk b, Context * context) {
        // execute both thunks if they haven't been already:
        if (!a.result.has_value()) {
            struct ExecutionResult result = execute_expression(a.expression, context);
            a.result.set(result);
        }
        if (!b.result.has_value()) {
            struct ExecutionResult result = execute_expression(b.expression, context);
            b.result.set(result);
        }
        struct ExecutionResult a_ = a.result.get();
        struct ExecutionResult b_ = a.result.get();
        if (a_.is_nil || b_.is_nil) {
            return {true, 0};
        } else {
            return {false, a_.value + b_.value};
        }
    }

    static struct ExecutionResult print_int(Thunk a, Context * context) {
        // execute thunk if it hasn't been already:
        if (!a.result.has_value()) {
            struct ExecutionResult result = execute_expression(a.expression, context);
            a.result.set(result);
        }
        std::cout << a.result.get().value;
        return {true, 0};
    }

    static struct ExecutionResult _if(Thunk a, Thunk b, Thunk c, Context * context) {
        return {true, 0};
    }
};

std::map<std::string, std::function<struct ExecutionResult(Thunk, Context*)>>
SystemFunctionsOneInput = {
    {"print_int", SystemFunctions::print_int},
};

std::map<std::string, std::function<struct ExecutionResult(Thunk, Thunk, Context*)>>
SystemFunctionsTwoInput = {
    {"add", SystemFunctions::add},
};

std::map<std::string, std::function<struct ExecutionResult(Thunk, Thunk, Thunk, Context*)>>
SystemFunctionsThreeInput = {
    {"if", SystemFunctions::_if},
};

struct ExecutionResult
execute_expression(struct Expression& expression, Context * context)
{
#ifdef LOGGING
    //std::cout << ExpressionType_map[expression.type] << " : " << expression.base_string << std::endl;
    //print_context(context);
#endif
    struct ExecutionResult result;
    switch (expression.type) {
        case ExpressionType::function_call:
            {
                std::string name = expression.expressions[0].base_string;
                int num_params_given = expression.expressions.size() - 1;
                std::vector<Thunk> thunked_params;
                for (int i = 1; i < (int)expression.expressions.size(); i++) {
                    Optional<struct ExecutionResult> r;
                    Thunk thunk = {expression.expressions[i], r, context->uuid->get(), *context};
                    thunked_params.push_back(thunk);
                    context->thunk_store->insert({thunk.name, thunk});
                }
                
                // if it's a 3 input system function:
                if (CONTAINS(SystemFunctionsThreeInput, name)) {
                    if (num_params_given != 3) {
                        std::string msg = "Function ";
                        msg += name + " requires 3 parameters, but " \
                               + std::to_string(num_params_given) + " were given.";
                        error(msg);
                    } else {
                        result = SystemFunctionsThreeInput[name](
                            thunked_params[0],
                            thunked_params[1],
                            thunked_params[2],
                            context
                        );
                    }

                // if it's a 2 input system function:
                } else if (CONTAINS(SystemFunctionsTwoInput, name)) {
                    if (num_params_given != 2) {
                        std::string msg = "Function ";
                        msg += name + " requires 2 parameters, but " \
                               + std::to_string(num_params_given) + " were given.";
                        error(msg);
                    } else {
                        result = SystemFunctionsTwoInput[name](
                            thunked_params[0],
                            thunked_params[1],
                            context
                        );
                    }

                // if it's a 1 input system function:
                } else if (CONTAINS(SystemFunctionsOneInput, name)) {
                    if (num_params_given != 1) {
                        std::string msg = "Function ";
                        msg += name + " requires 1 parameter, but " \
                               + std::to_string(num_params_given) + " were given.";
                        error(msg);
                    } else {
                        result = SystemFunctionsOneInput[name](
                            thunked_params[0],
                            context
                        );
                    }

                } else {
                    // check if it's a user defined function:
                    bool found_match = false;
                    for (struct Function function : *context->functions) {
                        if (function.name == name) {
                            if (num_params_given != (int)function.parameters.size()) {
                                //error

                            } else {
                                found_match = true;
                                // add thunks to variable map
                                for (int i = 0; i < num_params_given; i++) {
                                    context->variable_to_thunk[function.parameters[i]] = thunked_params[i].name;
                                }
                                result = execute_expression(function.expression, context);
                                break;
                            }
                        }
                    }

                    if (!found_match) {
                        std::string msg = "Function " + name + \
                                           " didn't match any known functions.";
                        error(msg);
                    }
                }
                break;
            }

        case ExpressionType::lazy_pair:
            {
                result = {true, 0};
                break;
            }

        case ExpressionType::eager_pair:
            {
                result = {true, 0};
                break;
            }

        case ExpressionType::variable:
            {
                std::string name = expression.base_string;
                if (!CONTAINS(context->variable_to_thunk, name)) {
                    std::string msg = "Variable " + name + " not defined.";
                    error(msg);
                }
                Thunk t = (*context->thunk_store)[context->variable_to_thunk[name]];
                if (!t.result.has_value()) {
                    result = execute_expression(t.expression, context);
                    t.result.set(result);
                } else {
                    result = t.result.get();
                }
                break;
            }

        case ExpressionType::base:
            {
                std::regex integer_re ("^\\s*\\(?\\s*(-?\\d+)\\s*\\)?\\s*$");
                std::regex nil_re ("^\\s*\\(?\\s*(nil)\\s*\\)?\\s*$");
                std::smatch match;

                if (std::regex_search(expression.base_string, match, integer_re)) {
                    std::string s = match.str(1);
                    result = {false, std::stoi(s)};

                } else if (std::regex_search(expression.base_string, match, nil_re)) {
                    result = {true, 0};

                } else {
                    std::string msg = "Failed to match expression " + expression.base_string;
                    error(msg);
                }
                break;
            }
    }
    return result;
}

void print_thunk(Thunk * thunk) {
    std::cout << "THUNK: " << thunk->name << " -> " << std::endl;
    if (thunk->result.has_value()) {
        std::cout << thunk->result.get().value;
    } else {
        std::cout << "?" << std::endl;
    }
}

void print_context(Context * context) {
    // print variables and thunks
    for (std::pair<std::string, std::string> v_t : context->variable_to_thunk) {
        Thunk t = (*context->thunk_store)[v_t.second];
        std::cout << "MAP " << v_t.first << " -> " << v_t.second << " :: ";
        print_thunk(&t);
        std::cout << std::endl;
    }
    // TODO: print data pair sides
}


#endif
