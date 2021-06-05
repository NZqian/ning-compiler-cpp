#include "type.hpp"

std::map<Stmt_type, std::string> Stmt_type2str{
    std::map<Stmt_type, std::string>::value_type(STMT_SIMPLE, "STMT_SIMPLE"),
    std::map<Stmt_type, std::string>::value_type(STMT_IF, "STMT_IF"),
    std::map<Stmt_type, std::string>::value_type(STMT_WHILE, "STMT_WHILE"),
    std::map<Stmt_type, std::string>::value_type(STMT_FOR, "STMT_FOR"),
    std::map<Stmt_type, std::string>::value_type(STMT_CONTINUE, "STMT_CONTINUE"),
    std::map<Stmt_type, std::string>::value_type(STMT_BREAK, "STMT_BREAK"),
    std::map<Stmt_type, std::string>::value_type(STMT_RETURN, "STMT_RETURN"),
    std::map<Stmt_type, std::string>::value_type(STMT_BODY, "STMT_BODY"),
    std::map<Stmt_type, std::string>::value_type(STMT_ASSIGN, "STMT_ASSIGN"),
    std::map<Stmt_type, std::string>::value_type(STMT_BLANK, "STMT_BLANK"),

};