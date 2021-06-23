#include "../global/global.hpp"
#include "../IR/three_address.hpp"
#include "baseblock.hpp"

class Optimizer
{
    std::shared_ptr<ThreeAddressCode> curCode;
    std::shared_ptr<FlowGraph> flowGraph;
    std::shared_ptr<SymTable> symtable;

public:
    Optimizer(std::shared_ptr<ThreeAddressCode> code, std::shared_ptr<SymTable> symtable);
    void Optimize();
    //为方便作用于基本块，均添加范围作为参数，范围包头不包尾部
    void WeakenExpr(int start, int end);
    void WeakenExpr();
    void RemoveConstVar(int start, int end);
    void RemoveConstVar();
    void RemoveTmpVar(int start, int end);
    void RemoveTmpVar();
    void RemoveDeletedThreeAddressCode();
    void RemoveDeadCode();
    void UpdateSymTable();
    void ShowCode();
};