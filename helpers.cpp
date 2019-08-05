#ifndef _HELPERS_
#define _HELPERS_

#include <string>

void error(std::string reason) {
    std::cout << "Error: " << reason << std::endl;
    exit(EXIT_FAILURE);
}

#define CONTAINS(s, x) ((s).find(x) != (s).end())
bool IS_WHITESPACE(char c) { return (c == '\n' || c == ' ' || c == '\t'); }
std::string TOKENIZER_BAD_TOKEN_ERROR_MESSAGE(int line_number, int column_number, char c) {
    std::string err = "Bad token \'";
    err += c;
    err += "\' at line ";
    err += std::to_string(line_number);
    err += " column ";
    err += std::to_string(column_number);
    return err;
}

template <class T>
class Optional {
    T value;
    bool defined;
public:
    Optional() {
        defined = false;
        T a;
        value = a;
    }
    bool has_value() {
        return defined;
    }
    T get() {
        if (!defined) {
            error("Internal error: get() can only be called if Optional is defined.");
        }
        return value;
    }
    void set(T _value) {
        defined = true;
        value = _value;
    }
};

void print_context(struct Context& context, int depth) {
    for (pair<std::string, std::string> v_t : context.variable_to_thunk) {
        context.thunk_store[v_t.second];
    }
}

void print_thunk(struct Thunk& thunk) {
    if (thunk.result.has_value()) {
        std::cout << "[thunk] " << thunk.name << " " << \
            thunk.result.get() << " " << ;
    } else {
        std::cout << "[thunk] " <<  << " " << thunk.name << " [no result]" << std::endl;
    }
}

#endif
