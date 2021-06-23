#include "AST.hpp"
#include "../frontend/symtable.hpp"
#include "../IR/three_address.hpp"

Visitor::Visitor()
{
    indent = "";
    symtable = std::make_shared<SymTable>();
    threeAddressCode = std::make_shared<ThreeAddressCode>();
    nameChanger = std::make_shared<NameChanger>();
	inLoop = false;
}
