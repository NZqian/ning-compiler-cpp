#include "AST.hpp"
#include "symtable.hpp"
#include "three_address.hpp"

Visitor::Visitor()
{
    indent = "";
    symtable = std::make_shared<SymTable>();
    threeAddressCode = std::make_shared<ThreeAddressCode>();
}
