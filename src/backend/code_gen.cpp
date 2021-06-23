#include "code_gen.hpp"

CodeGener::CodeGener(const std::string &outputFilename, std::shared_ptr<ThreeAddressCode> codes, std::shared_ptr<SymTable> symtable)
    : outputFilename(outputFilename), codes(codes), symtable(symtable)
{
    fileWriter = std::fstream(outputFilename, std::fstream::out);
    codePos = 0;
}

std::string CodeGener::GetRegister(const std::string &var)
{
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
                    regName[i] = GetRegister(code->addresses[i]->address);
                } 
                fileWriter << "\tadd\t\t" << regName[0] << ", " << regName[1] << ", " << regName[2] << std::endl;
            }
            break;
            case THREE_OP_MINUS:
            {
                for(int i = 0; i < 3; i++)
                {
                    regName[i] = GetRegister(code->addresses[i]->address);
                } 
                fileWriter << "\tsub\t" << regName[0] << ", " << regName[1] << ", " << regName[2] << std::endl;
            }
            break;
            case THREE_OP_MUL:
            {
                for(int i = 0; i < 3; i++)
                {
                    regName[i] = GetRegister(code->addresses[i]->address);
                } 
                fileWriter << "\tmul\t" << regName[0] << ", " << regName[1] << ", " << regName[2] << std::endl;
            }
            break;
            case THREE_OP_DIV:
            {
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
                    regName[i] = GetRegister(code->addresses[i]->address);
                } 
                fileWriter << "\tmov\t\t" << regName[0] << ", " << regName[1] << std::endl;
            }
            break;
            case THREE_OP_LABEL:
            {
                fileWriter << code->addresses[0]->address << ":" << std::endl;
            }
            break;
            case THREE_OP_JUMP:
            {
                fileWriter << "\tbl\t\t" << code->addresses[0]->address << std::endl;
            }
            break;
            case THREE_OP_CMP:
            {
                for(int i = 0; i < 2; i++)
                {
                    regName[i] = GetRegister(code->addresses[i]->address);
                } 
                fileWriter << "\tcmp\t\t" << code->addresses[0]->address << ", " << code->addresses[1]->address << std::endl;
            }
            break;
            case THREE_OP_JUMP_LE:
            {
                fileWriter << "\tble\t\t" << code->addresses[0]->address << std::endl;
            }
            break;
            case THREE_OP_JUMP_LT:
            {
                fileWriter << "\tblt\t\t" << code->addresses[0]->address << std::endl;
            }
            break;
            case THREE_OP_JUMP_GE:
            {
                fileWriter << "\tbge\t\t" << code->addresses[0]->address << std::endl;
            }
            break;
            case THREE_OP_JUMP_GT:
            {
                fileWriter << "\tbgt\t\t" << code->addresses[0]->address << std::endl;
            }
            break;
            case THREE_OP_JUMP_EQ:
            {
                fileWriter << "\tbeq\t\t" << code->addresses[0]->address << std::endl;
            }
            break;
            case THREE_OP_JUMP_NE:
            {
                fileWriter << "\tbne\t\t" << code->addresses[0]->address << std::endl;
            }
            break;
            case THREE_OP_FUNC_CALL:
            {
                fileWriter << "\tbl\t\t" << code->addresses[0]->address << std::endl;
            }
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