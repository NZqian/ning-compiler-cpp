#include "type.hpp"

std::map<BType, std::string> BType2Str{
    {INT, "INT"},
    {VOID, "VOID"},
    {NONE, "NONE"}
};

std::map<Stmt_type, std::string> Stmt_type2str{
    {STMT_SIMPLE, "STMT_SIMPLE"},
    {STMT_IF, "STMT_IF"},
    {STMT_WHILE, "STMT_WHILE"},
    {STMT_FOR, "STMT_FOR"},
    {STMT_CONTINUE, "STMT_CONTINUE"},
    {STMT_BREAK, "STMT_BREAK"},
    {STMT_RETURN, "STMT_RETURN"},
    {STMT_BODY, "STMT_BODY"},
    {STMT_ASSIGN, "STMT_ASSIGN"},
    {STMT_BLANK, "STMT_BLANK"},
    {STMT_EXPR, "STMT_EXPR"}
};