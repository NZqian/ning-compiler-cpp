#pragma once
#include "../IR/three_address.hpp"
#include "../global/global.hpp"

//一个基本块
class BaseBlock
{
public:
    std::vector<std::shared_ptr<BaseBlock>> in, out;
    //包头不包尾
    std::pair<int, int> range;
    std::vector<std::shared_ptr<ThreeAddress>>::iterator start, end;
    BaseBlock(int start, int end, std::vector<std::shared_ptr<BaseBlock>> in, std::vector<std::shared_ptr<BaseBlock>> out);
    BaseBlock(int start, int end);
};

class FlowGraph
{
    //空入基本块
    std::shared_ptr<BaseBlock> in;
    std::shared_ptr<ThreeAddressCode> oriCode;
    void SplitBlock();
    void LinkBlock();

public:
    FlowGraph();
    std::vector<std::shared_ptr<BaseBlock>> blocks;
    void ConstrctGraph(std::shared_ptr<ThreeAddressCode> oriCode);
    void Show();
};