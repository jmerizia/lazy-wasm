#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#include "parse.hpp"
#include "tokenize.hpp"
#include "execute.hpp"
#include "helpers.hpp"

#define LOGGING = true

int main(int argc, char * argv[])
{
    if (argc < 2) error("No input file specified.");

    std::string fname (argv[1]);
    std::ifstream f (fname);
    if (!f) {
        error("Input file does not exist.");
    }

    auto pr = parse_file(f);
    f.close();


    std::vector<struct Expression> expressions = pr.first;
    std::vector<struct Function> functions = pr.second;

    std::cout << "Expressions:" << std::endl;
    for (struct Expression expression : expressions) {
        print_expression_tree(expression, 1);
    }
    std::cout << "Function expressions:" << std::endl;
    for (struct Function function : functions) {
        print_expression_tree(function.expression, 1);
    }

    for (struct Expression expression : expressions) {

        std::map<std::string, Thunk> thunk_store;
        std::map<std::string, std::string> variable_to_thunk;
        std::stack<int> datapair_sides;
        UUID uuid;
        Context context = {
            &functions,
            &thunk_store,
            variable_to_thunk,
            datapair_sides,
            &uuid,
        };

        execute_expression(expression, &context);
    }

    return 0;
}
