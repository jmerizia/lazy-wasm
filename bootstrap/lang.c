#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

#define MAX_INPUT_LEN 10000
#define MAX_TOKEN_LEN 1000
#define SINGLE_CHAR_TOKENS "()[],"


/*
 * QUEUE
 */

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

Queue * new_queue()
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
    return q;
}

void queue_push(Queue * q, void * data)
{
    Node * n = malloc(sizeof(Node));
    n->data = data;
    q->tail->prev->next = n;
    n->prev = q->tail->prev;
    q->tail->prev = n;
    n->next = q->tail;
}

void destroy_queue(Queue * q)
{
    Node * cur, * next;
    for (cur = q->head; cur != q->tail; ) {
        next = cur->next;
        free(cur);
        cur = next;
    }
    free(q->tail);
    free(q);
}

Node * queue_begin(Queue * q) { return q->head->next; }
Node * queue_end(Queue * q) { return q->tail; }


/*
 * EXPRESSIONS
 */

enum ExpressionType;
struct Expression;

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
    Queue * children;
    ExpressionType type;
} typedef Expression;

Expression * new_expression(void);
void destroy_expression(Expression * e);
void print_expression(Expression * e, int d);

Expression * new_expression()
{
    Expression * e = malloc(sizeof(Expression));
    e->children = new_queue();
    return e;
}

void destroy_expression(Expression * e)
{
    Node * cur = queue_begin(e->children),
         * end = queue_end(e->children);
    for (; cur != end; cur = cur->next) {
        destroy_expression(cur->data);
    }
    destroy_queue(e->children);
    free(e);
}

void print_expression(Expression * e, int d)
{
    for (int i = 0; i < d; i++) printf("  ");
    printf("%s : %s\n", ExpressionTypeString[e->type],
            e->value == NULL ? "0" : e->value);
    Node * cur = queue_begin(e->children),
         * end = queue_end(e->children);
    for (; cur != end; cur = cur->next) {
        print_expression(cur->data, d+1);
    }
}


/*
 * LEXING
 */

char input[MAX_INPUT_LEN];
char token[MAX_TOKEN_LEN];
int idx;

bool member_of(char c, char * set)
{
    for (int i = 0; set[i] != '\0'; i++)
        if (c == set[i])
            return true;
    return false;
}

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

    printf("%s\n", token);

    // check length not zero
    if (strlen(token) < 1) {
        printf("Token should not be empty string!\n");
        exit(EXIT_FAILURE);
    }
    // update global program index
    idx = i;
    return true;
}


/*
 * PARSING
 */

Expression * parse_program(void);
bool parse_statement (Expression * root);
bool parse_list      (Expression * root);
bool parse_id        (Expression * root);
bool parse_primitive (Expression * root);

bool parse_primitive(Expression * root)
{
    Expression * e = new_expression();
    int prv_idx = idx;
    if (!lex()) {
        printf("Error: Expected token\n");
        exit(EXIT_FAILURE);
    }
    int len = strlen(token);
    bool is_num = strcmp(token, "0") == 0 ? true : atoi(token) != 0;
    bool is_str = token[0] == '\"' && token[len-1] == '\"';
    if (strcmp(token, "ANY") != 0
            && strcmp(token, "TRUE") != 0
            && strcmp(token, "FALSE") != 0
            && strcmp(token, "NULL") != 0
            && !is_num && !is_str) {
        idx = prv_idx;
        destroy_expression(e);
        return false;
    }
    e->value = malloc((strlen(token)+1) * sizeof(char));
    strcpy(e->value, token);
    e->type = Primitive;
    queue_push(root->children, e);
    printf("primitive finish\n");
    return true;
}

bool parse_id(Expression * root)
{
    Expression * e = new_expression();
    int prv_idx = idx;
    if (!lex()) {
        printf("Error: Expected token\n");
        exit(EXIT_FAILURE);
    }
    bool is_id = true;
    for (int i = 0; token[i] != '\0'; i++) {
        if (!(isalpha(token[i]) || token[i] == '_')) {
            is_id = false;
            break;
        }
    }
    if (!is_id) {
        idx = prv_idx; // go back to previous token
        destroy_expression(e);
        return false;
    }
    e->value = malloc((strlen(token)+1) * sizeof(char));
    strcpy(e->value, token);
    e->type = Id;
    queue_push(root->children, e);
    printf("id finish\n");
    return true;
}

bool parse_list(Expression * root)
{
    Expression * e = new_expression();
    int prv_idx = idx;
    if (!lex()) {
        printf("Error: Expected token\n");
        exit(EXIT_FAILURE);
    }
    if (strcmp(token, "[") != 0) {
        destroy_expression(e);
        idx = prv_idx;
        return false;
    }
    while (parse_id(e) || parse_statement(e) || parse_list(e) || parse_primitive(e));
    lex();
    if (strcmp(token, "]") != 0) {
        // error
        printf("Error: Expected closing paren!\n");
        exit(EXIT_FAILURE);
    }
    e->value = NULL;
    e->type = List;
    queue_push(root->children, e);
    printf("list finish\n");
    return true;
}

bool parse_statement(Expression * root)
{
    int prv_idx = idx;
    if (!lex()) return false;
    Expression * e = new_expression();
    if (strcmp(token, "(") != 0 || !parse_id(e)) {
        destroy_expression(e);
        idx = prv_idx; // go back to previous token
        return false;
    }
    while (parse_id(e) || parse_statement(e) || parse_list(e) || parse_primitive(e));
    lex();
    if (strcmp(token, ")") != 0) {
        // error
        printf("Error: Expected closing paren!\n");
        exit(EXIT_FAILURE);
    }
    e->value = NULL;
    e->type = Statement;
    queue_push(root->children, e);
    printf("statmt finish\n");
    return true;
}

Expression * parse_program()
{
    Expression * e = new_expression();
    while (parse_statement(e));
    e->value = NULL;
    e->type = Program;
    printf("prog finish\n");
    return e;
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
    Expression * e = parse_program();
    print_expression(e, 0);

    return 0;
}
