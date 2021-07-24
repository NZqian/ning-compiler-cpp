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
    LRU = 0;
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

void CodeGener::GenGlobalVar()
{

}

void CodeGener::GenFunction()
{
}
void CodeGener::GenCode()
{
    GenWelcome();
    fileWriter << "\t.global main\n";
    GenGlobalVar();
}

void CodeGener::GenWelcome()
{
    fileWriter << "################################\n";
    fileWriter << "#                              #\n";
    fileWriter << "#  Generated by ning compiler  #\n";
    fileWriter << "#                              #\n";
    fileWriter << "################################\n\n";
}