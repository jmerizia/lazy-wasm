#ifndef _TOKENIZER_
#define _TOKENIZER_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include "helpers.cpp"

enum class FileTokenType {
    function_declaration_name,
    function_declaration_parameter,
    function_declaration_arrow,
    function_declaration_expression,
    expression,
    none,
};

std::map<FileTokenType, std::string> FileTokenType_map = {
    {FileTokenType::function_declaration_name, "function_declaration_name"},
    {FileTokenType::function_declaration_parameter, "function_declaration_parameter"},
    {FileTokenType::function_declaration_arrow, "function_declaration_arrow"},
    {FileTokenType::function_declaration_expression, "function_declaration_expression"},
    {FileTokenType::expression, "expression"},
    {FileTokenType::none, "none"},
};

struct FileToken {
    std::string value;
    FileTokenType type;
};

enum class ExpressionTokenType {
    expression_ternary,
    expression_function_parameter,
    expression_pair_first,
    expression_pair_second,
    ternary_if,
    ternary_condition,
    ternary_then,
    ternary_first,
    ternary_else,
    ternary_second,
    function_call_name,
    function_call_parameter,
    pair_first,
    pair_second,
    unknown,
    none,
};

std::map<ExpressionTokenType, std::string> ExpressionTokenType_map = {
    {ExpressionTokenType::expression_ternary, "expression_ternary"},
    {ExpressionTokenType::expression_function_parameter, "expression_function_parameter"},
    {ExpressionTokenType::expression_pair_first, "expression_pair_first"},
    {ExpressionTokenType::expression_pair_second, "expression_pair_second"},
    {ExpressionTokenType::ternary_if, "ternary_if"},
    {ExpressionTokenType::ternary_condition, "ternary_condition"},
    {ExpressionTokenType::ternary_then, "ternary_then"},
    {ExpressionTokenType::ternary_first, "ternary_first"},
    {ExpressionTokenType::ternary_else, "ternary_else"},
    {ExpressionTokenType::ternary_second, "ternary_second"},
    {ExpressionTokenType::function_call_name, "function_call_name"},
    {ExpressionTokenType::function_call_parameter,"function_call_parameter"},
    {ExpressionTokenType::pair_first, "pair_first"},
    {ExpressionTokenType::pair_second, "pair_second"},
    {ExpressionTokenType::unknown, "unknown"},
    {ExpressionTokenType::none, "none"},
};

enum class ExpressionTokenTypeHelper {
    expression,
    ternary,
    function_call,
    pair,
    unknown,
}

struct ExpressionToken {
    std::string value;
    ExpressionTokenType type;
};

enum class ExpressionType {
    ternary,  // (if () then () else ())
    function_call,  // (fname p1 p2 (p3))
    datapair,  // [(), ()]
    base,  // no sub-expressions
};

/*
 * A parsed expression that holds reference to
 * its thunks.
 */
struct Expression {
    ExpressionType type;
    std::vector<struct Expression> expressions;
};

/*
 * A callable function containing a name and an expression.
 */
struct Function {
    std::string name;
    struct Expression expression;
};


/* 
 * Reads a string and parses it into Tokens.
 * The string must be a valid expression.
 */
std::vector<struct ExpressionToken>
tokenize_expression(std::string s) {
    std::vector<struct ExpressionToken> tokens;
    std::string cur;
    ExpressionTokenTypeHelper cur_type = ExpressionTokenTypeHelper::unknown;
    /*
    expression_ternary,
    expression_function_parameter,
    expression_pair_first,
    expression_pair_second,
    ternary_if,
    ternary_condition,
    ternary_then,
    ternary_first,
    ternary_else,
    ternary_second,
    function_call_name,
    function_call_parameter,
    pair_first,
    pair_second,
    unknown,
    none,
    */
    /*
    expression,
    ternary,
    function_call,
    pair,
    unknown,
    */
    char paren_count = 0;
    char bracket_count = 0;
    char prev_c = 0;

    for (char c : s) {
        if (IS_WHITESPACE(c)) {
            if (!IS_WHITESPACE(prev_c)) {
                switch (cur_type) {
                    case ExpressionTokenType::expression:
                        break;
                    case ExpressionTokenType::ternary_if:
                        break;
                    case ExpressionTokenType::ternary_condition:
                        break;
                    case ExpressionTokenType::ternary_then:
                        break;
                    case ExpressionTokenType::ternary_first:
                        break;
                    case ExpressionTokenType::ternary_else:
                        break;
                    case ExpressionTokenType::ternary_second:
                        break;
                    case ExpressionTokenType::function_call_name:
                        break;
                    case ExpressionTokenType::function_call_parameter:
                        break;
                    case ExpressionTokenType::pair_first:
                        break;
                    case ExpressionTokenType::pair_second:
                        break;
                    case ExpressionTokenType::unknown:
                        break;
                    case ExpressionTokenType::none:
                        // looking at the edge of a word for an unknown expression
                        if (cur == "if") {
                            cur_type = ExpressionTokenType::ternary_if;
                        } else {
                            cur_type = ExpressionTokenType::function_call;
                        }
                        std::string value = cur;
                        struct ExpressionToken token = {value, cur_type};
                        tokens.push_back(token);
                        cur.clear();

                        if (cur_type == ExpressionTokenType::ternary_if) {
                            cur_type = ExpressionTokenType::ternary_condition;
                        } else {
                            cur_type = ExpressionTokenType::function_call;
                        }

                        break;
                }

            } else {
                // pass, to skip all occurances of 2+ spaces
            }

        } else if (c == '(') {
            switch (cur_type) {
                case ExpressionTokenTypeHelper::expression:
                    break;
                case ExpressionTokenTypeHelper::ternary:
                    break;
                case ExpressionTokenTypeHelper::function_call:
                    break;
                case ExpressionTokenTypeHelper::pair:
                    break;
                case ExpressionTokenTypeHelper::unknown:
                    break;
            }

        } else if (c == ')') {
            switch (cur_type) {
                case ExpressionTokenTypeHelper::expression:
                    break;
                case ExpressionTokenTypeHelper::ternary:
                    break;
                case ExpressionTokenTypeHelper::function_call:
                    break;
                case ExpressionTokenTypeHelper::pair:
                    break;
                case ExpressionTokenTypeHelper::unknown:
                    break;
            }

        } else if (c == '[') {
            switch (cur_type) {
                case ExpressionTokenTypeHelper::expression:
                    break;
                case ExpressionTokenTypeHelper::ternary:
                    break;
                case ExpressionTokenTypeHelper::function_call:
                    break;
                case ExpressionTokenTypeHelper::pair:
                    bracket_count++;
                    break;
                case ExpressionTokenTypeHelper::unknown:
                    cur_type = ExpressionTokenTypeHelper::pair;
                    bracket_count = 1;
                    break;
            }

        } else if (c == ']') {
            switch (cur_type) {
                case ExpressionTokenTypeHelper::expression:
                    break;
                case ExpressionTokenTypeHelper::ternary:
                    break;
                case ExpressionTokenTypeHelper::function_call:
                    break;
                case ExpressionTokenTypeHelper::pair:
                    bracket_count--;
                    if (bracket_count == 0) {

                    }
                    break;
                case ExpressionTokenTypeHelper::unknown:
                    //error
                    break;
            }

        } else if (c == ',') {
            switch (cur_type) {
                case ExpressionTokenTypeHelper::expression:
                    break;
                case ExpressionTokenTypeHelper::ternary:
                    break;
                case ExpressionTokenTypeHelper::function_call:
                    break;
                case ExpressionTokenTypeHelper::pair:

                    break;
                case ExpressionTokenTypeHelper::unknown:
                    break;
            }

        } else if (/* [A-Za-z_] */
                (65 <= c && c <= 90) ||
                (97 <= c && c <= 122) ||
                (c == 95)) {
            switch (cur_type) {
                case ExpressionTokenTypeHelper::expression:
                    break;
                case ExpressionTokenTypeHelper::ternary:
                    break;
                case ExpressionTokenTypeHelper::function_call:
                    break;
                case ExpressionTokenTypeHelper::pair:
                    break;
                case ExpressionTokenTypeHelper::unknown:
                    cur += c;
                    break;
            }
        } else {

        }

        prev_c = c;
    }

    return tokens;
}

/* 
 * Reads a file and parses it into Tokens.
 */
std::vector<struct FileToken>
tokenize_file(std::ifstream& f)
{
    std::vector<struct FileToken> tokens;
    std::string cur;
    FileTokenType cur_type = FileTokenType::none;
    int line_number = 1;
    int column_number = 1;
    char prev_c = 0;
    int paren_count = 0;
    /*
    function_declaration_name,
    function_declaration_parameter,
    function_declaration_arrow,
    function_declaration_expression,
    expression,
    none,
     */
    char c;
    while (f.get(c)) {
        if (IS_WHITESPACE(c)) {
            if (!IS_WHITESPACE(prev_c)) {
                switch (cur_type) {
                    case FileTokenType::function_declaration_name:
                        {
                            std::string value = cur;
                            struct FileToken token = {value, cur_type};
                            tokens.push_back(token);
                            cur_type = FileTokenType::function_declaration_parameter;
                            cur.clear();
                            break;
                        }
                    case FileTokenType::function_declaration_parameter:
                        {
                            std::string value = cur;
                            struct FileToken token = {value, cur_type};
                            tokens.push_back(token);
                            cur.clear();
                            break;
                        }
                    case FileTokenType::function_declaration_arrow:
                        {
                            if (prev_c != '>') {
                                error(TOKENIZER_BAD_TOKEN_ERROR_MESSAGE(line_number, column_number, c));
                            }
                            std::string value = cur;
                            struct FileToken token = {value, cur_type};
                            tokens.push_back(token);
                            cur.clear();
                            cur_type = FileTokenType::function_declaration_expression;
                            break;
                        }
                    case FileTokenType::function_declaration_expression:
                        cur += ' ';
                        break;
                    case FileTokenType::expression:
                        cur += ' ';
                        break;
                    case FileTokenType::none:
                        // pass, we are not in any block
                        break;
                }

            } else {
                // pass, to skip all occurances of 2+ spaces
            }

        } else if (c == '(') {
            switch (cur_type) {
                case FileTokenType::function_declaration_name:
                    break;
                case FileTokenType::function_declaration_arrow:
                    cur_type = FileTokenType::function_declaration_expression;
                    cur += c;
                    paren_count = 1;
                    break;
                case FileTokenType::function_declaration_parameter:
                    break;
                case FileTokenType::function_declaration_expression:
                    paren_count++;
                    cur += c;
                    break;
                case FileTokenType::expression:
                    paren_count++;
                    cur += c;
                    break;
                case FileTokenType::none:
                    cur += c;
                    cur_type = FileTokenType::expression;
                    paren_count = 1;
                    break;
            }

        } else if (c == ')') {
            switch (cur_type) {
                case FileTokenType::function_declaration_name:
                    cur += c;
                    break;
                case FileTokenType::function_declaration_arrow:
                    break;
                case FileTokenType::function_declaration_parameter:
                    cur += c;
                    break;
                case FileTokenType::function_declaration_expression:
                    paren_count--;
                    cur += c;
                    if (paren_count == 0) {
                        std::string value = cur;
                        struct FileToken token = {value, cur_type};
                        tokens.push_back(token);
                        cur.clear();
                        cur_type = FileTokenType::none;
                    }
                    break;
                case FileTokenType::expression:
                    paren_count--;
                    cur += c;
                    if (paren_count == 0) {
                        std::string value = cur;
                        struct FileToken token= {value, cur_type};
                        tokens.push_back(token);
                        cur.clear();
                        cur_type = FileTokenType::none;
                    }
                    break;
                case FileTokenType::none:
                    cur_type = FileTokenType::function_declaration_name;
                    cur += c;
                    break;
            }

        } else if (/* [A-Za-z_] */
                (65 <= c && c <= 90) ||
                (97 <= c && c <= 122) ||
                (c == 95)) {
            switch (cur_type) {
                case FileTokenType::function_declaration_name:
                    cur += c;
                    break;
                case FileTokenType::function_declaration_arrow:
                    error(TOKENIZER_BAD_TOKEN_ERROR_MESSAGE(line_number, column_number, c));
                    break;
                case FileTokenType::function_declaration_parameter:
                    cur += c;
                    break;
                case FileTokenType::function_declaration_expression:
                    cur += c;
                    break;
                case FileTokenType::expression:
                    cur += c;
                    break;
                case FileTokenType::none:
                    cur_type = FileTokenType::function_declaration_name;
                    cur += c;
                    break;
            }

        } else if (c == '=') {
            switch (cur_type) {
                case FileTokenType::function_declaration_name:
                    error(TOKENIZER_BAD_TOKEN_ERROR_MESSAGE(line_number, column_number, c));
                    break;
                case FileTokenType::function_declaration_arrow:
                    error(TOKENIZER_BAD_TOKEN_ERROR_MESSAGE(line_number, column_number, c));
                    break;
                case FileTokenType::function_declaration_parameter:
                    if (!IS_WHITESPACE(prev_c)) {
                        // the parameter has not been finished
                        error(TOKENIZER_BAD_TOKEN_ERROR_MESSAGE(line_number, column_number, c));
                    } else {
                        cur += c;
                        cur_type = FileTokenType::function_declaration_arrow;
                    }
                    break;
                case FileTokenType::function_declaration_expression:
                    cur += c;
                    break;
                case FileTokenType::expression:
                    cur += c;
                    break;
                case FileTokenType::none:
                    error(TOKENIZER_BAD_TOKEN_ERROR_MESSAGE(line_number, column_number, c));
                    break;
            }

        } else if (c == '>') {
            switch (cur_type) {
                case FileTokenType::function_declaration_name:
                    break;
                case FileTokenType::function_declaration_arrow:
                    if (prev_c != '=') {
                        error(TOKENIZER_BAD_TOKEN_ERROR_MESSAGE(line_number, column_number, c));
                    } else {
                        cur += c;
                    }
                    break;
                case FileTokenType::function_declaration_parameter:
                    error(TOKENIZER_BAD_TOKEN_ERROR_MESSAGE(line_number, column_number, c));
                    break;
                case FileTokenType::function_declaration_expression:
                    cur += c;
                    break;
                case FileTokenType::expression:
                    cur += c;
                    break;
                case FileTokenType::none:
                    break;
            }

        } else {
            switch (cur_type) {
                case FileTokenType::function_declaration_name:
                    error(TOKENIZER_BAD_TOKEN_ERROR_MESSAGE(line_number, column_number, c));
                    break;
                case FileTokenType::function_declaration_arrow:
                    error(TOKENIZER_BAD_TOKEN_ERROR_MESSAGE(line_number, column_number, c));
                    break;
                case FileTokenType::function_declaration_parameter:
                    error(TOKENIZER_BAD_TOKEN_ERROR_MESSAGE(line_number, column_number, c));
                    break;
                case FileTokenType::function_declaration_expression:
                    cur += c;
                    break;
                case FileTokenType::expression:
                    cur += c;
                    break;
                case FileTokenType::none:
                    error(TOKENIZER_BAD_TOKEN_ERROR_MESSAGE(line_number, column_number, c));
                    break;
            }
        }

        if (c == '\n') {
            line_number++;
            column_number = 1;
        } else {
            column_number++;
        }

        prev_c = c;

    }
    return tokens;
}

#endif
