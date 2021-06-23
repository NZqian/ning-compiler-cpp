#include "symtable.hpp"

Definition::Definition(const Definition &def)
{
    this->type = def.type;
    this->pointer = def.pointer;
}

Definition::Definition()
{
    this->type = DEFINIT;
    this->pointer = nullptr;
}

/*
SymTable::SymTable()
{
}
*/

bool SymTable::Insert(BaseAST *definition, const std::string &name, DefinitionType type)
{
    if(SearchTable(name))
        return false;
    Definition def(type, definition);
    definitions[name] = def;
    return true;
}

bool SymTable::Delete(const std::string &name)
{
    auto iter = definitions.find(name);
    if(iter != definitions.end())
    {
        definitions.erase(iter);
        return true;
    }
    return false;
}

BaseAST *SymTable::SearchTable(const std::string &name)
{
    if(definitions.find(name) == definitions.end())
        return nullptr;
    return definitions[name].pointer;
}

Definition *SymTable::SearchTableDefinition(const std::string &name)
{
    if(definitions.find(name) == definitions.end())
        return nullptr;
    return &definitions[name];
}

bool SymTable::InsertVarSavePos(const std::string &varName, const std::string &pos)
{
    Definition *definition = nullptr;
    definition = SearchTableDefinition(varName);
    if(!definition)
    {
        return false;
    }
    definition->storePlace.push_back(pos);
    return true;
}

bool SymTable::DeleteVarSavePos(const std::string &varName, const std::string &pos)
{
    Definition *definition = nullptr;
    definition = SearchTableDefinition(varName);
    if(!definition)
    {
        return false;
    }
    auto iter = std::find(definition->storePlace.begin(), definition->storePlace.end(), pos);
    if (iter == definition->storePlace.end())
    {
        return false;
    }
    definition->storePlace.erase(iter);
    return true;
}

void SymTable::Show()
{
    for (auto definition : definitions)
    {
        std::cout << definition.first << std::endl;
    }
}