#pragma once
#include "../global/global.hpp"
#include "../IR/three_address.hpp"

enum CodeGenerStatus
{
    GENER_GLOBAL_VAR,
    GENER_FUNC,
    GENER_END,
};

class Register
{
public:
    std::vector<std::string> storedVars;
    std::vector<std::string> storedVarsBackup;
    std::string regName;
    int LRU;
    Register(const std::string &regName)
    :regName(regName)
    {
        LRU = 0;
    }
    bool HaveVar(const std::string &varName);
    bool InsertVar(const std::string &varName);
    bool RemoveVar(const std::string &varName);
    void Clear();
};

class CodeGener
{
    std::string outputFilename;
    std::shared_ptr<ThreeAddressCode> codes;
    int codePos;
    std::shared_ptr<SymTable> symtable;
    std::fstream fileWriter;
    std::map<std::string, std::string> address2registers;
    std::set<std::string> usedRegisters;
    std::vector<std::shared_ptr<Register>> registers;
    std::string regs2push;


    void GenWelcome();
    void GenGlobalVar();
    void GenFunction();

public:
    CodeGener(const std::string &outputFilename, std::shared_ptr<ThreeAddressCode> codes, std::shared_ptr<SymTable> symtable);
    void GenCode();
};