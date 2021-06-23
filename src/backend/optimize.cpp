#include "optimize.hpp"

bool IsTmpVar(const std::string &varName)
{
    if (varName.size() && varName[0] == '@')
    {
        return true;
    }
    return false;
}

Optimizer::Optimizer(std::shared_ptr<ThreeAddressCode> code,
                     std::shared_ptr<SymTable> symtable)
    : curCode(code), symtable(symtable)
{
    flowGraph = std::make_shared<FlowGraph>();
}

void Optimizer::Optimize()
{
    RemoveConstVar();
}

void Optimizer::WeakenExpr(int start, int end)
{
    //var1 = var2 op var3, var3 is literal
    std::string var[3];
    std::vector<std::shared_ptr<Address>> addresses;
    for (int i = start; i < end; i++)
    {
        std::shared_ptr<ThreeAddress> code = curCode->codes[i];
        switch (code->op)
        {
        case THREE_OP_ADD:
        case THREE_OP_MINUS:
        {
            addresses = code->addresses;
            //is literal
            if (addresses[1]->address[0] == '#')
            {
                std::swap(addresses[1], addresses[2]);
            }
            /*
                if (var[1][0] == '#')
                {
                    std::swap(var[1], var[2]);
                }
                std::string valStr = std::string(var[2].begin()+1, var[2].end()));
                */

            std::string valStr = std::string(addresses[2]->address.begin() + 1, addresses[2]->address.end());
            if (std::atoi(valStr.c_str()) == 0)
            {
                curCode->codes[i] = std::make_shared<ThreeAddress>(THREE_OP_ASSIGN, std::vector<std::shared_ptr<Address>>{std::make_shared<Address>(THREE_TMP_VAR, addresses[0]->address), std::make_shared<Address>(addresses[1]->type, addresses[1]->address)});
            }
        }
        break;
        case THREE_OP_MUL:
        {
            addresses = code->addresses;
            //is literal
            if (addresses[1]->address[0] == '#')
            {
                std::swap(addresses[1], addresses[2]);
            }
            std::string valStr = std::string(addresses[2]->address.begin() + 1, addresses[2]->address.end());
            int val = std::atoi(valStr.c_str());
            switch (val)
            {
            case 0:
                curCode->codes[i] = std::make_shared<ThreeAddress>(THREE_OP_ASSIGN, std::vector<std::shared_ptr<Address>>{std::make_shared<Address>(THREE_TMP_VAR, addresses[0]->address), std::make_shared<Address>(THREE_LITERAL, "#0")});
                break;
            case 1:
                curCode->codes[i] = std::make_shared<ThreeAddress>(THREE_OP_ASSIGN, std::vector<std::shared_ptr<Address>>{std::make_shared<Address>(THREE_TMP_VAR, addresses[0]->address), std::make_shared<Address>(addresses[1]->type, addresses[1]->address)});
                break;
            case 2:
                curCode->codes[i] = std::make_shared<ThreeAddress>(THREE_OP_ADD, std::vector<std::shared_ptr<Address>>{std::make_shared<Address>(THREE_TMP_VAR, addresses[0]->address), std::make_shared<Address>(addresses[1]->type, addresses[1]->address), std::make_shared<Address>(addresses[1]->type, addresses[1]->address)});
                break;
            }
        }
        break;
        case THREE_OP_DIV:
        {
            addresses = code->addresses;
            //a / a = 1;
            if (addresses[1]->address == addresses[2]->address)
            {
                curCode->codes[i] = std::make_shared<ThreeAddress>(THREE_OP_ASSIGN, std::vector<std::shared_ptr<Address>>{std::make_shared<Address>(THREE_TMP_VAR, addresses[0]->address), std::make_shared<Address>(THREE_LITERAL, "#0")});
            }
            //0 / a = 0
            else if (std::atoi(addresses[1]->address.c_str()) == 0)
            {
                curCode->codes[i] = std::make_shared<ThreeAddress>(THREE_OP_ASSIGN, std::vector<std::shared_ptr<Address>>{std::make_shared<Address>(THREE_TMP_VAR, addresses[0]->address), std::make_shared<Address>(THREE_LITERAL, "#0")});
            }
            //a / 1 = a
            else if (std::atoi(addresses[2]->address.c_str()) == 1)
            {
                curCode->codes[i] = std::make_shared<ThreeAddress>(THREE_OP_ASSIGN, std::vector<std::shared_ptr<Address>>{std::make_shared<Address>(THREE_TMP_VAR, addresses[0]->address), std::make_shared<Address>(addresses[1]->type, addresses[1]->address)});
            }
        }
        break;
        default:
            //do nothing
            break;
        }
    }
}

void Optimizer::WeakenExpr()
{
    WeakenExpr(0, curCode->codes.size());
}

void Optimizer::RemoveConstVar()
{
    RemoveConstVar(0, curCode->codes.size());
}

void Optimizer::RemoveConstVar(int start, int end)
{
    std::vector<std::string> constToRemove;
    std::map<std::string, int> const2val;
    for (int i = start; i < end; i++)
    {
        std::shared_ptr<ThreeAddress> code = curCode->codes[i];
        if (code->op == THREE_OP_CONST_VAR_DEF)
        {
            std::string constName = code->addresses[0]->address;
            const2val[constName] = std::atoi(std::string(code->addresses[1]->address.begin() + 1, code->addresses[1]->address.end()).c_str());
            constToRemove.push_back(constName);
            code->op = THREE_OP_TO_REMOVE;
        }
    }
    for (int i = start; i < end; i++)
    {
        std::shared_ptr<ThreeAddress> code = curCode->codes[i];
        for (auto address : code->addresses)
        {
            std::string curAddress = address->address;
            //if (std::find(address, ))
            auto iter = std::find(constToRemove.begin(), constToRemove.end(), curAddress);
            if (iter != constToRemove.end())
            {
                address->address = "#" + std::to_string(const2val[curAddress]);
            }
        }
    }
}

void Optimizer::RemoveDeletedThreeAddressCode()
{
    for (auto iter = curCode->codes.begin(); iter != curCode->codes.end();)
    {
        if ((*iter)->op == THREE_OP_TO_REMOVE)
        {
            /*
            if ((*iter)->addresses[0]->type == THREE_TMP_VAR)
            {
                symtable->Delete((*iter)->addresses[0]->address);
            }
            */
            curCode->codes.erase(iter);
        }
        else
        {
            iter++;
        }
    }
}

void Optimizer::RemoveTmpVar()
{
    RemoveTmpVar(0, curCode->codes.size());
}

void Optimizer::RemoveTmpVar(int start, int end)
{
    std::map<std::string, int> tmpVarUseCnt;
    for (int i = start; i < end; i++)
    {
        std::shared_ptr<ThreeAddress> code = curCode->codes[i];
        for (int j = 1; j < code->addresses.size(); j++)
        {
            std::shared_ptr<Address> address = code->addresses[j];
            if (code->op != THREE_OP_ASSIGN && IsTmpVar(address->address))
            {
                tmpVarUseCnt[address->address] += 1;
            }
        }
    }
    for (int i = start; i < end; i++)
    {
        std::shared_ptr<ThreeAddress> code = curCode->codes[i];
        //赋值语句中的左值为临时变量且未被使用
        if (code->op == THREE_OP_ASSIGN && IsTmpVar(code->addresses[1]->address) && tmpVarUseCnt[code->addresses[1]->address] == 0)
        {
            std::string tmpVarName = code->addresses[1]->address;
            std::string replaceVarName = code->addresses[0]->address;
            for (int j = i - 1; j >= start; j--)
            {
                std::shared_ptr<ThreeAddress> prevCode = curCode->codes[j];
                if (prevCode->addresses.size() && prevCode->addresses[0]->address == tmpVarName)
                {
                    prevCode->addresses[0]->address = replaceVarName;
                    break;
                }
            }
            code->op = THREE_OP_TO_REMOVE;
        }
    }
    RemoveDeletedThreeAddressCode();
}

void Optimizer::RemoveDeadCode()
{
    flowGraph->ConstrctGraph(curCode);
    for (auto block : flowGraph->blocks)
    {
        //永远无法到达的基本块
        if (!block->in.size())
        {
            for (int i = block->range.first; i < block->range.second; i++)
            {
                curCode->codes[i]->op = THREE_OP_TO_REMOVE;
            }
        }
    }
    RemoveDeletedThreeAddressCode();
}

void Optimizer::UpdateSymTable()
{
    for (auto code : curCode->codes)
    {
        for (auto address : code->addresses)
        {
            if (address->type == THREE_TMP_VAR)
            {
                symtable->Insert(nullptr, address->address, TMPVAR);
            }
        }
    }
}

void Optimizer::ShowCode()
{
    curCode->Show();
}