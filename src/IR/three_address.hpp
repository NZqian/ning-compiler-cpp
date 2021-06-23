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
    Address(ThreeAddressType type, std::string address);
};

//一条三地址码
class ThreeAddress
{
public:
    ThreeAddressOp op;
    std::vector<std::shared_ptr<Address>> addresses;

    std::string label;
    ThreeAddress(ThreeAddressOp op, std::vector<std::shared_ptr<Address>> addresses);
    void Show();
    static std::string GenLabel();
    static std::string GenCodeLabel();
    static std::string GetNextLabel(const std::string &label);
    static std::string GenTmpVar();
    static std::shared_ptr<ThreeAddress> MakeLabel();
    static std::shared_ptr<ThreeAddress> MakeLabel(const std::string &);
};

class ThreeAddressCode
{
public:
    //std::list<std::shared_ptr<ThreeAddress>> codes;
    std::vector<std::shared_ptr<ThreeAddress>> codes;
    ThreeAddressCode();
    void Show();
};
