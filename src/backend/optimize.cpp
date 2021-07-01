#include "optimize.hpp"

bool IsTmpVar(const std::string &varName)
{
    if (varName.size() && varName[0] == '@')
    {
        return true;
    }
    return false;
}

int Is2n(int val)
{
    int n = 0;
    while (val > 1)
    {
        if (val % 2 != 0)
        {
            return false;
        }
        else
        {
            val >>= 1;
            n += 1;
        }
    }
    return n;
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

            if (addresses[2]->type == THREE_LITERAL)
            {
                std::string valStr = std::string(addresses[2]->address.begin() + 1, addresses[2]->address.end()); if (std::atoi(valStr.c_str()) == 0)
                {
                    curCode->codes[i] = std::make_shared<ThreeAddress>(THREE_OP_ASSIGN, std::vector<std::shared_ptr<Address>>{std::make_shared<Address>(THREE_TMP_VAR, addresses[0]->address), std::make_shared<Address>(addresses[1]->type, addresses[1]->address)});
                }
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
            default:
            {
                if(Is2n(val))
                    curCode->codes[i] = std::make_shared<ThreeAddress>(THREE_OP_ASSIGN_SHIFT, std::vector<std::shared_ptr<Address>>{std::make_shared<Address>(THREE_TMP_VAR, addresses[0]->address), std::make_shared<Address>(addresses[1]->type, addresses[1]->address), std::make_shared<Address>(THREE_SHIFT, "ASL #" + std::to_string(val))});
            }
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
            else if (addresses[1]->address[0] == '#' &&
                     std::atoi(std::string(addresses[1]->address.begin() + 1, addresses[1]->address.end()).c_str()) == 0)
            {
                curCode->codes[i] = std::make_shared<ThreeAddress>(THREE_OP_ASSIGN, std::vector<std::shared_ptr<Address>>{std::make_shared<Address>(THREE_TMP_VAR, addresses[0]->address), std::make_shared<Address>(THREE_LITERAL, "#0")});
            }
            //a / 1 = a
            else if (addresses[2]->address[0] == '#' &&
                     std::atoi(std::string(addresses[2]->address.begin() + 1, addresses[2]->address.end()).c_str()) == 1)
            {
                curCode->codes[i] = std::make_shared<ThreeAddress>(THREE_OP_ASSIGN, std::vector<std::shared_ptr<Address>>{std::make_shared<Address>(THREE_TMP_VAR, addresses[0]->address), std::make_shared<Address>(addresses[1]->type, addresses[1]->address)});
            }
            //2的指数
            else if (addresses[2]->address[0] == '#' &&
                     Is2n(std::atoi(std::string(addresses[2]->address.begin() + 1, addresses[2]->address.end()).c_str())))
            {
                curCode->codes[i] = std::make_shared<ThreeAddress>(THREE_OP_ASSIGN_SHIFT, std::vector<std::shared_ptr<Address>>{
                                                                                              std::make_shared<Address>(THREE_TMP_VAR, addresses[0]->address),
                                                                                              std::make_shared<Address>(addresses[1]->type, addresses[1]->address),
                                                                                              std::make_shared<Address>(THREE_SHIFT, "ASR #" + std::to_string(Is2n(std::atoi(std::string(addresses[2]->address.begin() + 1, addresses[2]->address.end()).c_str()))))});
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
            Definition *def = symtable->SearchTableDefinition(code->addresses[0]->address);
            VariableAST *var = (VariableAST*)def->pointer;
            if (!var->dimensions.size())
            {
                std::string constName = code->addresses[0]->address;
                const2val[constName] = std::atoi(std::string(code->addresses[1]->address.begin() + 1, code->addresses[1]->address.end()).c_str());
                constToRemove.push_back(constName);
                code->op = THREE_OP_TO_REMOVE;
            }
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
                address->type = THREE_LITERAL;
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
        //return的addresses可能为空，不能运行到下面
        if (code->op == THREE_OP_RETURN)
        {
            continue;
        }
        //赋值语句中的左值为临时变量且未被使用
        if (IsTmpVar(code->addresses[0]->address) && tmpVarUseCnt[code->addresses[0]->address] == 0)
        {
            std::string tmpVarName = code->addresses[0]->address;
            //std::string replaceVarName = code->addresses[0]->address;
            for (int j = i + 1; j < end; j++)
            {
                std::shared_ptr<ThreeAddress> afterCode = curCode->codes[j];
                if (afterCode->addresses.size() > 1 && afterCode->addresses[1]->address == tmpVarName)
                {
                    code->addresses[0]->address = afterCode->addresses[0]->address;
                    //afterCode->addresses[0]->address = replaceVarName;
                    afterCode->op = THREE_OP_TO_REMOVE;
                    break;
                }
            }
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

//等号左侧的数组元素加上标记，方便与等号右侧数组元素值区分
void Optimizer::ProcessArrayItemName()
{
    for (auto code : curCode->codes)
    {
        if (code->op == THREE_OP_ASSIGN)
        {
            std::string leftAddress = code->addresses[0]->address;
            int pos = -1;
            for (int i = 0; i < leftAddress.size(); i++)
            {
                if (leftAddress[i] == '$')
                {
                    pos = i;
                    break;
                }
            }
            //是数组元素变量，在等号左侧
            //维度不可能为符号，用!标记
            if (pos != -1)
            {
                code->addresses[0]->address += "$!";
            }
        }
    }
}

void Optimizer::ShowCode()
{
    curCode->Show();
}