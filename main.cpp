#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include "parse.cpp"
#include "tokenize.cpp"
#include "execute.cpp"
#include "helpers.cpp"

int main(int argc, char * argv[])
{
    if (argc < 2) error("No input file specified.");

    std::string fname (argv[1]);
    std::ifstream f (fname);
    if (!f) {
        error("Input file does not exist.");
    }

    //struct Expression expression = parse_expression_string("(nil)");
    //std::vector<struct Function> functions;
    //std::vector<struct Variable> variables;
    //struct ExecutionResult result = execute_expression(expression, functions, variables, 0);
    //if (result.is_nil) {
    //    std::cout << "nil" << std::endl;
    //} else {
    //    std::cout << result.value << std::endl;
    //}

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

    std::vector<struct Variable> variables;
    struct ExecutionResult result = execute_expression(expressions[0], functions, variables, 0);
    std::cout << "Result: ";
    if (result.is_nil) {
        std::cout << "nil" << std::endl;
    } else {
        std::cout << result.value << std::endl;
    }

    return 0;
}
