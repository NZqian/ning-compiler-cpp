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

Registers::Registers()
{
    for (int i = 0; i < 11; i++)
    {
        registers.push_back(std::make_shared<Register>("r" + std::to_string(i)));
    }
}

std::string Registers::FindReg(const std::string &varName)
{
    for (auto reg : registers)
    {
        if (reg->HaveVar(varName))
        {
            return reg->regName;
        }
    }
    return "";
}

std::string Registers::AllocReg(const std::string &varName)
{
    for (auto reg : registers)
    {
        if (reg->storedVars.size() == 0)
        {
            reg->InsertVar(varName);
            return reg->regName;
        }
    }
    return "";
}

void Registers::ClearRegs(int startPos)
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
    registers = std::make_shared<Registers>();
    fileWriter = std::fstream(outputFilename, std::fstream::out);
    codePos = 0;
}

void CodeGener::SpillReg(const std::string &regName)
{
    std::shared_ptr<Register> reg2remove;
    for (auto reg : registers->registers)
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

void CodeGener::SpillAReg()
{
    std::shared_ptr<Register> useLessReg;
    int minUseCnt = INT32_MAX;
    for (auto reg : registers->registers)
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

//flag == 1表示获取等式左边变量的寄存器
//等式左边的的值被修改，应该删除寄存器中保存的其他变量
std::string CodeGener::GetRegister(const std::string &var, bool flag)
{
    if (var == "return_value")
    {
        return "r0";
    }
    
    if (var[0] == '#' || var[0] == 'r')
    {
        return var;
    }

    std::string regName = registers->FindReg(var);
    if (regName != "")
    {
        return regName;
    }
    //当前变量不存在任何一个寄存器中，分配一个。

    regName = registers->AllocReg(var);
    if (regName != "")
    {
        Definition *varDef = symtable->SearchTableDefinition(var);
        WriteLdr(regName, varDef->storePlaces[0]);
        return regName;
    }
    else
    {
        //spill后递归一次
        SpillAReg();
        return GetRegister(var, flag);
    }

    return var;
    /*
    //存在地址->寄存器对应关系
    if (address2registers.find(var) != address2registers.end())
    {
        return address2registers[var];
    }
    else
    {
        std::string registerName;
        for (int i = 7; i < 11; i++)
        {
            //unused register
            registerName = "R" + std::to_string(i);
            if (usedRegisters.count(registerName) == 0)
            {
                usedRegisters.insert(registerName);
                address2registers[var] = registerName;
                return registerName;
            }
        }
    }
    */
}

std::string GetRegister()
{

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
        if (code->op != THREE_OP_VAR_DEF && code->op != THREE_OP_VAR_DECL)
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
        symtable->InsertVarSavePos(var->name, var->name + "_addr");
    }
}

void CodeGener::PushCallParam()
{
    //前四个参数放在r0-r3寄存器中
    for (int i = 0; i < 4; i++)
    {
        if (codes->codes[codePos]->op != THREE_OP_PUSH_STACK)
        {
            return;
        }
        std::string regName = "r" + std::to_string(i);
        //registers->EmptyReg(regName);
        SpillReg(regName);
        WriteMov(regName, codes->codes[codePos]->addresses[0]->address);
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
            SpillReg("r10");
            regName = "r10";
        }
        else
        {
            regName = code->addresses[0]->address;
        }
        WriteMov(regName, code->addresses[0]->address);
        WritePush(regName);
    }
}

void CodeGener::GenFunction()
{
    std::shared_ptr<ThreeAddress> code;
    code = codes->codes[codePos];
    fileWriter << "\n\t.text\n";
    FunctionAST *func = (FunctionAST *)symtable->SearchTable(code->addresses[0]->address);
    fileWriter << func->name << ":\n";
    fileWriter << "\tpush\t{fp, lr}\n";
    fileWriter << "\tadd\t\tfp, sp, #0\n";
    codePos++;

    int paramNum = func->parameters.size();
    registers->ClearRegs(paramNum);

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
                {
                    if (address->address != "")
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
    /*
    for (auto var : vars)
    {
        std::cout << var<< std::endl;
    }
    */

    // 创建内存空间
    if (usedVars.size())
    {
        WriteSub("sp", "sp", "#" + std::to_string(4 * (2 + varNum)));
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
                varDef->storePlaces.push_back("fp, #-" + std::to_string(8 + i * 4));
            }
            //初值在寄存器中
            else if (pos < 4)
            {
                Definition* varDef = symtable->SearchTableDefinition(var);
                varDef->storePlaces.push_back("fp, #-" + std::to_string(8 + i * 4));
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
        }
    }

    //for ()

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
                fileWriter << "\tsub\t" << regName[0] << ", " << regName[1] << ", " << regName[2] << std::endl;
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
                fprintf(stderr, "div not completed yet\t");
                /*
                for(int i = 0; i < 3; i++)
                {
                    regName[i] = GetRegister(code->addresses[i]->address);
                } 
                fileWriter << "\tmul " << regName[0] << ", " << regName[1] << ", " << regName[2] << std::endl;
                */
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
                for(int i = 0; i < 3; i++)
                {
                    bool flag = 0;
                    if (i == 0)
                        flag = 1;
                    regName[i] = GetRegister(code->addresses[i]->address, flag);
                } 
                WriteMov(regName[0], regName[1], regName[2]);
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
                WriteBl(code->addresses[0]->address);
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
                /*
            }
            break;
            case THREE_OP_FUNC_CALL:
            {
                */
                code = codes->codes[codePos];
                WriteBl(code->addresses[0]->address);
            }
            break;
            case THREE_OP_RETURN:
            {
                //可能没有返回值
                if (code->addresses.size())
                {
                    WriteMov("r0", GetRegister(code->addresses[0]->address, 0));
                }
            }
            break;
        }
        if (code->op == THREE_OP_FUNC_DEF)
        {
            break;
        }
    }
    fileWriter << "\tsub\t\tsp, fp, #0\n";
    fileWriter << "\tpop\t\t{fp, pc}\n";
    fileWriter << "\t.ltorg\n";
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

void CodeGener::WriteStr(const std::string &address1, const std::string &address2)
{
    fileWriter << "\tstr\t\t" << address1 << ", [" << address2 << "]" << std::endl;
}

void CodeGener::WriteLdr(const std::string &address1, const std::string &address2)
{
    fileWriter << "\tldr\t\t" << address1 << ", [" << address2 << "]" << std::endl;
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

void CodeGener::WriteBl(const std::string &address)
{
    fileWriter << "\tbl\t\t" << address << std::endl;
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
