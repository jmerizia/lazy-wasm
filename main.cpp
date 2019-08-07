#include <string>
#include <vector>
#include <fstream>
#include <iostream>

#include "parse.hpp"
#include "tokenize.hpp"
#include "execute.hpp"
#include "helpers.hpp"

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

#ifdef LOGGING
    std::cout << "Expressions:" << std::endl;
    for (struct Expression expression : expressions) {
        print_expression_tree(expression, 1);
    }
    std::cout << "Function expressions:" << std::endl;
    for (struct Function function : functions) {
        print_expression_tree(function.expression, 1);
    }
#endif

    for (struct Expression expression : expressions) {

        Context * context = new Context();
        for (struct Function function : functions) {
            context->add_function(function);
        }

        execute_expression(expression, context, 0);

        delete context;
    }

    return 0;
}
