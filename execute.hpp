#ifndef _EXECUTE_H_
#define _EXECUTE_H_

#include <stack>
#include "parse.hpp"
#include "helpers.hpp"

struct ExecutionResult {
    bool is_nil;
    int value;
};

class Thunk;
class Context {
public:
    std::vector<struct Function> * functions;
    std::map<std::string, Thunk> * thunk_store;
    std::map<std::string, std::string> variable_to_thunk;
    std::stack<int> datapair_sides;
    UUID * uuid;
};

class Thunk {
public:
    struct Expression expression;
    Optional<struct ExecutionResult> result;
    std::string name;
    Context context;
};

struct ExecutionResult execute_expression(struct Expression&, Context*);
void print_thunk(Thunk*);
void print_context(Context*);

#endif
