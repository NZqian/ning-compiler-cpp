#pragma once

#include "global.hpp"
#include "AST.hpp"

class Definition
{
    public:
    DefinitionType type;
    BaseAST* pointer;
    Definition(DefinitionType type, BaseAST *pointer) : type(type), pointer(pointer){}
    Definition(const Definition &def);
    Definition();
};

class SymTable
{
    public:
    std::map<std::string, Definition> definitions;
    bool Insert(BaseAST *, const std::string &name, DefinitionType type);
    BaseAST *SearchTable(const std::string &name);
    //Table();
};

class NameChanger
{
    std::vector<std::vector<std::pair<std::string, std::string> > > namescope;
    std::string curNewName;
    public:
    NameChanger() : curNewName("a"){}
    void EnterNameScope();
    void ExitNameScope();
    std::string GetNewVarName(std::shared_ptr<SymTable> symtable);
    void AddChange(std::string oldName, std::shared_ptr<SymTable> symtable);
    std::string UseChange(std::string curName);
    bool InNameScope(std::string curName);
    bool InCurNameScope(std::string curName);
};