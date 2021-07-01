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

    int arrayShift;

    void WriteOpOne(const std::string &address);
    void WriteOpTwo(const std::string &address);
    void WriteOpThree(const std::string &address);

    std::string GetRegister(const std::string &var, bool flag);
    std::string GetRegister();
    std::shared_ptr<Register> FindRegByName(const std::string &regName);
    std::shared_ptr<Register> FindRegByVar(const std::string &varName);
    void SpillReg(const std::string &regName);
    std::string SpillAReg();
    void SpillAllReg();
    std::string AllocReg(const std::string &varName);
    void EmptyReg(const std::string &reg);
    void ClearRegs(int startPos);
    void UpdateRegLRU();
    int AvailableRegNum();

    void PushCallParam();
    void ProcessVarAddress();
    void InitGlobalArray(BaseAST *arr);
    void InitLocalArray(BaseAST *arr, int baseAddr);
    void AllocLocalVariable(FunctionAST *func);
    void LoadArrayItem(const std::string &item, const std::string &regName);
    void LoadArrayItemAddress(const std::string &item, const std::string &regName);
    std::vector<std::string> SplitArrayItem(const std::string &item);

    void GenWelcome();
    void GenGlobalVar();
    void GenFunction();

public:
    CodeGener(const std::string &outputFilename, std::shared_ptr<ThreeAddressCode> codes, std::shared_ptr<SymTable> symtable);
    void GenCode();
    void WriteLabel(const std::string &label);
    void WritePush(const std::string &address);
    void WritePop(const std::string &address);
    void WriteStr(const std::string &address1, const std::string &address2);
    void WriteLdr(const std::string &address1, const std::string &address2);
    void WriteMov(const std::string &address1, const std::string &address2);
    void WriteMov(const std::string &address1, const std::string &address2, const std::string &address3);
    void WriteCmp(const std::string &address1, const std::string &address2);
    void WriteAdd(const std::string &address1, const std::string &address2, const std::string &address3);
    void WriteSub(const std::string &address1, const std::string &address2, const std::string &address3);
    void WriteMul(const std::string &address1, const std::string &address2, const std::string &address3);
    void WriteB(const std::string &address);
    void WriteBl(const std::string &address);
    void WriteBx(const std::string &address);
    void WriteBeq(const std::string &address);
    void WriteBne(const std::string &address);
    void WriteBlt(const std::string &address);
    void WriteBle(const std::string &address);
    void WriteBgt(const std::string &address);
    void WriteBge(const std::string &address);
};