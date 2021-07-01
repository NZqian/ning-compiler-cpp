#pragma once
#include "../global/global.hpp"

enum DefinitionType
{
    FUNCTION,
    VARIABLE,
	TMPVAR,
    DEFINIT,
};

enum BType
{
    INT,
    VOID,
    NONE,
};

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
    STMT_CONTINUE,
    STMT_BREAK,
    STMT_RETURN,
    STMT_BODY,
    STMT_ASSIGN,
    STMT_BLANK,
    STMT_EXPR,
};

enum ThreeAddressOp
{
    THREE_OP_PARAM,
	THREE_OP_LABEL,
    THREE_OP_ASSIGN,
    THREE_OP_ASSIGN_SHIFT,
    THREE_OP_ASSIGN_ARRAY,
    THREE_OP_ADD,
    THREE_OP_MINUS,
    THREE_OP_MUL,
    THREE_OP_DIV,
    THREE_OP_MOD,
    THREE_OP_CMP,
    THREE_OP_GT,
    THREE_OP_GE,
    THREE_OP_LT,
    THREE_OP_LE,
    THREE_OP_EQ,
    THREE_OP_NE,
    THREE_OP_NOT,
    THREE_OP_AND,
    THREE_OP_OR,
    THREE_OP_PUSH_STACK,
    THREE_OP_FUNC_CALL,
    THREE_OP_FUNC_DEF,
    THREE_OP_VAR_DEF,
	THREE_OP_CONST_VAR_DEF,
    THREE_OP_ARRAY_DEF,
	THREE_OP_CONST_ARRAY_DEF,
    THREE_OP_INDEX,
    THREE_OP_VAR_DECL,
    THREE_OP_RETURN,
    THREE_OP_JUMP,
    THREE_OP_JUMP_GT,
    THREE_OP_JUMP_GE,
    THREE_OP_JUMP_LT,
    THREE_OP_JUMP_LE,
    THREE_OP_JUMP_EQ,
    THREE_OP_JUMP_NE,
	THREE_OP_TO_REMOVE,
};

enum ThreeAddressType
{
    THREE_TMP_VAR,
    THREE_VARIABLE,
    THREE_LITERAL,
    THREE_FUNCTION,
    THREE_EXPR,
    THREE_LABEL,
    THREE_SHIFT,
    THREE_INDEX,
};

/*
enum AddressType
{
    ADDRESS_FUNCTION,
    ADDRESS_VARIABLE,
    ADDRESS_LITERAL,
    ADDRESS_THREE,
}
*/

extern std::map<Stmt_type, std::string> Stmt_type2str;
extern std::map<BType, std::string> BType2Str;
extern std::map<ThreeAddressOp, std::string> ThreeAddressOp2Str;
extern std::map<ThreeAddressOp, ThreeAddressOp> ExprOp2JumpOp;
extern std::map<ThreeAddressOp, ThreeAddressOp> ExprOp2JumpOpReversed;
extern std::map<ThreeAddressType, std::string> ThreeAddressType2Str;
extern std::map<std::string, ThreeAddressOp> strOp2ThreeAddressOp;

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