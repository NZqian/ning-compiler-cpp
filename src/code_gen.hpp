#pragma once
#include "global.hpp"
#include "three_address.hpp"

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
    std::fstream fileWriter;
    CodeGenerStatus status;
    std::map<void *, std::string> address2registers;
    std::set<std::string> usedRegisters;

    std::string GetRegister(void *address);
    void GenWelcome();
public:
    CodeGener(const std::string &outputFilename, std::shared_ptr<ThreeAddressCode> codes);
    void GenCode();
};