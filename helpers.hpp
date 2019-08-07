#ifndef _HELPERS_H_
#define _HELPERS_H_

#include <string>
#include <iostream>

#define CONTAINS(s, x) ((s).find(x) != (s).end())

//#define LOGGING

void error(std::string);
bool IS_WHITESPACE(char);
std::string TOKENIZER_BAD_TOKEN_ERROR_MESSAGE(int, int, char);

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

class UUID {
    int c;
public:
    UUID() { c = 0; }
    std::string get() {
        c++;
        return "T" + std::to_string(c);
    }
};

#endif
