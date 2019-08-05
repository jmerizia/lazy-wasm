#ifndef _UUID_
#define _UUID_

#include <string>

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
