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
#define MAX_TOKEN_LEN 1000
#define SINGLE_CHAR_TOKENS "()[],+-*/="
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
    for (Node * e = queue_begin(q), * end = queue_end(q); e != end; e = e->next)

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

char input[MAX_INPUT_LEN];
char token[MAX_TOKEN_LEN];
int idx;

bool lex()
{
    int i = idx, j = 0;

    // skip whitespace
    while (member_of(input[i], " \r\n\t")) i++;

    // skip comment
    while (input[i] == '#') {
        while (input[i] != '\n' && input[i] != '\0') i++;
        if (input[i] == '\n') i++;

        // skip whitespace after comment
        while (member_of(input[i], " \r\n\t")) i++;
    }

    // if no token left
    if (input[i] == '\0') return false;

    // if single character token
    if (member_of(input[i], SINGLE_CHAR_TOKENS)) {
        token[j++] = input[i++];

    } else if (input[i] == '\"') { // string token
        token[j++] = input[i++];
        while (input[i] != '\0' && input[i] != '\"') {
            token[j++] = input[i++];
        }
        if (input[i] == '\"') {
            token[j++] = input[i++];
        } else {
            printf("Error: Unterminated string\n");
        }

    } else { // multi character token
        while (input[i] != '\0'
                && !member_of(input[i], SINGLE_CHAR_TOKENS)
                && !isspace(input[i])
                && input[i] != '#') {
            token[j++] = input[i++];
        }
    }
    token[j] = '\0';

    // check length not zero
    expect(strlen(token) >= 1, "Token should not be empty string!\n");
    // update global program index
    idx = i;
    return true;
}


/*******************
 *     PARSING     *
 *******************/

Expression * parse_program(void);
bool parse_statement (Expression * root);
bool parse_list      (Expression * root);
bool parse_id        (Expression * root);
bool parse_primitive (Expression * root);

bool parse_primitive(Expression * root)
{
    int prv_idx = idx;
    if (!lex()) {
        printf("Error: Expected token\n");
        exit(EXIT_FAILURE);
    }
    int len = strlen(token);
    bool is_num = strcmp(token, "0") == 0 ? true : atoi(token) != 0;
    bool is_str = token[0] == '\"' && token[len-1] == '\"';
    bool is_any = (strcmp(token, "ANY") == 0);
    bool is_true = (strcmp(token, "TRUE") == 0);
    bool is_false = (strcmp(token, "FALSE") == 0);
    bool is_null = (strcmp(token, "NULL") == 0);
    if (!(is_num || is_str || is_any || is_true || is_false || is_null)) {
        idx = prv_idx;
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
    return true;
}

bool parse_id(Expression * root)
{
    int prv_idx = idx;
    if (!lex()) {
        printf("Error: Expected token\n");
        exit(EXIT_FAILURE);
    }
    Expression * e = new_expression(token, Id, PrimitiveANY);
    bool is_id = true;
    for (int i = 0; token[i] != '\0'; i++) {
        if (!(isalpha(token[i]) || member_of(token[i], "_+-*/"))) {
            is_id = false;
            break;
        }
    }
    if (!is_id) {
        idx = prv_idx; // go back to previous token
        destroy_expression(e);
        return false;
    }
    queue_push(root->children, e);
    return true;
}

bool parse_list(Expression * root)
{
    int prv_idx = idx;
    if (!lex()) {
        printf("Error: Expected token\n");
        exit(EXIT_FAILURE);
    }
    Expression * e = new_expression(NULL, List, PrimitiveANY);
    if (strcmp(token, "[") != 0) {
        destroy_expression(e);
        idx = prv_idx;
        return false;
    }
    while (parse_id(e) || parse_statement(e) || parse_list(e) || parse_primitive(e));
    lex();
    expect(strcmp(token, "]") == 0, "Error: Expected closing paren!\n");
    e->value = NULL;
    e->type = List;
    queue_push(root->children, e);
    return true;
}

bool parse_statement(Expression * root)
{
    int prv_idx = idx;
    if (!lex()) return false;
    Expression * e = new_expression(NULL, Statement, PrimitiveANY);
    if (strcmp(token, "(") != 0 || !parse_id(e)) {
        destroy_expression(e);
        idx = prv_idx; // go back to previous token
        return false;
    }
    while (parse_id(e) || parse_statement(e) || parse_list(e) || parse_primitive(e));
    lex();
    expect(strcmp(token, ")") == 0, "Error: Expected closing paren!\n");
    queue_push(root->children, e);
    return true;
}

Expression * parse_program()
{
    Expression * e = new_expression(NULL, Program, PrimitiveANY);
    while (parse_statement(e));
    return e;
}


/*******************
 *    EXECUTION    *
 *******************/

/* lifecycle:
 *   - Creation: on primitive execution
 *   - Destruction: upon terminal usage (not passing)
 *   *** Result can only be used once ***
 */
struct Result {
    long long num;
    PrimitiveType type;
    char * str;
} typedef Result;

/* lifecycle:
 *   - Creation: When statement "def" is executed
 *   - Destruction: Program termination
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
struct Context {
    Queue/*<Thunk>*/ * thunks;
} typedef Context;

/* lifecycle:
 *   - Creation: Before execution call
 *   - Destruction: After execution returns
 */
struct Thunk {
    char * name;  // variable name
    Expression * e;
    Result * res;
    Context * cx;  // multiple thunks may map to the same context
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

Function * new_function(char * name, Expression * e)
{
    /*
     * This function expects the sub-Expression queue to be formatted like so:
     *   HEAD <-> Id("def") <-> Id(name) [ <-> Id(param1) <-> Id(param2) ... ]  \
     *     <-> any(declaration) <-> TAIL
     * Minimum expected queue size = 3 ("def", name, and declaration)
     */
    Function * f = malloc(sizeof(Function));
    f->name = malloc((strlen(name) + 1) * sizeof(char));
    strcpy(f->name, name);
    int i = 0, len = queue_size(e->children);
    queue_foreach(node, e->children) {
        if (i == 0 || i == len-1) continue;
        Expression * ec = node->data;
        expect(ec->type == Id, "Error (internal): new_function :: Bad Expression tree.");
        expect(ec->value != NULL, "Error (internal): new_function :: Bad Expression tree.");
        char * param = malloc((strlen(ec->value) + 1) * sizeof(char));
        strcpy(param, ec->value);
        queue_push(f->params, param);
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

Thunk * new_thunk(char * name /*required*/, Expression * e, Context * cx)
{
    Thunk * t = malloc(sizeof(Thunk));
    t->e = e;
    t->res = NULL;
    t->name = clone_string(name);
    t->cx = cx;
    return t;
}

void destroy_thunk(Thunk * t)
{
    free(t->name);
    free(t);
}

Context * new_context(Context * cx_old)
{
    // if cx_old is given, clone it
    Context * cx_new = malloc(sizeof(Context));
    cx_new->thunks = new_queue(cx_old == NULL ? NULL : cx_old->thunks);
    return cx_new;
}

void destroy_context(Context * sc)
{
    destroy_queue(sc->thunks);
    free(sc);
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

Thunk * find_thunk(char * name, Context * cx)
{
    queue_foreach(node, cx->thunks) {
        Thunk * t = node->data;
        if (streq(t->name, name)) {
            return t;
        }
    }
    return NULL;
}

void add_thunk_to_context(Context * cx, Thunk * t)
{
    // attempt to remove a thunk with the same name
    queue_foreach(node, cx->thunks) {
        Thunk * tt = node->data;
        if (streq(t->name, tt->name)) {
            queue_erase(node);
            // We do not destroy the associated thunk because thunks are stored
            // on the recursion stack of execute.
        }
    }
    queue_push(cx->thunks, t);
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

void execute(Thunk * t)
{
    if (t->e->type == Program) {
        queue_foreach(node, t->e->children) {
            Expression * ec = node->data;
            Thunk * tt = new_thunk("*", ec, t->cx /* same context */);
            execute(tt);
            t->res = tt->res;
            destroy_thunk(tt);
        }

    } else if (t->e->type == Statement) {
        expect(queue_size(t->e->children) >= 1,
                "Error: Expected Statement to have more children.\n");
        char * name = ((Expression *) queue_begin(t->e->children)->data)->value;
        if (streq(name, "def")) {
            // add function to ftable
            expect(queue_size(t->e->children) >= 3,
                    "Error: Expected function name and definition.");
            int i = 0, len = queue_size(t->e->children);
            queue_foreach(node, t->e->children) {
                Expression * ec = node->data;
                if (i == 0) expect(ec->type == Id, "Error: Expected function name to be id.");
                else if (i == len-1) continue;
                else expect(ec->type == Id, "Error: Expected function parameter to be id.");
                i++;
            }
            char * fname = ((Expression *) queue_begin(t->e->children)->next->data)->value;
            expect(fname != NULL, "Error: Function name is NULL.\n");
            // check if function already exists by name:
            expect(find_function(fname) == NULL,
                    "Error: function '%s' redeclaration not allowed!\n", fname);

            Function * f = new_function(fname, t->e);
            queue_push(ftable, f);

        } else if (streq(name, "match")) {
            expect(false, "Error: 'match' not implemented yet!");

        } else if (streq(name, "do")) {
            expect(false, "Error: 'do' not implemented yet!");

        } else if (streq(name, "let")) {
            expect(false, "Error: 'let' not implemented yet!");

        } else if (streq(name, "get")) {
            expect(false, "Error: 'get' not implemented yet!");

        } else if (streq(name, "print")) {
            expect(queue_size(t->e->children) == 2,
                    "Invalid number of arguments for 'print' function.\n");
            Expression * ec = queue_begin(t->e->children)->next->data;
            Thunk * tt = new_thunk("*", ec);
            execute(tt, cx);
            // perform print:
            print_result(tt->res);
            destroy_thunk(tt);
            t->res = new_result(0, NULL, PrimitiveNULL);

        } else if (strlen(name) == 1 && member_of(name[0], "+-*/")) {
            expect(queue_size(t->e->children) == 3,
                    "Invalid number of arguments for '%s' function.\n", name);
            Expression * ea = queue_begin(t->e->children)->next->data;
            Expression * eb = queue_begin(t->e->children)->next->next->data;
            Thunk * tta = new_thunk("*", ea);
            Thunk * ttb = new_thunk("*", eb);
            execute(tta, cx);
            execute(ttb, cx);
            long long num;
            if (name[0] == '+') num = tta->res->num + ttb->res->num;
            if (name[0] == '-') num = tta->res->num - ttb->res->num;
            if (name[0] == '*') num = tta->res->num * ttb->res->num;
            if (name[0] == '/') num = tta->res->num / ttb->res->num;
            t->res = new_result(num, NULL, PrimitiveNumber);
            destroy_result(tta->res);
            destroy_result(ttb->res);
            destroy_thunk(tta);
            destroy_thunk(ttb);

        } else {
            Function * userfunc = find_function(name);
            expect(userfunc != NULL, "Error: Couldn't find function named %s!\n", name);
            expect(queue_size(t->e->children) - 2 == queue_size(userfunc->params),
                    "Error: Expected %d variables to function %s, but got %d!",
                    queue_size(t->e->children), userfunc->name, queue_size(userfunc->params+2));
            Context * cx_new = new_context(cx);
            // Create thunks for all function parameters:
            Node * cur = queue_begin(t->e->children)->next;
            queue_foreach(node, userfunc->params) {
                char * param_id = node->data;
                Expression * ec = cur->data;
                Thunk * tt = new_thunk(param_id, ec);
                add_thunk_to_context(cx, tt);
                cur = cur->next;
            }
            Thunk * tt = new_thunk("*", userfunc->def);
            execute(tt, cx_new);
            t->res = tt->res;
            destroy_thunk(tt);
            destroy_context(cx_new);
        }

    } else if (t->e->type == List) {

    } else if (t->e->type == Id) {

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
    char * line;
    FILE * fp;
    size_t len = 0;

    if (argc < 2) {
        printf("Usage: %s <input.lang>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Read input from file:
    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    while (getline(&line, &len, fp) != -1) {
        strcat(input, line);
    }
    fclose(fp);
    if (line) free(line);

    // parse program into rooted tree:
    Expression * program = parse_program();
    print_expression(program, 0);

    // Initialize Function Table:
    ftable = new_queue(NULL);

    // Execute program:
    Context * context = new_context(NULL);
    Thunk * thunk = new_thunk("*", program, context);
    execute(thunk);

    // clean up
    destroy_expression(program);
    destroy_queue(ftable);
    destroy_context(context);

    return 0;
}
