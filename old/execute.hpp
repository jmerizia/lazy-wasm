#ifndef _EXECUTE_H_
#define _EXECUTE_H_

#include <stack>
#include "parse.hpp"
#include "helpers.hpp"

struct ExecutionResult {
    bool is_nil;
    int value;
};

class Context;
class Thunk {
public:
    struct Expression expression;
    Optional<struct ExecutionResult> result;
    std::string name;
    Context * context;
    Thunk(
        struct Expression expression,
        Optional<struct ExecutionResult> result,
        std::string name,
        Context * context
    ) {
        this->expression = expression;
        this->result = result;
        this->name = name;
        this->context = context;
    }
};

class Context {
public:
    std::vector<struct Function> * functions;
    std::map<std::string, Thunk*> * thunk_store;
    std::map<std::string, std::string> * variable_to_thunk;
    std::stack<int> * datapair_sides;
    UUID * uuid;
    Context(
        std::vector<struct Function> * functions,
        std::map<std::string, Thunk*> * thunk_store,
        UUID * uuid
    ) {
        this->functions = functions;
        this->thunk_store = thunk_store;
        this->uuid = uuid;
        this->variable_to_thunk = new std::map<std::string, std::string>();
        this->datapair_sides = new std::stack<int>();
    }
    ~Context() {
        delete this->variable_to_thunk;
        delete this->datapair_sides;
    }
    Context(const Context& other) {
        // pass reference
        this->functions = other.functions;
        this->thunk_store = other.thunk_store;
        this->uuid = other.uuid;
        // copy
        this->variable_to_thunk =
            new std::map<std::string, std::string>(*other.variable_to_thunk);
        this->datapair_sides = new std::stack<int>(*other.datapair_sides);
    }
    void add_thunk(Thunk * thunk) {
        (*this->thunk_store)[thunk->name] = thunk;
    }
    void remove_thunk(std::string name) {
        (*this->thunk_store).erase((*this->thunk_store).find(name));
    }
    void add_thunk_mapping(std::string variable, std::string thunkid) {
        (*this->variable_to_thunk)[variable] = thunkid;
    }
    Thunk * get_thunk_from_variable(std::string name) {
        if (CONTAINS(*this->variable_to_thunk, name)) {
            return (*this->thunk_store)[(*this->variable_to_thunk)[name]];
        } else {
            std::string msg = "Undefined variable" + name;
            error(msg);
        }
    }
};


struct ExecutionResult execute_expression(struct Expression&, Context*, int);
void print_thunk(Thunk*, int);
void print_context(Context*, int);

#endif
