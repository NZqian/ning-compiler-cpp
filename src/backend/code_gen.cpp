#include "code_gen.hpp"

bool Register::HaveVar(const std::string &varName)
{
    for (auto var : storedVars)
    {
        if (var == varName)
        {
            return true;
        }
    }
    return false;
}

bool Register::InsertVar(const std::string &varName)
{
    if (HaveVar(varName))
    {
        return false;
    }
    storedVars.push_back(varName);
    return true;
}

bool Register::RemoveVar(const std::string &varName)
{
    if (!HaveVar(varName))
    {
        return false;
    }
    auto iter = storedVars.begin();
    while (iter != storedVars.end())
    {
        if ((*iter) == varName)
        {
            storedVars.erase(iter);
            return true;
        }
        iter++;
    }
}

void Register::Clear()
{
    if(storedVars.size())
        storedVars.clear();
}

//通过寄存器名获取到指针
std::shared_ptr<Register> CodeGener::FindRegByName(const std::string &regName)
{
    for (auto reg : registers)
    {
        if (reg->regName == regName)
        {
            return reg;
        }
    }
    return nullptr;
}

//根据保存的变量名找到寄存器指针
std::shared_ptr<Register> CodeGener::FindRegByVar(const std::string &varName)
{
    for (auto reg : registers)
    {
        if (reg->HaveVar(varName))
        {
            return reg;
        }
    }
    return nullptr;
}

std::string CodeGener::AllocReg(const std::string &varName)
{
    for (int i = registers.size() - 1; i >= 0; i--)
    {
        std::shared_ptr<Register> reg = registers[i];
        if (reg->storedVars.size() == 0)
        {
            reg->InsertVar(varName);
            return reg->regName;
        }
    }
    return "";
}

void CodeGener::ClearRegs(int startPos)
{
    std::shared_ptr<Register> reg;
    for (int i = startPos; i < registers.size(); i++)
    {
        reg = registers[i];
        reg->storedVars.clear();
    }
}

CodeGener::CodeGener(const std::string &outputFilename, std::shared_ptr<ThreeAddressCode> codes, std::shared_ptr<SymTable> symtable)
    : outputFilename(outputFilename), codes(codes), symtable(symtable)
{
    //r0-r10
    for (int i = 0; i < 11; i++)
    {
        registers.push_back(std::make_shared<Register>("r" + std::to_string(i)));
    }
    fileWriter = std::fstream(outputFilename, std::fstream::out);
    codePos = 0;
}

void CodeGener::SpillReg(const std::string &regName)
{
    std::shared_ptr<Register> reg2remove;
    for (auto reg : registers)
    {
        if (reg->regName == regName)
        {
            reg2remove = reg;
        }
    }
    for (std::string var2remove : reg2remove->storedVars)
    {
        Definition* varDef = symtable->SearchTableDefinition(var2remove);
        //varDef->storePlaces.push_back("tobeadd");
        WriteStr(reg2remove->regName, varDef->storePlaces[0]);
    }
}

void CodeGener::SpillAllReg()
{
    for (auto reg : registers)
    {
        for (std::string var2remove : reg->storedVars)
        {
            Definition* varDef = symtable->SearchTableDefinition(var2remove);
            //varDef->storePlaces.push_back("tobeadd");
            WriteStr(reg->regName, varDef->storePlaces[0]);
        }
    }
}

void CodeGener::SpillAReg()
{
    std::shared_ptr<Register> useLessReg;
    int minUseCnt = INT32_MAX;
    for (auto reg : registers)
    {
        int curUseCnt = 0;
        for (auto var : reg->storedVars)
        {
            Definition* varDef = symtable->SearchTableDefinition(var);
            curUseCnt += varDef->useCount;
        }
        if (curUseCnt < minUseCnt)
        {
            minUseCnt = curUseCnt;
            useLessReg = reg;
        }
    }
    for (std::string var2remove : useLessReg->storedVars)
    {
        Definition* varDef = symtable->SearchTableDefinition(var2remove);
        WriteStr(useLessReg->regName, varDef->storePlaces[0]);
    }
}

bool IsLiteral(const std::string &str)
{
    if (str.size() && (str[0] == '#') ||
     (std::string(str.begin(), str.begin() + 3) == "ASR")
     ||(std::string(str.begin(), str.begin() + 3) == "ASR") )
    {
        return true;
    }
    return false;
}

bool IsReg(const std::string &str)
{
    if (str.size() && str[0] == 'r' && str[1] >= '0' && str[1] <= '9')
    {
        return true;
    }
    return false;
}

//flag == 1表示获取等式左边变量的寄存器
//等式左边的的值被修改，应该删除寄存器中保存的其他变量
std::string CodeGener::GetRegister(const std::string &var, bool flag)
{
    if (var == "return_value")
    {
        return "r0";
    }
    
    if (var[0] == '#' || (var[0] == 'r' && (var[1] >= '0' && var[1] <='9')))
    {
        return var;
    }

    std::shared_ptr<Register> reg = FindRegByVar(var);
    if (reg)
    {
        return reg->regName;
    }
    //当前变量不存在任何一个寄存器中，分配一个。

    std::string regName = AllocReg(var);
    if (regName != "")
    {
        //非临时变量，先读取
        //if (var[0] != '@')
        {
            Definition *varDef = symtable->SearchTableDefinition(var);
            FindRegByName(regName)->InsertVar(var);
            if (varDef->storePlaces[0][0] == '.')
                WriteLdr(regName, varDef->storePlaces[0]);
            else
                WriteLdr(regName, "[" + std::string(varDef->storePlaces[0] + "]"));
        }
        return regName;
    }
    else
    {
        //spill后递归一次
        SpillAReg();
        return GetRegister(var, flag);
    }

    return var;
}

//将寄存器中的值存到其他地方
/*
void CodeGener::EmptyRegister(const std::string &reg)
{
    for
}
*/

void CodeGener::GenGlobalVar()
{
    std::shared_ptr<ThreeAddress> code;
    for (;; codePos++)
    {
        code = codes->codes[codePos];
        //if (code->op != THREE_OP_VAR_DEF && code->op != THREE_OP_VAR_DECL && code->op != THREE_OP_CONST_VAR_DEF)
        if (code->op != THREE_OP_VAR_DEF && code->op != THREE_OP_VAR_DECL && code->op != THREE_OP_CONST_VAR_DEF)
        {
            break;
        }
        VariableAST *var = (VariableAST *)symtable->SearchTable(code->addresses[0]->address);
        fileWriter << "\t.data\n";
        fileWriter << var->name << ":\n";
        if (var->val)
        {
            //array
            if (var->dimensions.size())
            {
            }
            else
            {
                fileWriter << "\t.word\t" << std::string(code->addresses[1]->address.begin() + 1, code->addresses[1]->address.end()) << "\n";
            }
        }
        else
        {
            fileWriter << "\t.word\t0\t"
                       << "\n";
        }
    }
    fileWriter << "\t.text\n";
    for(int i = 0; i < codePos; i++)
    {
        code = codes->codes[i];
        VariableAST *var = (VariableAST *)symtable->SearchTable(code->addresses[0]->address);
        fileWriter << "." << var->name << "_addr:\n";
        fileWriter << "\t.word\t" << var->name << "\n";
        symtable->InsertVarSavePos(var->name, "." + std::string(var->name + "_addr"));
    }
}

void CodeGener::PushCallParam()
{
    //前四个参数放在r0-r3寄存器中
    for (int i = 0; i < 4; i++)
    {
        if (codes->codes[codePos]->op != THREE_OP_PUSH_STACK)
        {
            /*
            for (int j = i; j < registers->registers.size(); j++)
            {
                SpillReg(registers->registers[j]->regName);
            }
            */
            codePos--;
            return;
        }
        std::string regName = "r" + std::to_string(i);
        //registers->EmptyReg(regName);
        SpillReg(regName);
        WriteMov(regName, GetRegister(codes->codes[codePos]->addresses[0]->address, 0));
        codePos++;
    }
    int tmpPos = codePos;
    while(codes->codes[codePos]->op == THREE_OP_PUSH_STACK)
    {
        codePos++;
    }
    //反向压栈
    for (int i = codePos - 1; i >= tmpPos; i--)
    {
        std::shared_ptr<ThreeAddress> code = codes->codes[i];
        std::string regName = "";
        //不能push常数
        if (code->addresses[0]->type == THREE_LITERAL)
        {
            SpillReg("r5");
            regName = "r5";
        }
        else
        {
            regName = code->addresses[0]->address;
        }
        WriteMov(regName, code->addresses[0]->address);
        WritePush(regName);
    }
    /*
    for (int j = 4; j < registers->registers.size(); j++)
    {
        SpillReg(registers->registers[j]->regName);
    }
    */
    codePos--;
}

void CodeGener::GenFunction()
{
    std::shared_ptr<ThreeAddress> code;
    code = codes->codes[codePos];
    fileWriter << "\n\t.text\n";
    FunctionAST *func = (FunctionAST *)symtable->SearchTable(code->addresses[0]->address);
    fileWriter << func->name << ":\n";
    codePos++;

    int paramNum = func->parameters.size();
    ClearRegs(paramNum);

    int varNum = 0;
    std::vector<std::string> paramVars;
    std::set<std::string> usedVars;
    for(int i = codePos; i < codes->codes.size(); i++)
    {
        code = codes->codes[i];
        if (code->op == THREE_OP_FUNC_DEF)
        {
            break;
        }
        if (code->op == THREE_OP_PARAM)
        {
            for (auto address : code->addresses)
            {
                if (address->type == THREE_VARIABLE || address->type == THREE_TMP_VAR)
                {
                    if (address->address != "")
                    {
                        paramVars.push_back(address->address);
                    }
                }
            }
        }
        else
        {
            for (auto address : code->addresses)
            {
                if (address->type == THREE_VARIABLE || address->type == THREE_TMP_VAR)
                //if (address->type == THREE_VARIABLE)
                {
                    if (address->address != "" && address->address != "return_value")
                    {
                        usedVars.insert(address->address);
                    }
                }
            }
        }
    }
    //varNum = vars.size();
    std::cout << "function " << func->name << " have used " << usedVars.size() << " vars" << std::endl;
    std::cout << "function " << func->name << " have param " << paramVars.size() << " vars" << std::endl;
    for (auto var : usedVars)
    {
        std::cout << var<< std::endl;
    }

    std::string regs2push = "";
    int paramRegNum = paramVars.size() > 4 ? 4 : paramVars.size();
    //不论如何r0不能push，不然就没返回值了
    for (int i = std::max(paramRegNum, 1); i < registers.size(); i++)
    {
        regs2push += std::string("r" + std::to_string(i)) + ", ";
    }
    if (func->name == "main")
    {
        regs2push = "";
    }
    regs2push += "fp, lr";
    WritePush(regs2push);
    //fileWriter << "\tpush\t{fp, lr}\n";
    WriteAdd("fp", "sp", "#4");

    // 创建内存空间
    if (usedVars.size())
    {
        WriteSub("sp", "sp", "#" + std::to_string(4 * (2 + usedVars.size())));
        int i = 0;
        for (auto var : usedVars)
        {
            int pos;
            for (pos = 0; pos < paramVars.size(); pos++)
            {
                if (paramVars[pos] == var)
                {
                    break;
                }
            }
            //不在参数中
            if (pos == paramVars.size())
            {
                Definition* varDef = symtable->SearchTableDefinition(var);
                varDef->storePlaces.push_back("sp, #" + std::to_string(i * 4));
            }
            //初值在寄存器中
            else if (pos < 4)
            {
                Definition* varDef = symtable->SearchTableDefinition(var);
                varDef->storePlaces.push_back("sp, #" + std::to_string(i * 4));
                WriteStr("r" + std::to_string(pos), varDef->storePlaces[0]);
                //清掉这个寄存器
                SpillReg("r" + std::to_string(pos));
            }
            //初值在栈中
            else
            {
                Definition* varDef = symtable->SearchTableDefinition(var);
                varDef->storePlaces.push_back("fp, #" + std::to_string((pos - 2) * 4));
            }
            i++;
        }
    }

    for(; codePos < codes->codes.size(); codePos++)
    {
        code = codes->codes[codePos];
        std::string regName[3];
        switch(code->op)
        {
            case THREE_OP_ADD:
            {
                for(int i = 0; i < 3; i++)
                {
                    bool flag = 0;
                    if (i == 0)
                        flag = 1;
                    regName[i] = GetRegister(code->addresses[i]->address, flag);
                } 
                WriteAdd(regName[0], regName[1], regName[2]);
            }
            break;
            case THREE_OP_MINUS:
            {
                for(int i = 0; i < 3; i++)
                {
                    bool flag = 0;
                    if (i == 0)
                        flag = 1;
                    regName[i] = GetRegister(code->addresses[i]->address, flag);
                } 
                WriteSub(regName[0], regName[1], regName[2]);
            }
            break;
            case THREE_OP_MUL:
            {
                for(int i = 0; i < 3; i++)
                {
                    bool flag = 0;
                    if (i == 0)
                        flag = 1;
                    regName[i] = GetRegister(code->addresses[i]->address, flag);
                } 
                WriteMul(regName[0], regName[1], regName[2]);
            }
            break;
            case THREE_OP_DIV:
            {
                SpillReg("r0");
                WriteMov("r0", GetRegister(code->addresses[1]->address, 0));
                SpillReg("r1");
                WriteMov("r1", GetRegister(code->addresses[2]->address, 0));
                WriteBl("__aeabi_idivmod");
                std::string returnReg = GetRegister(code->addresses[0]->address, 0);
                WriteMov(returnReg, "r0");
                fprintf(stderr, "div not completed yet\t");
            }
            break;
            case THREE_OP_MOD:
            {
                SpillReg("r0");
                WriteMov("r0", GetRegister(code->addresses[1]->address, 0));
                SpillReg("r1");
                WriteMov("r1", GetRegister(code->addresses[2]->address, 0));
                WriteBl("__aeabi_idivmod");
                std::string returnReg = GetRegister(code->addresses[0]->address, 0);
                WriteMov(returnReg, "r1");
            }
            break;
            case THREE_OP_ASSIGN:
            {
                for(int i = 0; i < 2; i++)
                {
                    bool flag = 0;
                    if (i == 0)
                        flag = 1;
                    regName[i] = GetRegister(code->addresses[i]->address, flag);
                } 
                WriteMov(regName[0], regName[1]);
            }
            break;
            case THREE_OP_ASSIGN_SHIFT:
            {
                for(int i = 0; i < 2; i++)
                {
                    bool flag = 0;
                    if (i == 0)
                        flag = 1;
                    regName[i] = GetRegister(code->addresses[i]->address, flag);
                } 
                WriteMov(regName[0], regName[1], code->addresses[2]->address);
            }
            break;
            case THREE_OP_VAR_DEF:
            {
                for (int i = 0; i < 2; i++)
                {
                    bool flag = 0;
                    if (i == 0)
                        flag = 1;
                    regName[i] = GetRegister(code->addresses[i]->address, flag);
                }
                FindRegByName(regName[0])->Clear();
                WriteMov(regName[0], regName[1]);
                std::string varName = code->addresses[0]->address;
                Definition *varDef = symtable->SearchTableDefinition(varName);
                WriteStr(regName[0], varDef->storePlaces[0]);
                FindRegByName(regName[0])->Clear();
            }
            break;
            case THREE_OP_LABEL:
            {
                WriteLabel(code->addresses[0]->address);
            }
            break;
            case THREE_OP_CMP:
            {
                for(int i = 0; i < 2; i++)
                {
                    regName[i] = GetRegister(code->addresses[i]->address, 0);
                } 
                WriteCmp(regName[0], regName[1]);
            }
            break;
            case THREE_OP_JUMP:
            {
                WriteB(code->addresses[0]->address);
            }
            break;
            case THREE_OP_JUMP_LE:
            {
                WriteBle(code->addresses[0]->address);
            }
            break;
            case THREE_OP_JUMP_LT:
            {
                WriteBlt(code->addresses[0]->address);
            }
            break;
            case THREE_OP_JUMP_GE:
            {
                WriteBge(code->addresses[0]->address);
            }
            break;
            case THREE_OP_JUMP_GT:
            {
                WriteBgt(code->addresses[0]->address);
            }
            break;
            case THREE_OP_JUMP_EQ:
            {
                WriteBeq(code->addresses[0]->address);
            }
            break;
            case THREE_OP_JUMP_NE:
            {
                WriteBne(code->addresses[0]->address);
            }
            break;
            case THREE_OP_PUSH_STACK:
            {
                PushCallParam();

                ///fileWriter << "\tpush\t" << GetRegister(code->addresses[0]->address) << std::endl;
            }
            break;
            case THREE_OP_FUNC_CALL:
            {
                code = codes->codes[codePos];
                WriteBl(code->addresses[0]->address);
                //SpillAllReg();
                //registers->ClearRegs(1);
            }
            break;
            case THREE_OP_RETURN:
            {
                //可能没有返回值
                if (code->addresses.size())
                {
                    WriteMov("r0", GetRegister(code->addresses[0]->address, 0));
                    WriteB("label_" + func->name);
                }
            }
            break;
        }
        if (code->op == THREE_OP_FUNC_DEF)
        {
            break;
        }
    }
    WriteLabel("label_" + func->name);
    WriteSub("sp", "fp", "#4");
    WritePop(regs2push);
    WriteBx("lr");
}

void CodeGener::GenCode()
{
    GenWelcome();
    fileWriter << "\t.global main\n";
    GenGlobalVar();
    while(codePos < codes->codes.size())
    {
        GenFunction();
    }
}

void CodeGener::GenWelcome()
{
    fileWriter << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n";
    fileWriter << "@                              @\n";
    fileWriter << "@  Generated by ning compiler  @\n";
    fileWriter << "@                              @\n";
    fileWriter << "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n\n";
}

void CodeGener::WriteLabel(const std::string &label)
{
    fileWriter << label << ":" << std::endl;
}

void CodeGener::WritePush(const std::string &address)
{
    fileWriter << "\tpush\t" << "{" << address << "}" << std::endl;
}

void CodeGener::WritePop(const std::string &address)
{
    fileWriter << "\tpop\t\t" << "{" << address << "}" << std::endl;
}

void CodeGener::WriteStr(const std::string &address1, const std::string &address2)
{
    fileWriter << "\tstr\t\t" << address1 << ", [" << address2 << "]" << std::endl;
}

void CodeGener::WriteLdr(const std::string &address1, const std::string &address2)
{
    fileWriter << "\tldr\t\t" << address1 << ", " << address2 << std::endl;
}

void CodeGener::WriteMov(const std::string &address1, const std::string &address2)
{
    fileWriter << "\tmov\t\t" << address1 << ", " << address2 << std::endl;
}

void CodeGener::WriteMov(const std::string &address1, const std::string &address2, const std::string &address3)
{
    fileWriter << "\tmov\t\t" << address1 << ", " << address2 << ", " << address3 << std::endl;
}

void CodeGener::WriteCmp(const std::string &address1, const std::string &address2)
{
    fileWriter << "\tcmp\t\t" << address1 << ", " << address2 << std::endl;
}

void CodeGener::WriteAdd(const std::string &address1, const std::string &address2, const std::string &address3)
{
    fileWriter << "\tadd\t\t" << address1 << ", " << address2 << ", " << address3 << std::endl;
}

void CodeGener::WriteSub(const std::string &address1, const std::string &address2, const std::string &address3)
{
    fileWriter << "\tsub\t\t" << address1 << ", " << address2 << ", " << address3 << std::endl;
}

void CodeGener::WriteMul(const std::string &address1, const std::string &address2, const std::string &address3)
{
    fileWriter << "\tmul\t\t" << address1 << ", " << address2 << ", " << address3 << std::endl;
}

void CodeGener::WriteB(const std::string &address)
{
    fileWriter << "\tb\t\t" << address << std::endl;
}

void CodeGener::WriteBl(const std::string &address)
{
    fileWriter << "\tbl\t\t" << address << std::endl;
}

void CodeGener::WriteBx(const std::string &address)
{
    fileWriter << "\tbx\t\t" << address << std::endl;
}

void CodeGener::WriteBeq(const std::string &address)
{
    fileWriter << "\tbeq\t\t" << address << std::endl;
}

void CodeGener::WriteBne(const std::string &address)
{
    fileWriter << "\tbne\t\t" << address << std::endl;
}

void CodeGener::WriteBlt(const std::string &address)
{
    fileWriter << "\tblt\t\t" << address << std::endl;
}

void CodeGener::WriteBle(const std::string &address)
{
    fileWriter << "\tble\t\t" << address << std::endl;
}

void CodeGener::WriteBgt(const std::string &address)
{
    fileWriter << "\tbgt\t\t" << address << std::endl;
}

void CodeGener::WriteBge(const std::string &address)
{
    fileWriter << "\tbge\t\t" << address << std::endl;
}
