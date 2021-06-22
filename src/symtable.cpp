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

BaseAST *SymTable::SearchTable(const std::string &name)
{
    if(definitions.find(name) == definitions.end())
        return nullptr;
    return definitions[name].pointer;
}