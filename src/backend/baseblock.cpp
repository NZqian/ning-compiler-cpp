#include "baseblock.hpp"

BaseBlock::BaseBlock(int start, int end,
                     std::vector<std::shared_ptr<BaseBlock>> in,
                     std::vector<std::shared_ptr<BaseBlock>> out)
    : range(std::make_pair(start, end)), in(std::move(in)), out(std::move(out))
{
}

BaseBlock::BaseBlock(int start, int end)
    : range(std::make_pair(start, end))
{
}

FlowGraph::FlowGraph()
{
    in = std::make_shared<BaseBlock>(-1, -1);
}

void FlowGraph::SplitBlock()
{    
    for(int i = 0; i < oriCode->codes.size(); i++)
    {
        //if 
    }
    std::vector<int> splitPoses;
    splitPoses.push_back(0);
    for(int i = 0; i < oriCode->codes.size(); i++)
    {
        std::shared_ptr<ThreeAddress> threeAddressCode = oriCode->codes[i];
        if(threeAddressCode->op == THREE_OP_JUMP
        || threeAddressCode->op == THREE_OP_JUMP_EQ
        || threeAddressCode->op == THREE_OP_JUMP_NE
        || threeAddressCode->op == THREE_OP_JUMP_GT
        || threeAddressCode->op == THREE_OP_JUMP_GE
        || threeAddressCode->op == THREE_OP_JUMP_LT
        || threeAddressCode->op == THREE_OP_JUMP_LE
        || threeAddressCode->op == THREE_OP_FUNC_CALL
        || threeAddressCode->op == THREE_OP_RETURN
        )
        {
            splitPoses.push_back(i+1);
        }
        else if(threeAddressCode->op == THREE_OP_LABEL
        || threeAddressCode->op == THREE_OP_FUNC_DEF)
        {
            splitPoses.push_back(i);
        }
    }
    int left, right;
    for(int i = 0; i < splitPoses.size() - 1; i++)
    {
        left = splitPoses[i];
        right = splitPoses[i+1];
        /*
        if (oriCode->codes[left]->op == THREE_OP_FUNC_DEF)
            left++;
            */
        if(right > left)
            blocks.push_back(std::make_shared<BaseBlock>(left, right));
    }
    left = splitPoses.back();
    right = oriCode->codes.size();
    /*
    if (oriCode->codes[left]->op == THREE_OP_FUNC_DEF)
        left++;
    */
    if(right > left) 
        blocks.push_back(std::make_shared<BaseBlock>(left, right));


}

void FlowGraph::LinkBlock()
{
    //for(auto block : blocks)
    for(int i = 0; i < blocks.size(); i++)
    {
        std::shared_ptr<BaseBlock> block = blocks[i];
        std::shared_ptr<ThreeAddress> startCode, endCode;
        startCode = oriCode->codes[block->range.first];
        //second为尾前一个
        endCode = oriCode->codes[block->range.second-1];
        //label可以通过跳转得到，也可以通过上一个指令转移得到
        if (startCode->op == THREE_OP_LABEL)
        {
            //上一条指令而来
            block->in.push_back(blocks[i-1]);
            //遍历得到跳转到此的块
            for(auto otherBlock : blocks)
            {
                if (otherBlock == block)
                {
                    continue;
                }
                std::shared_ptr<ThreeAddress> otherStartCode, otherEndCode;
                otherStartCode = oriCode->codes[otherBlock->range.first];
                //second为尾前一个
                otherEndCode = oriCode->codes[otherBlock->range.second-1];
                if (otherEndCode->op == THREE_OP_JUMP
                || otherEndCode->op == THREE_OP_JUMP_EQ
                || otherEndCode->op == THREE_OP_JUMP_NE
                || otherEndCode->op == THREE_OP_JUMP_GT
                || otherEndCode->op == THREE_OP_JUMP_GE
                || otherEndCode->op == THREE_OP_JUMP_LT
                || otherEndCode->op == THREE_OP_JUMP_LE
                )
                {
                    block->in.push_back(otherBlock);
                }
            }
        }
        else if (startCode->op == THREE_OP_FUNC_DEF)
        {
            for(auto otherBlock : blocks)
            {
                if (otherBlock == block)
                {
                    continue;
                }
                std::shared_ptr<ThreeAddress> otherStartCode, otherEndCode;
                otherStartCode = oriCode->codes[otherBlock->range.first];
                //second为尾前一个
                otherEndCode = oriCode->codes[otherBlock->range.second-1];
                if (otherEndCode->op == THREE_OP_FUNC_CALL
                    && otherEndCode->addresses[0]->address == startCode->addresses[0]->address
                )
                {
                    block->in.push_back(otherBlock);
                }
            }
        }
        //普通语句，通过上一个指令转移得到      
        else
        {
            //全局变量，则in为上一个块，main为下一个块
            if(i == 0)
            {
                in->out.push_back(block);
                block->in.push_back(in);
                std::shared_ptr<BaseBlock> main;
                for(auto block : blocks)
                {
                    std::shared_ptr<ThreeAddress> threeAddressCode;
                    threeAddressCode = oriCode->codes[block->range.first];
                    if (threeAddressCode->op == THREE_OP_FUNC_DEF
                        && threeAddressCode->addresses[0]->address == "main")
                    {
                        main = block;
                    }
                }
                block->out.push_back(main);
            }
            else
            {
                block->in.push_back(blocks[i-1]);
                blocks[i-1]->out.push_back(block);
            }
        }

        if (endCode->op == THREE_OP_JUMP
            || endCode->op == THREE_OP_JUMP_EQ
            || endCode->op == THREE_OP_JUMP_NE
            || endCode->op == THREE_OP_JUMP_GT
            || endCode->op == THREE_OP_JUMP_GE
            || endCode->op == THREE_OP_JUMP_LT
            || endCode->op == THREE_OP_JUMP_LE
        )
        {
            for(auto otherBlock : blocks)
            {
                if (otherBlock == block)
                {
                    continue;
                }
                std::shared_ptr<ThreeAddress> otherStartCode, otherEndCode;
                otherStartCode = oriCode->codes[otherBlock->range.first];
                //second为尾前一个
                otherEndCode = oriCode->codes[otherBlock->range.second-1];
                if (otherStartCode->op == THREE_OP_LABEL)
                {
                    block->out.push_back(otherBlock);
                }
            }
        }
        else if (endCode->op == THREE_OP_FUNC_CALL)
        {
            for(auto otherBlock : blocks)
            {
                if (otherBlock == block)
                {
                    continue;
                }
                std::shared_ptr<ThreeAddress> otherStartCode, otherEndCode;
                otherStartCode = oriCode->codes[otherBlock->range.first];
                //second为尾前一个
                otherEndCode = oriCode->codes[otherBlock->range.second-1];
                if (otherStartCode->op == THREE_OP_FUNC_DEF)
                {
                    block->out.push_back(otherBlock);
                }
            }
        }
        //普通语句，可以转移到下一个指令
        else
        {
            if (startCode->op != THREE_OP_FUNC_DEF)
            {
                //无全局变量或全局变量已被处理
                /*
                if (!(in->out.size() && i == 0))
                i == 0 && 
                */
                if(i == 0 && in->out.size())
                {

                }
                else
                {
                    if(i + 1 < blocks.size())
                    {
                        block->out.push_back(blocks[i+1]);
                        blocks[i+1]->in.push_back(block);
                    }
                }
            }
        }
    }
    //没有全局变量，in的下一个块应该是对main的调用
    if(!in->out.size())
    {
        std::shared_ptr<BaseBlock> main;
        for(auto block : blocks)
        {
            std::shared_ptr<ThreeAddress> threeAddressCode;
            threeAddressCode = oriCode->codes[block->range.first];
            if (threeAddressCode->op == THREE_OP_FUNC_DEF
                && threeAddressCode->addresses[0]->address == "main")
            {
                main = block;
            }
        }
        in->out.push_back(main);
    }
}

void FlowGraph::ConstrctGraph(std::shared_ptr<ThreeAddressCode> oriCode)
{
    in = std::make_shared<BaseBlock>(-1, -1);
    blocks.clear();
    this->oriCode = oriCode;
    SplitBlock();
    std::cout << "split done" << std::endl;
    LinkBlock();
    std::cout << "link done" << std::endl;
}

void FlowGraph::Show()
{
    std::map<void*, int> address2index;
    address2index[in.get()] = -1;
    for (int i = 0; i < blocks.size(); i++)
    {
        address2index[blocks[i].get()] = i;
    }

    for (int i = 0; i < blocks.size(); i++)
    {
        std::cout << "Block " << i << ":" << std::endl;

        std::cout << "in block addresses: " << std::endl;
        for(int j = 0; j < blocks[i]->in.size(); j++)
        {
            std::cout << address2index[blocks[i]->in[j].get()] << ' ';
        }
        std::cout << std::endl;

        std::cout << "out block addresses: " << std::endl;
        for(int j = 0; j < blocks[i]->out.size(); j++)
        {
            std::cout << address2index[blocks[i]->out[j].get()] << ' ';
        }
        std::cout << std::endl;

        for(int j = blocks[i]->range.first; j < blocks[i]->range.second; j++)
        {
            //std::cout << oriCode->codes[j] << std::endl;
            oriCode->codes[j]->Show();
        }
    }
}