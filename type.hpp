#pragma once
#include "global.hpp"

enum Visit_type
{
    VISIT_OUTPUT,
    VISIT_JSON,
    VISIT_CHECK,
};

enum Tok_type
{
    TOK_EOF = -127,
    TOK_IDENTIFIER,
    TOK_OPERATOR,
    TOK_SYMBOL,
    TOK_NUMBER,
    TOK_CONST,
    TOK_INT,
    TOK_VOID,
    TOK_IF,
    TOK_ELSE,
    TOK_WHILE,
    TOK_CONTINUE,
    TOK_BREAK,
    TOK_RETURN,
    TOK_UNKNOWN,
};

enum Stmt_type
{
    STMT_SIMPLE,
    STMT_IF,
    STMT_WHILE,
    STMT_FOR,
    STMT_CONTINUE,
    STMT_BREAK,
    STMT_RETURN,
    STMT_BODY,
    STMT_ASSIGN,
    STMT_BLANK
};

/*
enum Expr_type
{
    TYPE
}
*/

const std::set<std::string> symbolTokens{
    "(", ")", "{", "}", "[", "]", ",", ";"};
const std::set<std::string> operatorTokens{
    "+", "-", "*", "/", "%",
    "==", "!=", "<", ">", "<=", ">=",
    "!", "&&", "||"};

enum Func_type
{
    FUNC_INT,
    FUNC_VOID,
    FUNC_UNKNOWN,
};

enum AST_type
{

    AST_UNKNOWN,
};