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
#include <limits.h>

#define DEBUG

#define MAX_INPUT_LEN 100000
#define SINGLE_CHAR_TOKENS "()[],+-*/=?:"
#define SYSTEM_FUNCTION_TOKENS "+-*/=?%:"
#define DEFAULT_HASHTABLE_SIZE 100
#define HASH_TYPE long long
#define streq(a, b) (strcmp((a), (b)) == 0)

// Some heavily used, pre-computed string hashes:
#define HASH_OF_DEF       335110
#define HASH_OF_LET       343316
#define HASH_OF_DO        10479
#define HASH_OF_MATCH     352476360
#define HASH_OF_TIMES     266
#define HASH_OF_PLUS      267
#define HASH_OF_MINUS     269
#define HASH_OF_DIVIDE    271
#define HASH_OF_PERCENT   261
#define HASH_OF_COMMA     268
#define HASH_OF_EQUAL     285
#define HASH_OF_COLON     282
#define HASH_OF_QUESTION  287
#define HASH_OF_READ_INT  11725402354228
#define HASH_OF_READ_CHAR 375212875132050
#define HASH_OF_PRINT     356168244
#define HASH_OF_GET       338196
#define HASH_OF_NULL      9985484
#define HASH_OF_ANY       298521
#define HASH_OF_TRUE      10179301
#define HASH_OF_FALSE     310491813


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

#define expect(condition, ...)             \
    do {                                   \
        if (!(condition)) {                \
            fprintf(stderr, __VA_ARGS__);  \
            exit(EXIT_FAILURE);            \
        }                                  \
    } while (0);

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

HASH_TYPE hash_string(char * s)
{
    HASH_TYPE h = 7;
    for (int i = 0; s[i] != '\0'; i++) {
        h += h * 31 + s[i];
    }
    return h;
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
    size_t size;
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
    q->size = 0;
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
    q->size++;
}

void queue_remove(Queue * q, Node * node)
{
    expect(node->prev != NULL, "Error (internal): queue_remove.\n");
    expect(node->next != NULL, "Error (internal): queue_remove.\n");
    Node * n = node->next;
    Node * p = node->prev;
    p->next = n;
    n->prev = p;
    destroy_node(node);
    q->size--;
}

size_t queue_size(Queue * q)
{
    return q->size;
}


/*******************
 *   HASH TABLE    *
 *******************/

struct HashTableItem {
    HASH_TYPE key;
    void * value;
    bool set;
} typedef HashTableItem;

struct HashTable {
    HashTableItem * table;
    size_t size;
    int capacity;
} typedef HashTable;

HashTable * new_hashtable(int capacity)
{
    HashTable * ht = malloc(sizeof(HashTable));
    ht->table = malloc(capacity * sizeof(HashTableItem));
    ht->size = 0;
    ht->capacity = capacity;
    for (int i = 0; i < capacity; i++) {
        ht->table[i].set = false;
    }
    return ht;
}

void destroy_hashtable(HashTable * ht)
{
    free(ht->table);
    free(ht);
}

#define hashtable_foreach(node, ht)                     \
    for (HashTableItem * node = (ht)->table;            \
         node != (ht)->table + (ht)->capacity;          \
         node++) if (node->set)

void hashtable_insert(HashTable * ht, HASH_TYPE key, void * value)
{
    // grow the hash table if needed:
    if (ht->size == ht->capacity) {
        HashTableItem * old_table = ht->table;
        int old_capacity = ht->capacity;
        ht->capacity *= 2;
        ht->size = 0;
        ht->table = malloc(ht->capacity * sizeof(HashTableItem));
        for (int i = 0; i < old_capacity; i++) {
            hashtable_insert(ht, old_table[i].key, old_table[i].value);
        }
        free(old_table);
    }

    int cap = ht->capacity;
    for (int i = key % cap, j = 0; j < cap; i = (i + 1) % cap, j++) {
        if (!ht->table[i].set) {
            ht->table[i].set = true;
            ht->table[i].key = key;
            ht->table[i].value = value;
            break;
        }
    }
    ht->size++;
}

void hashtable_remove(HashTable * ht, HASH_TYPE key)
{
    expect(ht->size != 0, "Error: Removing item from empty HashTable.\n");
    int cap = ht->capacity;
    int found = false;
    for (int i = key % cap, j = 0; j < cap; i = (i + 1) % cap, j++) {
        if (ht->table[i].set && ht->table[i].key == key) {
            ht->table[i].set = false;
            found = true;
            break;
        }
    }

    expect(found, "Error: Couldn't find element in HashTable with key %lld.\n", key);
    ht->size--;
}

HashTableItem * hashtable_find(HashTable * ht, HASH_TYPE key)
{
    int cap = ht->capacity;
    for (int i = key % cap, j = 0; j < cap; i = (i + 1) % cap, j++) {
        if (ht->table[i].set && ht->table[i].key == key) {
            return ht->table + i;
        }
    }
    return NULL;
}

size_t hashtable_size(HashTable * ht)
{
    return ht->size;
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
    PrimitiveChar = 6,
} typedef PrimitiveType;
char * PrimitiveTypeString[7] = {
    "PrimitiveANY",
    "PrimitiveTRUE",
    "PrimitiveFALSE",
    "PrimitiveNULL",
    "PrimitiveNumber",
    "PrimitiveString",
    "PrimitiveChar",
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
    HASH_TYPE value;
    Queue/*<Expression>*/ * children;
    ExpressionType type;
    PrimitiveType ptype;
    char * str;
} typedef Expression;

Expression * new_expression(HASH_TYPE value, ExpressionType type, PrimitiveType ptype, char * str);
void destroy_expression(Expression * e);
void print_expression(Expression * e, HashTable * symbols, int d);

Expression * new_expression(HASH_TYPE value, ExpressionType type, PrimitiveType ptype, char * str)
{
    Expression * e = malloc(sizeof(Expression));
    e->children = new_queue(NULL);
    e->value = value;
    e->type = type;
    e->ptype = ptype;
    e->str = str == NULL ? NULL : clone_string(str);
    return e;
}

void destroy_expression(Expression * e)
{
    queue_foreach(node, e->children) {
        destroy_expression(node->data);
    }
    if (e->str) free(e->str);
    destroy_queue(e->children);
    free(e);
}

void print_expression(Expression * e, HashTable * symbols, int d)
{
    for (int i = 0; i < d; i++) printf("  ");
    printf("%s : ", ExpressionTypeString[e->type]);
    if (e->type == Primitive) {
        if (e->ptype == PrimitiveString) {
            printf("(\"%s\")\n", e->str);
        } else if (e->ptype == PrimitiveNumber) {
            printf("(%lld)\n", e->value);
        } else {
            HashTableItem * item = hashtable_find(symbols, e->value);
            expect(item != NULL, "Error (internal): Primitive Expression should have token.\n");
            printf("(%s)\n", (char *)item->value);
        }
    } else {
        HashTableItem * item = hashtable_find(symbols, e->value);
        expect(item != NULL, "Error (internal): Non-Primitive expression should have token.\n");
        printf("%s : %s\n", ExpressionTypeString[e->type], (char *)item->value);
    }
    queue_foreach(node, e->children) {
        print_expression(node->data, symbols, d+1);
    }
}


/*******************
 *     LEXING      *
 *******************/

HashTable * new_symbol_table()
{
    HashTable * symbols = new_hashtable(DEFAULT_HASHTABLE_SIZE);
    hashtable_insert(symbols, HASH_OF_DEF,       clone_string("def"     ));
    hashtable_insert(symbols, HASH_OF_LET,       clone_string("let"     ));
    hashtable_insert(symbols, HASH_OF_DO,        clone_string("do"      ));
    hashtable_insert(symbols, HASH_OF_MATCH,     clone_string("match"   ));
    hashtable_insert(symbols, HASH_OF_TIMES,     clone_string("*"       ));
    hashtable_insert(symbols, HASH_OF_PLUS,      clone_string("+"       ));
    hashtable_insert(symbols, HASH_OF_MINUS,     clone_string("-"       ));
    hashtable_insert(symbols, HASH_OF_DIVIDE,    clone_string("/"       ));
    hashtable_insert(symbols, HASH_OF_PERCENT,   clone_string("%"       ));
    hashtable_insert(symbols, HASH_OF_COMMA,     clone_string(","       ));
    hashtable_insert(symbols, HASH_OF_EQUAL,     clone_string("="       ));
    hashtable_insert(symbols, HASH_OF_COLON,     clone_string(":"       ));
    hashtable_insert(symbols, HASH_OF_QUESTION,  clone_string("?"       ));
    hashtable_insert(symbols, HASH_OF_READ_INT,  clone_string("read_int"));
    hashtable_insert(symbols, HASH_OF_READ_CHAR, clone_string("read_char"));
    hashtable_insert(symbols, HASH_OF_PRINT,     clone_string("print"   ));
    hashtable_insert(symbols, HASH_OF_GET,       clone_string("get"     ));
    return symbols;
}

struct Lexer {
    char * input;
    int idx;
    int prev_idx;
    HashTable * symbols;
} typedef Lexer;

Lexer * new_lexer(char * input)
{
    Lexer * lex = malloc(sizeof(Lexer));
    lex->input = input;
    lex->idx = 0;
    lex->prev_idx = -1;
    lex->symbols = new_symbol_table();
    return lex;
}

void destroy_lexer(Lexer * lex)
{
    hashtable_foreach(node, lex->symbols) {
        char * token = node->value;
        expect(token != NULL, "Error (internal): destroy_lexer(): token is null.\n");
        free(token);
    }
    destroy_hashtable(lex->symbols);
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
        expect(lex->input[i] == '\"', "Error: Unterminated string.\n");
        i++;

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

    // Add this token to the symbol table:
    token = new_string(r-l+1);
    substring(token, lex->input, l, r);
    HASH_TYPE key = hash_string(token);
    HashTableItem * item = hashtable_find(lex->symbols, key);
    if (item == NULL) {
        hashtable_insert(lex->symbols, key, token);
    } else {
        // destroy the string we made so we can output the already allocated token
        destroy_string(token);
        token = item->value;
    }

    lex->prev_idx = lex->idx;
    lex->idx = i;
    return token;
}

void lexer_back(Lexer * lex)
{
    expect(lex->prev_idx != -1, "Error (internal): Lexer: Invalid call to lexer_back().\n");
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
        return false;
    }
    PrimitiveType ptype;
    if      (is_num)   ptype = PrimitiveNumber;
    else if (is_str)   ptype = PrimitiveString;
    else if (is_any)   ptype = PrimitiveANY;
    else if (is_true)  ptype = PrimitiveTRUE;
    else if (is_false) ptype = PrimitiveFALSE;
    else if (is_null)  ptype = PrimitiveNULL;
    else               ptype = PrimitiveANY;
    HASH_TYPE key;
    char * str;
    if (is_str) {
        key = 0;
        int sz = strlen(token);
        str = new_string(sz);
        substring(str, token, 1, sz-1);
    } else if (is_num) {
        key = atoi(token);
        str = NULL;
    } else {
        key = hash_string(token);
        str = NULL;
    }
    Expression * e = new_expression(key, Primitive, ptype, str);
    queue_push(root->children, e);
    if (str) destroy_string(str);
    return true;
}

bool parse_id(Expression * root, Lexer * lex)
{
    char * token = lexer_seek(lex);
    expect(token != NULL, "Error: Expected token.\n");
    HASH_TYPE key = hash_string(token);
    Expression * e = new_expression(key, Id, PrimitiveANY, NULL);
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
        return false;
    }
    queue_push(root->children, e);
    return true;
}

bool parse_list(Expression * root, Lexer * lex)
{
    char * token = lexer_seek(lex);
    expect(token != NULL, "Error: Expected token.\n");
    HASH_TYPE key = hash_string(token);
    Expression * e = new_expression(key, List, PrimitiveANY, NULL);
    if (strcmp(token, "[") != 0) {
        lexer_back(lex);
        destroy_expression(e);
        return false;
    }
    while (parse_id(e, lex) ||
           parse_statement(e, lex) ||
           parse_list(e, lex) ||
           parse_primitive(e, lex));
    token = lexer_seek(lex);
    expect(strcmp(token, "]") == 0, "Error: Expected closing paren!\n");
    queue_push(root->children, e);
    return true;
}

bool parse_statement(Expression * root, Lexer * lex)
{
    char * token = lexer_seek(lex);
    if (token == NULL) {
        return false;
    }
    HASH_TYPE key = hash_string(token);
    Expression * e = new_expression(key, Statement, PrimitiveANY, NULL);
    if (strcmp(token, "(") != 0 || !parse_id(e, lex)) {
        lexer_back(lex);
        destroy_expression(e);
        return false;
    }
    while (parse_primitive(e, lex) ||
           parse_id(e, lex) ||
           parse_statement(e, lex) ||
           parse_list(e, lex));
    token = lexer_seek(lex);
    expect(strcmp(token, ")") == 0, "Error: Expected closing paren!\n");
    queue_push(root->children, e);
    return true;
}

Expression * parse_program(Lexer * lex)
{
    Expression * e = new_expression(HASH_OF_TIMES, Program, PrimitiveANY, NULL);
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
    HASH_TYPE name;
    Queue/*<HASH_TYPE>*/ * params;
    Expression * def;
} typedef Function;
Queue/*<Function>*/ * ftable;

/* lifecycle:
 *   - Creation: Before execution call
 *   - Destruction: After execution returns
 */
struct Thunk {
    HASH_TYPE name;  // variable name
    Expression * e;
    Result * res;
    Queue/*<Thunk>*/ * context;
} typedef Thunk;

Result * new_result(HASH_TYPE num, char * str, PrimitiveType type)
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
    else if (res->type == PrimitiveChar)   printf("%c\n", (char)res->num);
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
    if (a->type == PrimitiveChar)   return a->num == b->num;
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
    if (res->type == PrimitiveChar)   return res->num != 0;
    return false;
}

HASH_TYPE hash_result(Result * res)
{
    // reserve first 3 bits for basic types:
    if (res->type == PrimitiveNULL)   return 0;
    if (res->type == PrimitiveANY)    return 1;
    if (res->type == PrimitiveTRUE)   return 2;
    if (res->type == PrimitiveFALSE)  return 3;
    if (res->type == PrimitiveString) return 8 * hash_string(res->str);
    if (res->type == PrimitiveNumber) return 8 * res->num;
    if (res->type == PrimitiveChar)   return 8 * res->num;
    return 0;
}

Function * new_function(HASH_TYPE name, Expression * e)
{
    /*
     * This function expects the sub-Expression queue to be formatted like so:
     *   HEAD <-> Id("def") <-> Id(name) [ <-> Id(param1) <-> Id(param2) ... ]  \
     *     <-> any(declaration) <-> TAIL
     * Minimum expected queue size = 3 ("def", name, and declaration)
     */
    Function * f = malloc(sizeof(Function));
    f->name = name;
    f->params = new_queue(NULL);
    int i = 0, len = queue_size(e->children);
    queue_foreach(node, e->children) {
        if (i > 1 && i != len-1) {
            Expression * ec = node->data;
            expect(ec->type == Id,
                    "Error (internal): new_function :: Bad Expression tree.\n");
            HASH_TYPE param = ec->value;
            queue_push(f->params, (void *) param);
        }
        i++;
    }
    f->def = queue_end(e->children)->prev->data;
    return f;
}

void destroy_function(Function * f)
{
    destroy_queue(f->params);
    free(f);
}

Thunk * new_thunk(HASH_TYPE name /*required*/, Expression * e, Queue/*<Thunk>*/ * context)
{
    Thunk * t = malloc(sizeof(Thunk));
    t->e = e;
    t->res = NULL;
    t->name = name;
    // This is tricky: Sometimes, we want a shared context between thunks,
    // sometimes we want a completely new  and empty context,
    // and sometimes we want a clone of the parent context (to avoid contamination).
    // So, just keep a pointer, and let calling function determine the context.
    t->context = context;
    return t;
}

void destroy_thunk(Thunk * t)
{
    free(t);
}

Function * find_function(HASH_TYPE name)
{
    queue_foreach(node, ftable) {
        Function * f = node->data;
        if (f->name == name) {
            return f;
        }
    }
    return NULL;
}

void execute(Thunk * t, HashTable * symbols)
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
            tc = new_thunk(HASH_OF_TIMES, ec, context);
            execute(tc, symbols);
            t->res = tc->res;
            destroy_thunk(tc);
        }
        destroy_queue(context);

    } else if (t->e->type == Statement) {
        expect(queue_size(t->e->children) >= 1,
                "Error: Expected Statement to have more children.\n");
        HASH_TYPE name = ((Expression *) queue_begin(t->e->children)->data)->value;
        if (name == HASH_OF_DEF) {
            // add function to ftable
            expect(queue_size(t->e->children) >= 3,
                    "Error: Expected function name and definition.\n");
            do {
                int i = 0, len = queue_size(t->e->children);
                queue_foreach(node, t->e->children) {
                    Expression * ec = node->data;
                    if (i == 0) expect(ec->type == Id, "Error: Expected function name to be id.\n");
                    if (i == len-1) continue;  // TODO: need to avoid duplicates!
                    expect(ec->type == Id, "Error: Expected function parameter to be id.\n");
                    i++;
                }
            } while (0);
            HASH_TYPE fname = ((Expression *) queue_begin(t->e->children)->next->data)->value;
            // check if function already exists by name:
            expect(find_function(fname) == NULL,
                    "Error: function '%s' redeclaration not allowed!\n",
                    (char *)hashtable_find(symbols, fname)->value);

            Function * f = new_function(fname, t->e);
            queue_push(ftable, f);

        } else if (name == HASH_OF_DO) {
            int i = 0;
            Expression * ec;
            Thunk * tc;
            Queue/*<Thunk>*/ * context = new_queue(t->context);
            queue_foreach(node, t->e->children) {
                if (i != 0) {
                    ec = node->data;
                    tc = new_thunk(HASH_OF_TIMES, ec, context);
                    execute(tc, symbols);
                    t->res = tc->res;
                    destroy_thunk(tc);
                }
                i++;
            }
            destroy_queue(context);

        } else if (name == HASH_OF_LET) {
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

        } else if (name == HASH_OF_GET) {
            expect(false, "Error: 'get' not implemented yet!\n");

        } else if (name == HASH_OF_READ_INT) {
            expect(queue_size(t->e->children) == 1,
                    "Error: Function 'read_int' expects no parameters.\n");
            long long num;
            expect(scanf(" %lld", &num) != EOF, "Error: read_int reached end of file.\n");
            t->res = new_result(num, NULL, PrimitiveNumber);

        } else if (name == HASH_OF_READ_CHAR) {
            expect(queue_size(t->e->children) == 1,
                    "Error: Function 'read_char' expects no parameters.\n");
            char c;
            expect(scanf(" %c", &c) != EOF, "Error: read_char reached end of file.\n");
            t->res = new_result(c, NULL, PrimitiveChar);

        } else if (name == HASH_OF_PRINT) {
            expect(queue_size(t->e->children) == 2,
                    "Invalid number of arguments for 'print' function.\n");
            Expression * ec = queue_begin(t->e->children)->next->data;
            Thunk * tt = new_thunk(HASH_OF_TIMES, ec, t->context);
            execute(tt, symbols);
            // perform print:
            print_result(tt->res);
            destroy_thunk(tt);
            t->res = new_result(0, NULL, PrimitiveNULL);

        } else if (name == HASH_OF_MATCH) {
            // TODO: Add match to grammar, and move this sanitization to the parser...
            expect(queue_size(t->e->children) >= 2,
                    "Error: Too few parameters to 'match' statement.\n");

            Result * res_given;
            do {
                Expression * ec_given = queue_begin(t->e->children)->next->data;
                Thunk * tc_given = new_thunk(HASH_OF_TIMES, ec_given, t->context);
                execute(tc_given, symbols);
                res_given = tc_given->res;
                destroy_thunk(tc_given);
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
                expect(ec_sep->value == HASH_OF_COLON,
                        "Error: Expected ':' token in match statement.\n");
                expect(cur->next != end,
                        "Error: Expected another parameter in match statement.\n");
                cur = cur->next;
                Expression * ec_ans = cur->data;

                // advice to either end, or start of next triplet:
                cur = cur->next;

                // Compute this test:
                Result * res_test;
                do {
                    Thunk * tc_test = new_thunk(HASH_OF_TIMES, ec_test, t->context);
                    execute(tc_test, symbols);
                    res_test = tc_test->res;
                    destroy_thunk(tc_test);
                } while (0);
                if (result_equal(res_given, res_test)) {
                    Thunk * tc_ans = new_thunk(HASH_OF_TIMES, ec_ans, t->context);
                    execute(tc_ans, symbols);
                    t->res = tc_ans->res;
                    destroy_thunk(tc_ans);
                    matched = true;
                    break;
                }
            }
            if (!matched) {
                t->res = new_result(0, NULL, PrimitiveNULL);
            }

        } else if (name == HASH_OF_QUESTION) {
            expect(queue_size(t->e->children) == 4,
                    "Expected 4 arguments for '?' statement.\n");
            Expression * e_test = queue_begin(t->e->children)->next->data;
            Thunk * t_test = new_thunk(HASH_OF_TIMES, e_test, t->context);
            execute(t_test, symbols);
            Expression * ec;
            if (result_is_true(t_test->res)) {
                ec = queue_begin(t->e->children)->next->next->data;
            } else {
                ec = queue_begin(t->e->children)->next->next->next->data;
            }
            Thunk * ans = new_thunk(HASH_OF_TIMES, ec, t->context);
            execute(ans, symbols);
            t->res = ans->res;
            destroy_thunk(ans);
            destroy_thunk(t_test);

        } else if (name == HASH_OF_PLUS   ||
                   name == HASH_OF_MINUS  ||
                   name == HASH_OF_TIMES  ||
                   name == HASH_OF_DIVIDE ||
                   name == HASH_OF_PERCENT) {
            expect(queue_size(t->e->children) == 3,
                    "Invalid number of arguments for '%s' function.\n",
                    (char *)hashtable_find(symbols, name)->value);
            Expression * ea = queue_begin(t->e->children)->next->data;
            Expression * eb = queue_begin(t->e->children)->next->next->data;
            Thunk * tta = new_thunk(HASH_OF_TIMES, ea, t->context);
            Thunk * ttb = new_thunk(HASH_OF_TIMES, eb, t->context);
            execute(tta, symbols);
            execute(ttb, symbols);
            long long num = 0;
            if      (name == HASH_OF_PLUS)    num = tta->res->num + ttb->res->num;
            else if (name == HASH_OF_MINUS)   num = tta->res->num - ttb->res->num;
            else if (name == HASH_OF_TIMES)   num = tta->res->num * ttb->res->num;
            else if (name == HASH_OF_DIVIDE)  num = tta->res->num / ttb->res->num;
            else if (name == HASH_OF_PERCENT) num = tta->res->num % ttb->res->num;
            else                              num = 0;
            t->res = new_result(num, NULL, PrimitiveNumber);
            // Note: The result may have been reused from somewhere else,
            //       and it still may be used elsewhere, so we cannot destroy it yet.
            destroy_thunk(tta);
            destroy_thunk(ttb);

        } else if (name == HASH_OF_EQUAL) {
            expect(queue_size(t->e->children) == 3,
                    "Invalid number of arguments for '=' function.\n");
            Expression * ea = queue_begin(t->e->children)->next->data;
            Expression * eb = queue_begin(t->e->children)->next->next->data;
            Thunk * tta = new_thunk(HASH_OF_TIMES, ea, t->context);
            Thunk * ttb = new_thunk(HASH_OF_TIMES, eb, t->context);
            execute(tta, symbols);
            execute(ttb, symbols);
            if (result_equal(tta->res, ttb->res)) {
                t->res = new_result(0, NULL, PrimitiveTRUE);
            } else {
                t->res = new_result(0, NULL, PrimitiveFALSE);
            }
            destroy_thunk(tta);
            destroy_thunk(ttb);

        } else {
            Function * userfunc = find_function(name);
            expect(userfunc != NULL,
                    "Error: Couldn't find function named %s!\n",
                    (char *)hashtable_find(symbols, name)->value);
            int num_params_expected = queue_size(userfunc->params);
            int num_params_supplied = queue_size(t->e->children) - 1; // ignore the function name
            expect(num_params_expected == num_params_supplied,
                    "Error: Expected %d parameters for function %s, but got %d.\n",
                    num_params_expected,
                    (char *)hashtable_find(symbols, name)->value,
                    num_params_supplied);
            // Create a new thunk, with an empty context.
            Queue/*<Thunk>*/ * context = new_queue(NULL);
            Thunk * tf = new_thunk(HASH_OF_TIMES, userfunc->def, context);
            // For each function parameter, create a new thunk with the context
            // of the current thunk being executed,
            // and add it to the function thunk's context.
            // That way, the only Ids visible in the function execution are the parameters.
            Node * cur = queue_begin(t->e->children)->next;
            queue_foreach(node, userfunc->params) {
                HASH_TYPE param_id = (HASH_TYPE)node->data;
                Expression * ec = cur->data;
                Thunk * tp = new_thunk(param_id, ec, t->context);
                queue_push(tf->context, tp);
                cur = cur->next;
            }
            execute(tf, symbols);
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
        HASH_TYPE name = t->e->value;
        bool found = false;
        Thunk * tc;
        queue_foreach(node, t->context) {
            tc = node->data;
            if (tc->name == name) {
                found = true;
                break;
            }
        }
        expect(found,
                "Error: Symbol %s not found.\n",
                (char *)hashtable_find(symbols, name)->value);
        execute(tc, symbols);
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
            t->res = new_result(1, t->e->str, PrimitiveString);

        } else if (t->e->ptype == PrimitiveNumber) {
            t->res = new_result(t->e->value, NULL, PrimitiveNumber);

        } else if (t->e->ptype == PrimitiveChar) {
            t->res = new_result(t->e->value, NULL, PrimitiveChar);

        } else {
            expect(false,
                    "Error: Couldn't match primitive expression '%s'.\n",
                    (char *)hashtable_find(symbols, t->e->value)->value);
        }
    }
}


int main(int argc, char * argv[])
{
    expect(argc >= 2, "Usage: %s <input.lang>\n", argv[0]);

    // Read input from file:
    char * input = read_file(argv[1]);

    // lex/parse program into rooted tree:
    Lexer * lex = new_lexer(input);
    Expression * program = parse_program(lex);
    //print_expression(program, lex->symbols, 0);

    // Initialize Function Table:
    ftable = new_queue(NULL);

    // Execute program:
    Queue/*<Thunk>*/ * context = new_queue(NULL);
    Thunk * thunk = new_thunk(HASH_OF_TIMES, program, context);
    execute(thunk, lex->symbols);

    // clean up
    destroy_thunk(thunk);
    destroy_queue(context);
    destroy_expression(program);
    destroy_queue(ftable);
    destroy_lexer(lex);
    destroy_string(input);

    return 0;
}
