#pragma once

#include "../global/global.hpp"
#include "../global/AST.hpp"

class Definition
{
public:
    DefinitionType type;
    BaseAST *pointer;
    std::vector<std::string> storePlaces;
    int useCount;
    Definition(DefinitionType type, BaseAST *pointer)
        : type(type), pointer(pointer)
    {
        useCount = 0;
    }
    Definition(const Definition &def);
    Definition();
};

class SymTable
{
public:
    std::map<std::string, Definition> definitions;
    bool Insert(BaseAST *, const std::string &name, DefinitionType type);
    bool Delete(const std::string &name);
    //get ast pointer
    BaseAST *SearchTable(const std::string &name);
    Definition *SearchTableDefinition(const std::string &name);
    bool InsertVarSavePos(const std::string &varName, const std::string &pos);
    bool DeleteVarSavePos(const std::string &varName, const std::string &pos);
    void Show();
    //Table();
};

class NameChanger
{
    std::vector<std::vector<std::pair<std::string, std::string>>> namescope;
    std::map<std::string, std::string> funcNameMap;
    std::string curNewName;

public:
    NameChanger() : curNewName("a") {}
    void EnterNameScope();
    void ExitNameScope();
    std::string GetNewVarName(std::shared_ptr<SymTable> symtable);
    void AddChange(std::string oldName, std::shared_ptr<SymTable> symtable);
    std::string UseChange(std::string curName);
    bool InNameScope(std::string curName);
    bool InCurNameScope(std::string curName);
    std::string GetNewFuncName(std::shared_ptr<SymTable> symtable, std::string curName);
    std::string ReplaceFuncName(std::string curName);
};