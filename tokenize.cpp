#ifndef _TOKENIZE_
#define _TOKENIZE_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <regex>

#include "helpers.hpp"
#include "tokenize.hpp"

std::map<FileTokenType, std::string> FileTokenType_map = {
    {FileTokenType::function_declaration_name, "function_declaration_name"},
    {FileTokenType::function_declaration_parameter, "function_declaration_parameter"},
    {FileTokenType::function_declaration_arrow, "function_declaration_arrow"},
    {FileTokenType::function_declaration_expression, "function_declaration_expression"},
    {FileTokenType::expression, "expression"},
    {FileTokenType::comment, "comment"},
    {FileTokenType::none, "none"},
};

std::map<ExpressionTokenType, std::string> ExpressionTokenType_map = {
    {ExpressionTokenType::expression, "expression"},
    {ExpressionTokenType::function_call_name, "function_call_name"},
    {ExpressionTokenType::function_call_parameter,"function_call_parameter"},
    {ExpressionTokenType::base, "base"},
    {ExpressionTokenType::variable, "variable"},
    {ExpressionTokenType::none, "none"},
};

/* 
 * Reads a string and parses it into Tokens.
 * The string must be a valid expression.
 */
std::vector<struct ExpressionToken> tokenize_expression(std::string s) {
    std::vector<struct ExpressionToken> tokens;

    // function (required parens): ^\s*\(\s*([\w0-9]+)\s+(.+)\s*\)\s*$
    // variable (optional parens): ^\s*\(?\s*(\w+)\s*\)?\s*$
    // base (optional parens): ^\s*\(?\s*(-?\d+|nil)\s*\)?\s*$
    std::regex function_re ("^\\s*\\(\\s*([\\w0-9]+)\\s+(.+)\\s*\\)\\s*$");
    std::regex variable_re ("^\\s*\\(?\\s*(\\w+)\\s*\\)?\\s*$");
    std::regex base_re ("^\\s*\\(?\\s*(-?\\d+|nil|'.+')\\s*\\)?\\s*$");
    std::smatch match;

    if (std::regex_search(s, match, function_re)) {
        if (match.size() == 3) {
            tokens.push_back({match.str(1), ExpressionTokenType::function_call_name});
            // need to parse parameters
            std::string params = match.str(2);
            std::string cur;
            int paren_count = 0;
            int i;
            for (i = 0; i < (int)params.size(); i++) {
                if (paren_count == 0 /* not in expression */) {
                    if (IS_WHITESPACE(params[i]) && cur.size() > 0) {
                        //std::string value = cur;
                        tokens.push_back({cur, ExpressionTokenType::function_call_parameter});
                        cur.clear();
                    }
                    while (i < (int)params.size() && IS_WHITESPACE(params[i])) {
                        i++;
                    }
                }
                if (i != (int)params.size()) {
                    cur += params[i];
                    if (params[i] == '(') {
                        paren_count++;
                    } else if (params[i] == ')') {
                        paren_count--;
                    } else {
                        // we should be looking at a [\w0-9] char
                    }
                }
            }
            if (cur.size()) {
                tokens.push_back({cur, ExpressionTokenType::function_call_parameter});
            }
        } else {
            std::string msg = "Invalid expression \"";
            if (s.size() > 30) {
                msg += s.substr(30);
                msg += "...";
            } else {
                msg += s;
            }
            msg += "\"";
            error(msg);
        }

    } else if (std::regex_search(s, match, base_re)) {
        if (match.size() == 2) {
            tokens.push_back({match.str(1), ExpressionTokenType::base});
        } else {
            std::string msg = "Invalid expression \"";
            if (s.size() > 30) {
                msg += s.substr(30);
                msg += "...";
            } else {
                msg += s;
            }
            msg += "\"";
            error(msg);
        }
    } else if (std::regex_search(s, match, variable_re)) {
        if (match.size() == 2) {
            tokens.push_back({match.str(1), ExpressionTokenType::variable});
        } else {
            std::string msg = "Invalid expression \"";
            if (s.size() > 30) {
                msg += s.substr(30);
                msg += "...";
            } else {
                msg += s;
            }
            msg += "\"";
            error(msg);
        }
    } else {
        std::string msg = "Failed to match expression \"";
        if (s.size() > 30) {
            msg += s.substr(0, 30);
            msg += "...";
        } else {
            msg += s;
        }
        msg += "\"";
        error(msg);
    }

    return tokens;
}

/* 
 * Reads a file and parses it into Tokens.
 */
std::vector<struct FileToken> tokenize_file(std::ifstream& f)
{
    std::vector<struct FileToken> tokens;
    std::string cur;
    FileTokenType cur_type = FileTokenType::none;
    FileTokenType prev_type = FileTokenType::none;
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
                            prev_type = cur_type;
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
                            prev_type = cur_type;
                            cur_type = FileTokenType::function_declaration_expression;
                            break;
                        }
                    case FileTokenType::function_declaration_expression:
                        cur += ' ';
                        break;
                    case FileTokenType::expression:
                        cur += ' ';
                        break;
                    case FileTokenType::comment:
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
                    prev_type = cur_type;
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
                case FileTokenType::comment:
                    break;
                case FileTokenType::none:
                    cur += c;
                    prev_type = cur_type;
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
                        prev_type = cur_type;
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
                        prev_type = cur_type;
                        cur_type = FileTokenType::none;
                    }
                    break;
                case FileTokenType::comment:
                    break;
                case FileTokenType::none:
                    prev_type = cur_type;
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
                case FileTokenType::comment:
                    break;
                case FileTokenType::none:
                    prev_type = cur_type;
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
                        prev_type = cur_type;
                        cur_type = FileTokenType::function_declaration_arrow;
                    }
                    break;
                case FileTokenType::function_declaration_expression:
                    cur += c;
                    break;
                case FileTokenType::expression:
                    cur += c;
                    break;
                case FileTokenType::comment:
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
                case FileTokenType::comment:
                    break;
                case FileTokenType::none:
                    break;
            }

        } else if (c == '-') {
            switch (cur_type) {
                case FileTokenType::function_declaration_name:
                    error(TOKENIZER_BAD_TOKEN_ERROR_MESSAGE(line_number, column_number, c));
                    break;
                case FileTokenType::function_declaration_arrow:
                    error(TOKENIZER_BAD_TOKEN_ERROR_MESSAGE(line_number, column_number, c));
                    break;
                case FileTokenType::function_declaration_parameter:
                    if (prev_c == '-') {
                        prev_type = cur_type;
                        cur_type = FileTokenType::comment;
                    }
                    break;
                case FileTokenType::function_declaration_expression:
                    if (prev_c == '-') {
                        prev_type = cur_type;
                        cur_type = FileTokenType::comment;
                    }
                    break;
                case FileTokenType::expression:
                    if (prev_c == '-') {
                        prev_type = cur_type;
                        cur_type = FileTokenType::comment;
                    }
                    break;
                case FileTokenType::comment:
                    break;
                case FileTokenType::none:
                    if (prev_c == '-') {
                        prev_type = cur_type;
                        cur_type = FileTokenType::comment;
                    }
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
                case FileTokenType::comment:
                    break;
                case FileTokenType::none:
                    error(TOKENIZER_BAD_TOKEN_ERROR_MESSAGE(line_number, column_number, c));
                    break;
            }
        }

        if (c == '\n') {
            line_number++;
            column_number = 1;
            if (cur_type == FileTokenType::comment) {
                cur_type = prev_type;
            }
        } else {
            column_number++;
        }

        prev_c = c;

    }
    return tokens;
}

#endif
