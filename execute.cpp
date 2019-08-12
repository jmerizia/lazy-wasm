#ifndef _EXECUTE_
#define _EXECUTE_

#include <vector>
#include <string>
#include <map>
#include <functional>
#include <regex>
#include <set>

#include "execute.hpp"
#include "helpers.hpp"
#include "parse.hpp"


class SystemFunctions {
public:
    static struct ExecutionResult add(Thunk * a, Thunk * b, Context * context, int depth) {
#ifdef LOGGING
        for (int i = 0; i < depth; i++) {
            std::cout << "  ";
        }
        std::cout << "SYSTEM ADD: " << a->name << " " << b->name << std::endl;
        print_context(context, depth);
#endif
        // execute both thunks if they haven't been already:
        if (!a->result.has_value()) {
            struct ExecutionResult result = execute_expression(a->expression, a->context, depth + 1);
            a->result.set(result);
        }
        if (!b->result.has_value()) {
            struct ExecutionResult result = execute_expression(b->expression, b->context, depth + 1);
            b->result.set(result);
        }
        struct ExecutionResult a_ = a->result.get();
        struct ExecutionResult b_ = b->result.get();
        if (a_.is_nil || b_.is_nil) {
            return {true, 0};
        } else {
            return {false, a_.value + b_.value};
        }
    }

    static struct ExecutionResult sub(Thunk * a, Thunk * b, Context * context, int depth) {
#ifdef LOGGING
        for (int i = 0; i < depth; i++) {
            std::cout << "  ";
        }
        std::cout << "SYSTEM SUB: " << a->name << " " << b->name << std::endl;
        print_context(context, depth);
#endif
        // execute both thunks if they haven't been already:
        if (!a->result.has_value()) {
            struct ExecutionResult result = execute_expression(a->expression, a->context, depth + 1);
            a->result.set(result);
        }
        if (!b->result.has_value()) {
            struct ExecutionResult result = execute_expression(b->expression, b->context, depth + 1);
            b->result.set(result);
        }
        struct ExecutionResult a_ = a->result.get();
        struct ExecutionResult b_ = b->result.get();
        if (a_.is_nil || b_.is_nil) {
            return {true, 0};
        } else {
            return {false, a_.value - b_.value};
        }
    }

    static struct ExecutionResult equals(Thunk * a, Thunk * b, Context * context, int depth) {
#ifdef LOGGING
        for (int i = 0; i < depth; i++) {
            std::cout << "  ";
        }
        std::cout << "SYSTEM EQUALS: " << a->name << " " << b->name << std::endl;
        print_context(context, depth);
#endif
        // execute both thunks if they haven't been already:
        if (!a->result.has_value()) {
            struct ExecutionResult result = execute_expression(a->expression, a->context, depth + 1);
            a->result.set(result);
        }
        if (!b->result.has_value()) {
            struct ExecutionResult result = execute_expression(b->expression, b->context, depth + 1);
            b->result.set(result);
        }
        struct ExecutionResult a_ = a->result.get();
        struct ExecutionResult b_ = b->result.get();
        if (a_.is_nil && b_.is_nil) { // both nil
            return {false, 1};
        } else if (!a_.is_nil && !b_.is_nil) { // both not nil
            if (a_.value == b_.value) {
                return {false, 1};
            } else {
                return {false, 0};
            }
        } else {
            return {false, 0};
        }
    }

    static struct ExecutionResult both(Thunk * a, Thunk * b, Context * context, int depth) {
#ifdef LOGGING
        for (int i = 0; i < depth; i++) {
            std::cout << "  ";
        }
        std::cout << "SYSTEM BOTH: " << a->name << " " << b->name << std::endl;
        print_context(context, depth);
#endif
        // execute both thunks (eagerly) if they haven't been already:
        if (!a->result.has_value()) {
            struct ExecutionResult result = execute_expression(a->expression, a->context, depth + 1);
            a->result.set(result);
        }
        if (!b->result.has_value()) {
            struct ExecutionResult result = execute_expression(b->expression, b->context, depth + 1);
            b->result.set(result);
        }
        return {false, 1};
    }

    static struct ExecutionResult first(Thunk * a, Context * context, int depth) {
#ifdef LOGGING
        for (int i = 0; i < depth; i++) {
            std::cout << "  ";
        }
        std::cout << "SYSTEM FIRST: " << a->name << std::endl;
        print_context(context, depth);
#endif
        Context * new_context = new Context(*context);
        // add datapair side
        new_context->datapair_sides->push(1);
        if (!a->result.has_value()) {
            struct ExecutionResult result = execute_expression(a->expression, a->context, depth + 1);
            a->result.set(result);
        }
        return a->result.get();
    }

    static struct ExecutionResult second(Thunk * a, Context * context, int depth) {
#ifdef LOGGING
        for (int i = 0; i < depth; i++) {
            std::cout << "  ";
        }
        std::cout << "SYSTEM SECOND: " << a->name << std::endl;
        print_context(context, depth);
#endif
        Context * new_context = new Context(*context);
        // add datapair side
        new_context->datapair_sides->push(2);
        if (!a->result.has_value()) {
            struct ExecutionResult result = execute_expression(a->expression, a->context, depth + 1);
            a->result.set(result);
        }
        return a->result.get();
    }

    static struct ExecutionResult lazy_pair(Thunk * a, Thunk * b, Context * context, int depth) {
#ifdef LOGGING
        for (int i = 0; i < depth; i++) {
            std::cout << "  ";
        }
        std::cout << "SYSTEM PAIR: " << a->name << " " << b->name << std::endl;
        print_context(context, depth);
#endif
        if (!context->datapair_sides->empty()) {
            int side = context->datapair_sides->top();
            context->datapair_sides->pop();
            if (side == 1) {
                if (!a->result.has_value()) {
                    struct ExecutionResult result = execute_expression(a->expression, a->context, depth + 1);
                    a->result.set(result);
                }
                return a->result.get();
            } else { //side == 2
                if (!b->result.has_value()) {
                    struct ExecutionResult result = execute_expression(b->expression, b->context, depth + 1);
                    b->result.set(result);
                }
                return b->result.get();
            }
        } else {
            return {false, 1};
        }
    }

    static struct ExecutionResult print_int(Thunk * a, Context * context, int depth) {
        // (no need for a new context)
#ifdef LOGGING
        for (int i = 0; i < depth; i++) {
            std::cout << "  ";
        }
        std::cout << "SYSTEM PRINT_INT: " << a->name << " " << std::endl;
        print_context(context, depth);
#endif
        // execute thunk if it hasn't been already:
        if (!a->result.has_value()) {
            struct ExecutionResult result = execute_expression(a->expression, a->context, depth + 1);
            a->result.set(result);
        }
        std::cout << a->result.get().value;
        return {true, 0};
    }

    static struct ExecutionResult print_char(Thunk * a, Context * context, int depth) {
        // (no need for a new context)
#ifdef LOGGING
        for (int i = 0; i < depth; i++) {
            std::cout << "  ";
        }
        std::cout << "SYSTEM PRINT_CHAR: " << a->name << " " << std::endl;
        print_context(context, depth);
#endif
        // execute thunk if it hasn't been already:
        if (!a->result.has_value()) {
            struct ExecutionResult result = execute_expression(a->expression, a->context, depth + 1);
            a->result.set(result);
        }
        std::cout << (char)a->result.get().value;
        return {true, 0};
    }

    static struct ExecutionResult _if(Thunk * a, Thunk * b, Thunk * c, Context * context, int depth) {
#ifdef LOGGING
        for (int i = 0; i < depth; i++) {
            std::cout << "  ";
        }
        std::cout << "SYSTEM IF: " << a->name << " " << b->name << " " << c->name << std::endl;
        print_context(context, depth);
#endif
        // execute the condition
        if (!a->result.has_value()) {
            struct ExecutionResult condition_result = execute_expression(a->expression, a->context, depth + 1);
            a->result.set(condition_result);
        }
        struct ExecutionResult cond = a->result.get();
        struct ExecutionResult result;
        if (cond.is_nil || cond.value == 0) {
            // evaluate then clause
            result = execute_expression(c->expression, c->context, depth + 1);
        } else {
            // evaluate else clause
            result = execute_expression(b->expression, b->context, depth + 1);
        }

        return result;
    }
};

std::map<std::string, std::function<struct ExecutionResult(Thunk*, Context*, int)>>
SystemFunctionsOneInput = {
    {"print_int", SystemFunctions::print_int},
    {"print_char", SystemFunctions::print_char},
    {"first", SystemFunctions::first},
    {"second", SystemFunctions::second},
};

std::map<std::string, std::function<struct ExecutionResult(Thunk*, Thunk*, Context*, int)>>
SystemFunctionsTwoInput = {
    {"add", SystemFunctions::add},
    {"sub", SystemFunctions::sub},
    {"equals", SystemFunctions::equals},
    {"pair", SystemFunctions::lazy_pair},
    {"both", SystemFunctions::both},
};

std::map<std::string, std::function<struct ExecutionResult(Thunk*, Thunk*, Thunk*, Context*, int)>>
SystemFunctionsThreeInput = {
    {"if", SystemFunctions::_if},
};

int cnt = 0;

struct ExecutionResult
execute_expression(struct Expression& expression, Context * context, int depth)
{
    cnt ++;
    if (cnt == 50000) {
        error("Recursion limit reached :(");
    }
    if (cnt % 1000 == 0) {
        std::cout << "execution count = " << cnt << std::endl;
    }
#ifdef LOGGING
    for (int i = 0; i < depth; i++) {
        std::cout << "  ";
    }
    std::cout << "execute" << std::endl;
    for (int i = 0; i < depth; i++) {
        std::cout << "  ";
    }
    if (expression.type == ExpressionType::function_call) {
        std::cout << ExpressionType_map[expression.type] << " : " << expression.expressions[0].base_string << std::endl;
    } else {
        std::cout << ExpressionType_map[expression.type] << " : " << expression.base_string << std::endl;
    }
#endif
    Context * new_context = new Context(*context);
    struct ExecutionResult result;
    switch (expression.type) {
        case ExpressionType::function_call:
            {
                std::string name = expression.expressions[0].base_string;
                int num_params_given = expression.expressions.size() - 1;
                std::vector<Thunk*> thunked_params;
                for (int i = 1; i < (int)expression.expressions.size(); i++) {
                    Optional<struct ExecutionResult> r;
                    Thunk * thunk = new Thunk(
                        expression.expressions[i],
                        r,
                        new_context->uuid->get(),  // this can be written also as context->uuit->get()
                        context  // thunk should use the calling context
                    );
                    thunked_params.push_back(thunk);
                    new_context->add_thunk(thunk);
                }
                
                // if it's a 3 input system function:
                if (CONTAINS(SystemFunctionsThreeInput, name)) {
                    if (num_params_given != 3) {
                        std::string msg = "Function ";
                        msg += name + " requires 3 parameters, but " \
                               + std::to_string(num_params_given) + " were given.";
                        error(msg);
                    } else {
#ifdef LOGGING
                        for (int i = 0; i < depth; i++) {
                            std::cout << "  ";
                        }
                        std::cout << "3-input system function" << std::endl;
                        print_context(new_context, depth);
#endif
                        result = SystemFunctionsThreeInput[name](
                            thunked_params[0],
                            thunked_params[1],
                            thunked_params[2],
                            new_context,
                            depth
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
#ifdef LOGGING
                        for (int i = 0; i < depth; i++) {
                            std::cout << "  ";
                        }
                        std::cout << "2-input system function" << std::endl;
                        print_context(new_context, depth);
#endif
                        result = SystemFunctionsTwoInput[name](
                            thunked_params[0],
                            thunked_params[1],
                            new_context,
                            depth
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
#ifdef LOGGING
                        for (int i = 0; i < depth; i++) {
                            std::cout << "  ";
                        }
                        std::cout << "1-input system function" << std::endl;
                        print_context(new_context, depth);
#endif
                        result = SystemFunctionsOneInput[name](
                            thunked_params[0],
                            new_context,
                            depth
                        );
                    }

                } else {
                    // check if it's a user defined function:
                    bool found_match = false;
                    for (struct Function function : *new_context->functions) {
                        if (function.name == name) {
                            if (num_params_given != (int)function.parameters.size()) {
                                //error

                            } else {
                                found_match = true;
                                // add thunks to variable map
                                for (int i = 0; i < num_params_given; i++) {
                                    new_context->add_thunk_mapping(
                                        function.parameters[i],
                                        thunked_params[i]->name
                                    );
                                }
#ifdef LOGGING
                                for (int i = 0; i < depth; i++) {
                                    std::cout << "  ";
                                }
                                std::cout << num_params_given << "-input user defined function" << std::endl;
                                print_context(new_context, depth);
#endif
                                result = execute_expression(function.expression, new_context, depth + 1);
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
                // TODO: clear out thunks from thunk store
                //for (Thunk * thunk : thunked_params) {
                //    delete thunk;
                //}
                //delete new_context;
                break;
            }

        case ExpressionType::variable:
            {
                std::string name = expression.base_string;
                if (!CONTAINS(*new_context->variable_to_thunk, name)) {
                    std::string msg = "Variable " + name + " not defined.";
                    error(msg);
                }
                Thunk * pt = (*new_context->thunk_store)[(*new_context->variable_to_thunk)[name]];
#ifdef LOGGING
                for (int i = 0; i < depth; i++) {
                    std::cout << "  ";
                }
                std::cout << "variable " << name << std::endl;
                print_context(new_context, depth);
#endif
                if (!pt->result.has_value()) {
                    result = execute_expression(pt->expression, pt->context, depth + 1);
                    pt->result.set(result);
                } else {
                    result = pt->result.get();
                }
                break;
            }

        case ExpressionType::base:
            {
                std::regex integer_re ("^\\s*\\(?\\s*(-?\\d+)\\s*\\)?\\s*$");
                std::regex nil_re ("^\\s*\\(?\\s*(nil)\\s*\\)?\\s*$");
                std::smatch match;

#ifdef LOGGING
                for (int i = 0; i < depth; i++) {
                    std::cout << "  ";
                }
                std::cout << "base " << expression.base_string << std::endl;
                print_context(new_context, depth);
#endif

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

#ifdef LOGGING
    for (int i = 0; i < depth; i++) {
        std::cout << "  ";
    }
    std::cout << "RET " << result.value << std::endl;
#endif
    return result;
}

void print_thunk(Thunk * thunk, int depth) {
    for (int i = 0; i < depth; i++) {
        std::cout << "  ";
    }
    std::cout << "{THUNK: " << thunk->name << " -> ";
    if (thunk->result.has_value()) {
        std::cout << thunk->result.get().value;
    } else {
        std::cout << "?";
    }
    std::cout << "}";
}

void print_context(Context * pcontext, int depth) {
    // print variables and thunks
    for (int i = 0; i < depth; i++) {
        std::cout << "  ";
    }
    std::cout << "Current Context:" << std::endl;
    if (pcontext->variable_to_thunk->empty()) {
        for (int i = 0; i < depth; i++) {
            std::cout << "  ";
        }
        std::cout << "(empty)" << std::endl;
    }
    std::set<std::string> printed;
    for (std::pair<std::string, std::string> v_t : *pcontext->variable_to_thunk) {
        for (int i = 0; i < depth; i++) {
            std::cout << "  ";
        }
        std::cout << "|";
        Thunk * pt = (*pcontext->thunk_store)[v_t.second];
        std::cout << "MAP " << v_t.first << " -> " << v_t.second << " :: ";
        print_thunk(pt, 0);
        std::cout << std::endl;
        printed.insert(v_t.second);
    }
    for (std::pair<std::string, Thunk*> s_t : *pcontext->thunk_store) {
        if (!CONTAINS(printed, s_t.second->name)) {
            for (int i = 0; i < depth; i++) {
                std::cout << "  ";
            }
            std::cout << "|";
            std::cout << "MAP " << "* -> " << s_t.second->name << " :: ";
            print_thunk(s_t.second, 0);
            std::cout << std::endl;
        }
    }
    // TODO: print data pair sides
}


#endif
