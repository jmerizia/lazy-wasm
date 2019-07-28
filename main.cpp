#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include "tokenizer.cpp"
#include "helpers.cpp"

int main(int argc, char * argv[])
{
    if (argc < 2) error("No input file specified.");

    std::string fname (argv[1]);
    std::ifstream f (fname);
    if (!f) {
        error("Input file does not exist.");
    }
    std::vector<struct FileToken> tokens = tokenize_file(f);
    for (struct FileToken token : tokens) {
        std::cout << token.value << " -- " << FileTokenType_map[token.type] << std::endl;
    }
    f.close();

    return 0;
}
