#include "type.hpp"

std::map<BType, std::string> BType2Str
{
    {INT, "INT"},
    {VOID, "VOID"},
    {NONE, "NONE"}
};

std::map<Stmt_type, std::string> Stmt_type2str
{
    {STMT_SIMPLE, "STMT_SIMPLE"},
    {STMT_IF, "STMT_IF"},
    {STMT_WHILE, "STMT_WHILE"},
    {STMT_CONTINUE, "STMT_CONTINUE"},
    {STMT_BREAK, "STMT_BREAK"},
    {STMT_RETURN, "STMT_RETURN"},
    {STMT_BODY, "STMT_BODY"},
    {STMT_ASSIGN, "STMT_ASSIGN"},
    {STMT_BLANK, "STMT_BLANK"},
    {STMT_EXPR, "STMT_EXPR"}
};

std::map<ThreeAddressOp, std::string> ThreeAddressOp2Str
{
    {THREE_OP_ASSIGN, "assign"},
    {THREE_OP_GT, "greater than"},
    {THREE_OP_FUNC_CALL, "func call"},
    {THREE_OP_FUNC_DEF, "func def"},
};

std::map<std::string, ThreeAddressOp> strOp2ThreeAddressOp
{
    {"+", THREE_OP_PLUS},
    {"-", THREE_OP_MINUS},
    {"*", THREE_OP_MUL},
    {"/", THREE_OP_DIV},
    {"%", THREE_OP_MOD},
    {">", THREE_OP_GT},
    {">=", THREE_OP_GE},
    {"<", THREE_OP_LT},
    {"<=", THREE_OP_LE},
    {"&&", THREE_OP_AND},
    {"||", THREE_OP_OR},
};