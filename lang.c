/*
 *   A simple functional programming language.
 *
 *   by Jacob Merizian
 *   License: MIT
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>

#define MAX_INPUT_LEN 100000
#define SINGLE_CHAR_TOKENS "()[],+-*/=?:"
#define SYSTEM_FUNCTION_TOKENS "+-*/=?%:"
#define streq(a, b) (strcmp((a), (b)) == 0)
#define DEBUG

/*******************
 *     HELPERS     *
 *******************/

bool member_of(char c, char * set)
{
    for (int i = 0; set[i] != '\0'; i++)
        if (c == set[i])
            return true;
    return false;
}

void expect(bool condition, char * msg, ...)
{
    if (!condition) {
        va_list argptr;
        va_start(argptr, msg);
        vfprintf(stderr, msg, argptr);
        exit(EXIT_FAILURE);
        va_end(argptr);
    }
}

void substring(char * dst, char * src, int l, int r)
{
    int n = strlen(src) + 1, j = 0;
    for (int i = l; i < r && i < n && src[i] != '\0'; i++) {
        dst[j++] = src[i];
    }
    dst[j] = '\0';
}

char * new_string(size_t sz)
{
    return malloc((sz + 1) * sizeof(char));
}

char * clone_string(char * s1)
{
    char * s2 = new_string(strlen(s1));
    strcpy(s2, s1);
    return s2;
}

void destroy_string(char * s)
{
    free(s);
}

char * read_file(char * fname)
{
    char * line;
    FILE * fp;
    size_t len = 0;
    char * input = new_string(MAX_INPUT_LEN);

    fp = fopen(fname, "r");
    expect(fp != NULL, "Error: Failed to open file %s.", fname);
    while (getline(&line, &len, fp) != -1) {
        strcat(input, line);
    }
    fclose(fp);
    if (line) free(line);
    return input;
}

/*******************
 *      QUEUE      *
 *******************/

struct Node;
struct Queue;

struct Node {
    void * data;
    struct Node * next;
    struct Node * prev;
} typedef Node;

struct Queue {
    Node * head;
    Node * tail;
} typedef Queue;

/* function headers */
Queue * new_queue(Queue * q_old);
void destroy_queue(Queue * q);
void queue_push(Queue * q, void * data);
size_t queue_size(Queue * q);

Node * new_node()
{
    return malloc(sizeof(Node));
}

void destroy_node(Node * node)
{
    free(node);
}

Node * queue_begin(Queue * q) { return q->head->next; }
Node * queue_end(Queue * q) { return q->tail; }
#define queue_foreach(e, q) \
    for (Node * e = q->head->next, * end = q->tail; e != end; e = e->next)
#define queue_foreach_reverse(e, q) \
    for (Node * e = q->tail->prev, * end = q->head; e != end; e = e->prev)

Queue * new_queue(Queue * q_old)
{
    Queue * q = malloc(sizeof(Queue));
    q->head = malloc(sizeof(Node));
    q->tail = malloc(sizeof(Node));
    q->head->data = NULL;
    q->tail->data = NULL;
    q->head->next = q->tail;
    q->tail->prev = q->head;
    q->head->prev = NULL;
    q->tail->next = NULL;
    if (q_old != NULL) {  // clone q_old
        queue_foreach(node, q_old) {
            queue_push(q, node->data);
        }
    }
    return q;
}

void destroy_queue(Queue * q)
{
    Node * cur, * next;
    for (cur = q->head; cur != q->tail; ) {
        next = cur->next;
        destroy_node(cur);
        cur = next;
    }
    free(q->tail);
    free(q);
}

void queue_push(Queue * q, void * data)
{
    Node * n = new_node();
    n->data = data;
    q->tail->prev->next = n;
    n->prev = q->tail->prev;
    q->tail->prev = n;
    n->next = q->tail;
}

void queue_erase(Node * node)
{
    expect(node->prev != NULL, "Error (internal): queue_erase.\n");
    expect(node->next != NULL, "Error (internal): queue_erase.\n");
    Node * n = node->next;
    Node * p = node->prev;
    p->next = n;
    n->prev = p;
    destroy_node(node);
}

size_t queue_size(Queue * q)
{
    size_t len = 0;
    queue_foreach(node, q) len++;
    return len;
}

/*******************
 *   EXPRESSIONS   *
 *******************/

enum ExpressionType;
struct Expression;

enum PrimitiveType {
    PrimitiveANY = 0,
    PrimitiveTRUE = 1,
    PrimitiveFALSE = 2,
    PrimitiveNULL = 3,
    PrimitiveNumber = 4,
    PrimitiveString = 5,
} typedef PrimitiveType;
char * PrimitiveTypeString[6] = {
    "PrimitiveANY",
    "PrimitiveTRUE",
    "PrimitiveFALSE",
    "PrimitiveNULL",
    "PrimitiveNumber",
    "PrimitiveString",
};

enum ExpressionType {
    Program = 0,
    Statement = 1,
    List = 2,
    Id = 3,
    Primitive = 4,
} typedef ExpressionType;
char * ExpressionTypeString[5] = {
    "Program",
    "Statement",
    "List",
    "Id",
    "Primitive",
};

struct Expression {
    char * value;
    Queue/*<Expression>*/ * children;
    ExpressionType type;
    PrimitiveType ptype;
} typedef Expression;

Expression * new_expression(char * value, ExpressionType type, PrimitiveType ptype);
void destroy_expression(Expression * e);
void print_expression(Expression * e, int d);

Expression * new_expression(char * value, ExpressionType type, PrimitiveType ptype)
{
    Expression * e = malloc(sizeof(Expression));
    e->children = new_queue(NULL);
    if (value != NULL) {
        e->value = clone_string(value);
    } else {
        e->value = NULL;
    }
    e->type = type;
    e->ptype = ptype;
    return e;
}

void destroy_expression(Expression * e)
{
    queue_foreach(node, e->children) {
        destroy_expression(node->data);
    }
    destroy_queue(e->children);
    free(e->value);
    free(e);
}

void print_expression(Expression * e, int d)
{
    for (int i = 0; i < d; i++) printf("  ");
    printf("%s : %s (%s)\n",
            ExpressionTypeString[e->type],
            e->value == NULL ? "0" : e->value,
            e->type == Primitive ? PrimitiveTypeString[e->ptype] : "-");
    queue_foreach(node, e->children) {
        print_expression(node->data, d+1);
    }
}


/*******************
 *     LEXING      *
 *******************/

struct Lexer {
    char * input;
    int idx;
    int prev_idx;
} typedef Lexer;

Lexer * new_lexer(char * input)
{
    Lexer * lex = malloc(sizeof(Lexer));
    lex->input = input;
    lex->idx = 0;
    lex->prev_idx = -1;
    return lex;
}

void destroy_lexer(Lexer * lex)
{
    free(lex);
}

char * lexer_seek(Lexer * lex)
{
    int l, r, i = lex->idx;
    char * token;

    // skip whitespace
    while (member_of(lex->input[i], " \r\n\t")) i++;

    // skip comment
    while (lex->input[i] == '#') {
        while (lex->input[i] != '\n' && lex->input[i] != '\0') i++;
        if (lex->input[i] == '\n') i++;

        // skip whitespace after comment
        while (member_of(lex->input[i], " \r\n\t")) i++;
    }

    // if no token left
    if (lex->input[i] == '\0') return NULL;

    l = i;

    // if single character token
    if (member_of(lex->input[i], SINGLE_CHAR_TOKENS)) {
        i++;

    } else if (lex->input[i] == '\"') { // string token
        // TODO: escape sequences!
        i++;
        while (lex->input[i] != '\0' && lex->input[i] != '\"') {
            i++;
        }
        if (lex->input[i] == '\"') {
            i++;
        } else {
            printf("Error: Unterminated string\n");
        }

    } else { // multi character token
        while (lex->input[i] != '\0'
                && !member_of(lex->input[i], SINGLE_CHAR_TOKENS)
                && !isspace(lex->input[i])
                && lex->input[i] != '#') {
            i++;
        }
    }
    r = i;

    // check length not zero
    expect(l < r, "Token should not be empty string!\n");

    token = new_string(r-l+1);
    substring(token, lex->input, l, r);

    lex->prev_idx = lex->idx;
    lex->idx = i;
    return token;
}

void lexer_back(Lexer * lex)
{
    expect(lex->prev_idx != -1, "Error: Lexer: Invalid call to lexer_back().\n");
    lex->idx = lex->prev_idx;
    lex->prev_idx = -1;
}


/*******************
 *     PARSING     *
 *******************/

Expression * parse_program(Lexer * lex);
bool parse_statement (Expression * root, Lexer * lex);
bool parse_list      (Expression * root, Lexer * lex);
bool parse_id        (Expression * root, Lexer * lex);
bool parse_primitive (Expression * root, Lexer * lex);

bool parse_primitive(Expression * root, Lexer * lex)
{
    char * token = lexer_seek(lex);
    expect(token != NULL, "Error: Expected token.\n");
    int len = strlen(token);
    bool is_num = strcmp(token, "0") == 0 ? true : atoi(token) != 0;
    bool is_str = token[0] == '\"' && token[len-1] == '\"';
    bool is_any = (strcmp(token, "ANY") == 0);
    bool is_true = (strcmp(token, "TRUE") == 0);
    bool is_false = (strcmp(token, "FALSE") == 0);
    bool is_null = (strcmp(token, "NULL") == 0);
    if (!(is_num || is_str || is_any || is_true || is_false || is_null)) {
        lexer_back(lex);
        destroy_string(token);
        return false;
    }
    PrimitiveType ptype;
    if      (is_num)   ptype = PrimitiveNumber;
    else if (is_str)   ptype = PrimitiveString;
    else if (is_any)   ptype = PrimitiveANY;
    else if (is_true)  ptype = PrimitiveTRUE;
    else if (is_false) ptype = PrimitiveFALSE;
    else if (is_null)  ptype = PrimitiveNULL;
    Expression * e;
    if (is_str) {
        int sz = strlen(token);
        char * tmp = new_string(sz);
        substring(tmp, token, 1, sz-1);
        e = new_expression(tmp, Primitive, ptype);
        free(tmp);
    } else {
        e = new_expression(token, Primitive, ptype);
    }
    queue_push(root->children, e);
    destroy_string(token);
    return true;
}

bool parse_id(Expression * root, Lexer * lex)
{
    char * token = lexer_seek(lex);
    expect(token != NULL, "Error: Expected token.\n");
    Expression * e = new_expression(token, Id, PrimitiveANY);
    bool is_id = true;
    for (int i = 0; token[i] != '\0'; i++) {
        if (!(isalpha(token[i]) ||
                    member_of(token[i], SYSTEM_FUNCTION_TOKENS) ||
                    token[i] == '_')) {
            is_id = false;
            break;
        }
    }
    if (!is_id) {
        lexer_back(lex);
        destroy_expression(e);
        destroy_string(token);
        return false;
    }
    queue_push(root->children, e);
    destroy_string(token);
    return true;
}

bool parse_list(Expression * root, Lexer * lex)
{
    char * token = lexer_seek(lex);
    expect(token != NULL, "Error: Expected token.\n");
    Expression * e = new_expression(NULL, List, PrimitiveANY);
    if (strcmp(token, "[") != 0) {
        lexer_back(lex);
        destroy_expression(e);
        destroy_string(token);
        return false;
    }
    while (parse_id(e, lex) ||
           parse_statement(e, lex) ||
           parse_list(e, lex) ||
           parse_primitive(e, lex));
    token = lexer_seek(lex);
    expect(strcmp(token, "]") == 0, "Error: Expected closing paren!\n");
    e->value = NULL;
    e->type = List;
    queue_push(root->children, e);
    destroy_string(token);
    return true;
}

bool parse_statement(Expression * root, Lexer * lex)
{
    char * token = lexer_seek(lex);
    if (token == NULL) {
        destroy_string(token);
        return false;
    }
    Expression * e = new_expression(NULL, Statement, PrimitiveANY);
    if (strcmp(token, "(") != 0 || !parse_id(e, lex)) {
        lexer_back(lex);
        destroy_expression(e);
        destroy_string(token);
        return false;
    }
    while (parse_primitive(e, lex) ||
           parse_id(e, lex) ||
           parse_statement(e, lex) ||
           parse_list(e, lex));
    token = lexer_seek(lex);
    expect(strcmp(token, ")") == 0, "Error: Expected closing paren!\n");
    queue_push(root->children, e);
    destroy_string(token);
    return true;
}

Expression * parse_program(Lexer * lex)
{
    Expression * e = new_expression(NULL, Program, PrimitiveANY);
    while (parse_statement(e, lex));
    return e;
}


/*******************
 *    EXECUTION    *
 *******************/

/* lifecycle:
 *   TODO: Figure out when to create/destroy these. GC??
 *   *** Result can only be used once ***
 *   TODO: Add function as a PrimitiveType so we can have first-class functions
 */
struct Result {
    long long num;
    PrimitiveType type;
    char * str;
} typedef Result;

/* lifecycle:
 *   - Creation: When statement "def" is executed
 *   - Destruction: Program termination
 *   TODO: Make functions scoped locally to a thunk
 */
struct Function {
    char * name;
    Queue/*<char *>*/ * params;
    Expression * def;
} typedef Function;
Queue/*<Function>*/ * ftable;

/* lifecycle:
 *   - Creation: Before execution call
 *   - Destruction: After execution returns
 */
struct Thunk {
    char * name;  // variable name
    Expression * e;
    Result * res;
    Queue/*<Thunk>*/ * context;
} typedef Thunk;

Result * new_result(long long num, char * str, PrimitiveType type)
{
    Result * res = malloc(sizeof(Result));
    res->num = num;
    res->type = type;
    if (str != NULL) {
        res->str = clone_string(str);
    } else {
        res->str = NULL;
    }
    return res;
}

void destroy_result(Result * res)
{
    free(res->str);
    free(res);
}

void print_result(Result * res)
{
    if      (res->type == PrimitiveANY)    printf("ANY\n");
    else if (res->type == PrimitiveTRUE)   printf("TRUE\n");
    else if (res->type == PrimitiveFALSE)  printf("FALSE\n");
    else if (res->type == PrimitiveNULL)   printf("NULL\n");
    else if (res->type == PrimitiveString) printf("%s\n", res->str);
    else if (res->type == PrimitiveNumber) printf("%lld\n", res->num);
}

bool result_equal(Result * a, Result * b)
{
    if (a->type == PrimitiveANY ||
        b->type == PrimitiveANY)    return true;
    if (a->type != b->type)         return false;
    if (a->type == PrimitiveTRUE)   return true;
    if (a->type == PrimitiveFALSE)  return true;
    if (a->type == PrimitiveNULL)   return true;
    if (a->type == PrimitiveString) return streq(a->str, b->str);
    if (a->type == PrimitiveNumber) return a->num == b->num;
    return false;
}

bool result_is_true(Result * res)
{
    if (res->type == PrimitiveANY)    return true;
    if (res->type == PrimitiveTRUE)   return true;
    if (res->type == PrimitiveFALSE)  return false;
    if (res->type == PrimitiveNULL)   return false;
    if (res->type == PrimitiveString) return true;
    if (res->type == PrimitiveNumber) return res->num != 0;
    return false;
}

Function * new_function(char * name, Expression * e)
{
    /*
     * This function expects the sub-Expression queue to be formatted like so:
     *   HEAD <-> Id("def") <-> Id(name) [ <-> Id(param1) <-> Id(param2) ... ]  \
     *     <-> any(declaration) <-> TAIL
     * Minimum expected queue size = 3 ("def", name, and declaration)
     */
    Function * f = malloc(sizeof(Function));
    f->name = clone_string(name);
    f->params = new_queue(NULL);
    int i = 0, len = queue_size(e->children);
    queue_foreach(node, e->children) {
        if (i > 1 && i != len-1) {
            Expression * ec = node->data;
            expect(ec->type == Id, "Error (internal): new_function :: Bad Expression tree.\n");
            expect(ec->value != NULL, "Error (internal): new_function :: Bad Expression tree.\n");
            char * param = clone_string(ec->value);
            queue_push(f->params, param);
        }
        i++;
    }
    f->def = queue_end(e->children)->prev->data;
    return f;
}

void destroy_function(Function * f)
{
    queue_foreach(node, f->params) {
        char * param = node->data;
        free(param);
    }
    free(f->name);
    free(f);
}

Thunk * new_thunk(char * name /*required*/, Expression * e, Queue/*<Thunk>*/ * context)
{
    expect(name != NULL, "Error (internal): Thunk name should not be NULL.\n");
    Thunk * t = malloc(sizeof(Thunk));
    t->e = e;
    t->res = NULL;
    t->name = clone_string(name);
    // This is tricky: Sometimes, we want a shared context between thunks,
    // sometimes we want a completely new  and empty context,
    // and sometimes we want a clone of the parent context (to avoid contamination).
    // So, just keep a pointer, and let calling function determine the context.
    t->context = context;
    return t;
}

void destroy_thunk(Thunk * t)
{
    free(t->name);
    free(t);
}

Function * find_function(char * name)
{
    queue_foreach(node, ftable) {
        Function * f = node->data;
        if (streq(f->name, name)) {
            return f;
        }
    }
    return NULL;
}

void execute(Thunk * t)
{
    if (t->res != NULL) {
        // do nothing, this has already been calculated

    } else if (t->e->type == Program) {
        Expression * ec;
        Thunk * tc;
        // Make a clone of context share variables in local scope,
        // without contaminating parent scope.
        Queue/*<Thunk>*/ * context = new_queue(t->context);
        queue_foreach(node, t->e->children) {
            ec = node->data;
            tc = new_thunk("*", ec, context);
            execute(tc);
            t->res = tc->res;
            destroy_thunk(tc);
        }
        destroy_queue(context);

    } else if (t->e->type == Statement) {
        expect(queue_size(t->e->children) >= 1,
                "Error: Expected Statement to have more children.\n");
        char * name = ((Expression *) queue_begin(t->e->children)->data)->value;
        if (streq(name, "def")) {
            // add function to ftable
            expect(queue_size(t->e->children) >= 3,
                    "Error: Expected function name and definition.\n");
            do {
                int i = 0, len = queue_size(t->e->children);
                queue_foreach(node, t->e->children) {
                    Expression * ec = node->data;
                    if (i == 0) expect(ec->type == Id, "Error: Expected function name to be id.\n");
                    else if (i == len-1) continue;  // TODO: need to avoid duplicates!
                    else expect(ec->type == Id, "Error: Expected function parameter to be id.\n");
                    i++;
                }
            } while (0);
            char * fname = ((Expression *) queue_begin(t->e->children)->next->data)->value;
            expect(fname != NULL, "Error (internal): Function name is NULL.\n");
            // check if function already exists by name:
            expect(find_function(fname) == NULL,
                    "Error: function '%s' redeclaration not allowed!\n", fname);

            Function * f = new_function(fname, t->e);
            queue_push(ftable, f);

        } else if (streq(name, "do")) {
            do {
                int i = 0;
                Expression * ec;
                Thunk * tc;
                Queue/*<Thunk>*/ * context = new_queue(t->context);
                queue_foreach(node, t->e->children) {
                    if (i != 0) {
                        ec = node->data;
                        tc = new_thunk("*", ec, context);
                        execute(tc);
                        t->res = tc->res;
                        destroy_thunk(tc);
                    }
                    i++;
                }
                destroy_queue(context);
            } while (0);

        } else if (streq(name, "let")) {
            expect(queue_size(t->e->children) == 3,
                    "Error: Expected 'let' statement to be given 2 parameters.\n");
            Expression * id = queue_begin(t->e->children)->next->data;
            Expression * ec = queue_begin(t->e->children)->next->next->data;
            expect(id->type == Id, "Error: Expected parameter 1 of 'let' statement to be Id.\n");
            // This thunk shouldn't see itself (so t->context is cloned first):
            Queue/*<Thunk>*/ * context = new_queue(t->context);
            Thunk * tc = new_thunk(id->value, ec, context);
            queue_push(t->context, tc);
            t->res = new_result(0, NULL, PrimitiveNULL);

        } else if (streq(name, "get")) {
            expect(false, "Error: 'get' not implemented yet!\n");

        } else if (streq(name, "print")) {
            expect(queue_size(t->e->children) == 2,
                    "Invalid number of arguments for 'print' function.\n");
            Expression * ec = queue_begin(t->e->children)->next->data;
            Thunk * tt = new_thunk("*", ec, t->context);
            execute(tt);
            // perform print:
            print_result(tt->res);
            destroy_thunk(tt);
            t->res = new_result(0, NULL, PrimitiveNULL);

        } else if (streq(name, "match")) {
            // TODO: Add match to grammar, and move this sanitization to the parser...
            expect(queue_size(t->e->children) >= 2,
                    "Error: Too few parameters to 'match' statement.\n");

            Result * res_given;
            do {
                Expression * ec_given = queue_begin(t->e->children)->next->data;
                Queue/*<Thunk>*/ * context = new_queue(t->context);
                Thunk * tc_given = new_thunk("*", ec_given, context);
                execute(tc_given);
                res_given = tc_given->res;
                destroy_thunk(tc_given);
                destroy_queue(context);
            } while (0);

            Node * cur = queue_begin(t->e->children)->next->next,
                 * end = queue_end(t->e->children);
            bool matched = false;
            for (int i = 0; cur != end; i++) {
                // process the first value
                Expression * ec_test = cur->data;

                // process the ':' token
                expect(cur->next != end, "Error: Expected ':' token in match statement.\n");
                cur = cur->next;
                Expression * ec_sep = cur->data;

                // process the third value
                expect(ec_sep->type == Id, "Error: Expected ':' token in match statement.\n");
                expect(streq(ec_sep->value, ":"), "Error: Expected ':' token in match statement.\n");
                expect(cur->next != end,
                        "Error: Expected another parameter in match statement.\n");
                cur = cur->next;
                Expression * ec_ans = cur->data;

                // advice to either end, or start of next triplet:
                cur = cur->next;

                // Compute this test:
                Result * res_test;
                do {
                    Queue/*<Thunk>*/ * context = new_queue(t->context);
                    Thunk * tc_test = new_thunk("*", ec_test, context);
                    execute(tc_test);
                    res_test = tc_test->res;
                    destroy_thunk(tc_test);
                    destroy_queue(context);
                } while (0);
                if (result_equal(res_given, res_test)) {
                    Queue/*<Thunk>*/ * context = new_queue(t->context);
                    Thunk * tc_ans = new_thunk("*", ec_ans, context);
                    execute(tc_ans);
                    t->res = tc_ans->res;
                    destroy_thunk(tc_ans);
                    destroy_queue(context);
                    matched = true;
                    break;
                }
            }
            if (!matched) {
                t->res = new_result(0, NULL, PrimitiveNULL);
            }

        } else if (streq(name, "?")) {
            expect(queue_size(t->e->children) == 4,
                    "Expected 4 arguments for '?' statement.\n");
            Expression * e_test = queue_begin(t->e->children)->next->data;
            Queue/*<Thunk>*/ * context = new_queue(t->context);
            Thunk * t_test = new_thunk("*", e_test, context);
            execute(t_test);
            Expression * ec;
            if (result_is_true(t_test->res)) {
                ec = queue_begin(t->e->children)->next->next->data;
            } else {
                ec = queue_begin(t->e->children)->next->next->next->data;
            }
            Thunk * ans = new_thunk("*", ec, t->context);
            execute(ans);
            t->res = ans->res;
            destroy_thunk(ans);
            destroy_thunk(t_test);
            destroy_queue(context);

        } else if (strlen(name) == 1 && member_of(name[0], "+-*/%")) {
            expect(queue_size(t->e->children) == 3,
                    "Invalid number of arguments for '%s' function.\n", name);
            Expression * ea = queue_begin(t->e->children)->next->data;
            Expression * eb = queue_begin(t->e->children)->next->next->data;
            Queue/*<Thunk>*/ * context_a = new_queue(t->context);
            Queue/*<Thunk>*/ * context_b = new_queue(t->context);
            Thunk * tta = new_thunk("*", ea, context_a);
            Thunk * ttb = new_thunk("*", eb, context_b);
            execute(tta);
            execute(ttb);
            long long num;
            if      (name[0] == '+') num = tta->res->num + ttb->res->num;
            else if (name[0] == '-') num = tta->res->num - ttb->res->num;
            else if (name[0] == '*') num = tta->res->num * ttb->res->num;
            else if (name[0] == '/') num = tta->res->num / ttb->res->num;
            else if (name[0] == '%') num = tta->res->num % ttb->res->num;
            t->res = new_result(num, NULL, PrimitiveNumber);
            // Note: The result may have been reused from somewhere else,
            //       and it still may be used elsewhere, so we cannot destroy it yet.
            destroy_thunk(tta);
            destroy_thunk(ttb);
            destroy_queue(context_a);
            destroy_queue(context_b);

        } else if (streq(name, "=")) {
            expect(queue_size(t->e->children) == 3,
                    "Invalid number of arguments for '%s' function.\n", name);
            Expression * ea = queue_begin(t->e->children)->next->data;
            Expression * eb = queue_begin(t->e->children)->next->next->data;
            Queue/*<Thunk>*/ * context_a = new_queue(t->context);
            Queue/*<Thunk>*/ * context_b = new_queue(t->context);
            Thunk * tta = new_thunk("*", ea, context_a);
            Thunk * ttb = new_thunk("*", eb, context_b);
            execute(tta);
            execute(ttb);
            if (result_equal(tta->res, ttb->res)) {
                t->res = new_result(0, NULL, PrimitiveTRUE);
            } else {
                t->res = new_result(0, NULL, PrimitiveFALSE);
            }
            destroy_thunk(tta);
            destroy_thunk(ttb);
            destroy_queue(context_a);
            destroy_queue(context_b);

        } else {
            Function * userfunc = find_function(name);
            expect(userfunc != NULL, "Error: Couldn't find function named %s!\n", name);
            int num_params_expected = queue_size(userfunc->params);
            int num_params_supplied = queue_size(t->e->children) - 1; // ignore the function name
            expect(num_params_expected == num_params_supplied,
                    "Error: Expected %d parameters for function %s, but got %d.\n",
                    num_params_expected, name, num_params_supplied);
            // Create a new thunk, with an empty context.
            Queue/*<Thunk>*/ * context = new_queue(NULL);
            Thunk * tf = new_thunk("*", userfunc->def, context);
            // For each function parameter, create a new thunk with the context
            // of the current thunk being executed,
            // and add it to the function thunk's context.
            // That way, the only Ids visible in the function execution are the parameters.
            Node * cur = queue_begin(t->e->children)->next;
            queue_foreach(node, userfunc->params) {
                char * param_id = node->data;
                Expression * ec = cur->data;
                Thunk * tp = new_thunk(param_id, ec, t->context);
                queue_push(tf->context, tp);
                cur = cur->next;
            }
            execute(tf);
            t->res = tf->res;
            // cleanup:
            queue_foreach(node, tf->context) {
                Thunk * tp = node->data;
                destroy_thunk(tp);
            }
            destroy_queue(context);
            destroy_thunk(tf);
        }

    } else if (t->e->type == List) {
        expect(false, "Error (internal): List type not implemented yet!\n");

    } else if (t->e->type == Id) {
        char * name = t->e->value;
        bool found = false;
        Thunk * tc;
        queue_foreach(node, t->context) {
            tc = node->data;
            if (streq(tc->name, name)) {
                found = true;
                break;
            }
        }
        expect(found, "Error: Symbol %s not found.\n", name);
        execute(tc);
        t->res = tc->res;

    } else if (t->e->type == Primitive) {
        if (t->e->ptype == PrimitiveNULL) {
            t->res = new_result(0, NULL, PrimitiveNULL);

        } else if (t->e->ptype == PrimitiveANY) {
            t->res = new_result(1, NULL, PrimitiveANY);

        } else if (t->e->ptype == PrimitiveTRUE) {
            t->res = new_result(1, NULL, PrimitiveTRUE);

        } else if (t->e->ptype == PrimitiveFALSE) {
            t->res = new_result(0, NULL, PrimitiveFALSE);

        } else if (t->e->ptype == PrimitiveString) {
            t->res = new_result(1, t->e->value, PrimitiveString);

        } else if (t->e->ptype == PrimitiveNumber) {
            t->res = new_result(atoi(t->e->value), NULL, PrimitiveNumber);

        } else {
            expect(false, "Error: Couldn't match primitive expression '%s'.\n", t->e->value);
        }
    }
}


int main(int argc, char * argv[])
{

    if (argc < 2) {
        printf("Usage: %s <input.lang>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Read input from file:
    char * input = read_file(argv[1]);

    // lex/parse program into rooted tree:
    Lexer * lex = new_lexer(input);
    Expression * program = parse_program(lex);
    //print_expression(program, 0);

    // Initialize Function Table:
    ftable = new_queue(NULL);

    // Execute program:
    Queue/*<Thunk>*/ * context = new_queue(NULL);
    Thunk * thunk = new_thunk("*", program, context);
    execute(thunk);

    // clean up
    destroy_thunk(thunk);
    destroy_queue(context);
    destroy_expression(program);
    destroy_queue(ftable);
    destroy_lexer(lex);
    destroy_string(input);

    return 0;
}
