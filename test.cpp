#include "tokenizer.cpp"
#include "error.cpp"
#include <string>
#include <vector>

int main() {
    std::vector<pair<string, string>> tokenization_test_files = {
        "test/function_tokenization.lang",
        "test/function_tokenization.out",
    };
    for (auto test_pair : tokenization_test_files) {
        std::ifstream f_lang (fname);
        std::ifstream f_out (fname);
        if (!f_lang || !f_out) {
            error("Test files not found.");
        }
        std::vector<struct Token> tokens = tokenize(f);
        std::string line;
        std::vector<std::pair<std::string, std::string>> lines;
        for (getline(f_out, line)) {

            std::string value = 
        }
        f.close();
    }
}
