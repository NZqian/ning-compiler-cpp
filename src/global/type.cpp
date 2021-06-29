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
    {THREE_OP_PARAM, "param"},
	{THREE_OP_LABEL, "label"},
    {THREE_OP_ASSIGN, "assign"},
    {THREE_OP_ASSIGN_SHIFT, "assign shift"},
    {THREE_OP_ASSIGN_ARRAY, "assign array"},
    {THREE_OP_ADD, "expr +"},
    {THREE_OP_MINUS, "expr -"},
    {THREE_OP_MUL, "expr *"},
    {THREE_OP_DIV, "expr /"},
    {THREE_OP_MOD, "expr %"},
    {THREE_OP_AND, "expr &&"},
    {THREE_OP_OR, "expr ||"},
    {THREE_OP_CMP, "compare"},
    {THREE_OP_GT, "greater than"},
    {THREE_OP_GE, "greater equal"},
    {THREE_OP_LT, "less than"},
    {THREE_OP_LE, "less equal"},
    {THREE_OP_EQ, "equal"},
    {THREE_OP_NE, "not equal"},
    {THREE_OP_PUSH_STACK, "push"},
    {THREE_OP_FUNC_CALL, "func call"},
    {THREE_OP_FUNC_DEF, "func def"},
    {THREE_OP_ARRAY_DEF, "array def"},
    {THREE_OP_CONST_ARRAY_DEF, "const array def"},
    {THREE_OP_VAR_DEF, "var def"},
    {THREE_OP_CONST_VAR_DEF, "const var def"},
    {THREE_OP_VAR_DECL, "var decl"},
    {THREE_OP_RETURN, "return"},
    {THREE_OP_JUMP, "jump"},
    {THREE_OP_JUMP_GT, "jump gt"},
    {THREE_OP_JUMP_GE, "jump ge"},
    {THREE_OP_JUMP_LT, "jump lt"},
    {THREE_OP_JUMP_LE, "jump le"},
    {THREE_OP_JUMP_EQ, "jump eq"},
    {THREE_OP_JUMP_NE, "jump ne"},
	{THREE_OP_TO_REMOVE, "deleted"},
};

std::map<ThreeAddressOp, ThreeAddressOp> ExprOp2JumpOp
{
    {THREE_OP_GT, THREE_OP_JUMP_GT},
    {THREE_OP_GE, THREE_OP_JUMP_GE},
    {THREE_OP_LT, THREE_OP_JUMP_LT},
    {THREE_OP_LE, THREE_OP_JUMP_LE},
    {THREE_OP_EQ, THREE_OP_JUMP_EQ},
    {THREE_OP_NE, THREE_OP_JUMP_NE},
};

std::map<ThreeAddressOp, ThreeAddressOp> ExprOp2JumpOpReversed
{
    {THREE_OP_GT, THREE_OP_JUMP_LE},
    {THREE_OP_GE, THREE_OP_JUMP_LT},
    {THREE_OP_LT, THREE_OP_JUMP_GE},
    {THREE_OP_LE, THREE_OP_JUMP_GT},
    {THREE_OP_EQ, THREE_OP_JUMP_NE},
    {THREE_OP_NE, THREE_OP_JUMP_EQ},
};

std::map<std::string, ThreeAddressOp> strOp2ThreeAddressOp
{
    {"+", THREE_OP_ADD},
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
    {"==", THREE_OP_EQ},
    {"!=", THREE_OP_NE},
};