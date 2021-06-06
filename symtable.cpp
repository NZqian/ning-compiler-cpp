#include "symtable.hpp"

SymTable::SymTable()
{
    tables = nullptr;
}

void SymTable::AddTable()
{
    std::shared_ptr<Table> newTable = std::make_shared<Table>();
    //if(tables)
        newTable->prevTable = tables;
    //else
        //newTable->prevTable = nullptr;
    tables = newTable;
}

void SymTable::DeleteTable()
{
    tables = tables->prevTable;
}

bool SymTable::Insert(BaseAST *definition, const std::string &name, DefinitionType type)
{
    for(auto curDefinition : tables->definitions)
    {
        if(curDefinition.name == name)
            return false;
    }
    tables->definitions.emplace_back(Definition(name, type, definition));
    return true;
}

BaseAST *SymTable::SearchTable(const std::string &name, DefinitionType type)
{
    std::shared_ptr<Table> table = tables;
    while(table)
    {
        for(auto definition : table->definitions)
        {
            if(definition.name == name && definition.type == type)
            {
                return definition.pointer;
            }
        }
        table = table->prevTable;
    }
    return nullptr;
}