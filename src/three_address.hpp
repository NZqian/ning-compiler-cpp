#pragma once
#include "global.hpp"
#include "AST.hpp"
#include "type.hpp"

class ThreeAddress;

//一个地址
class Address
{
    public:
    ThreeAddressType type;
    void *address;
    Address(ThreeAddressType type, void *address);
};

//一条三地址码
class ThreeAddress
{
    ThreeAddress *selfAddress;
    ThreeAddressOp op;
    std::vector<Address> addresses;

    public:
    ThreeAddress(ThreeAddressOp op, std::vector<Address> addresses);
    void Show();
};

class ThreeAddressCode
{
    public:
    std::list<std::shared_ptr<ThreeAddress> > codes;
    ThreeAddressCode();
    void Show();
};