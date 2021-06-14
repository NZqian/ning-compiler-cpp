#pragma once

#include "global.hpp"
#include "AST.hpp"

class Definition
{
    public:
    std::string name;
    DefinitionType type;
    BaseAST* pointer;
    Definition(const std::string &name, DefinitionType type, BaseAST *pointer) : name(name), type(type), pointer(pointer){}
};

class Table
{
    public:
    std::shared_ptr<Table> prevTable;
    std::vector<Definition> definitions;
    //Table();
};

class SymTable
{
    public:
    std::shared_ptr<Table> tables;
    SymTable();
    void AddTable();
    void DeleteTable();

    bool Insert(BaseAST *, const std::string &name, DefinitionType type);
    BaseAST *SearchTable(const std::string &name, DefinitionType type);
};