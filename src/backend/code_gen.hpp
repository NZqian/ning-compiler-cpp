#pragma once
#include "../global/global.hpp"
#include "../IR/three_address.hpp"

enum CodeGenerStatus
{
    GENER_GLOBAL_VAR,
    GENER_FUNC,
    GENER_END,
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

    std::string GetRegister(const std::string &var);
    void GenWelcome();
	void GenGlobalVar();
	void GenFunction();
public:
    CodeGener(const std::string &outputFilename, std::shared_ptr<ThreeAddressCode> codes, std::shared_ptr<SymTable> symtable);
    void GenCode();
};