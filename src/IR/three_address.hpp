#pragma once
#include "../global/AST.hpp"
#include "../global/global.hpp"
#include "../global/type.hpp"
#include "../frontend/symtable.hpp"

class ThreeAddress;

//一个地址
class Address
{
public:
    ThreeAddressType type;
    std::string address;
    BaseAST *pointer;
    Address(ThreeAddressType type, std::string address);
    Address(ThreeAddressType type, BaseAST *pointer);
};

//一条三地址码
class ThreeAddress
{
public:
    ThreeAddressOp op;
    std::vector<std::shared_ptr<Address>> addresses;

    ThreeAddress(ThreeAddressOp op, std::vector<std::shared_ptr<Address>> addresses);
    void Show();
    static std::string GenCodeLabel();
    static std::string GenTmpVar();
    static std::shared_ptr<ThreeAddress> MakeLabel();
    static std::shared_ptr<ThreeAddress> MakeLabel(const std::string &);
};

class ThreeAddressCode
{
public:
    std::vector<std::shared_ptr<ThreeAddress>> codes;
    void Show();
};
