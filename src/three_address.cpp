#include "three_address.hpp"

ThreeAddress::ThreeAddress(ThreeAddressOp op, std::vector<Address> addresses)
    : op(op), addresses(addresses)
{
    selfAddress = this;
}

Address::Address(ThreeAddressType type, void *address)
    : type(type), address(address)
{
}

ThreeAddressCode::ThreeAddressCode()
{
}

void Visitor::GenThreeAddress(BaseAST *ast)
{
    if (typeid(*ast) == typeid(ProgAST))
    {
        ProgAST *prog = (ProgAST *)ast;
        for (auto child : prog->children)
            child->Traverse(*this, THREEADDRESS);
    }

    else if (typeid(*ast) == typeid(BlockAST))
    {
        BlockAST *block = (BlockAST *)ast;
        for (auto child : block->children)
            child->Traverse(*this, THREEADDRESS);
    }

    else if (typeid(*ast) == typeid(LiteralAST))
    {
        LiteralAST *literal = (LiteralAST *)ast;
        tmpAddress = literal;
    }

    else if (typeid(*ast) == typeid(VariableAST))
    {
        VariableAST *var = (VariableAST *)ast;
        tmpAddress = var;
        //definition
        if (var->val)
        {
            var->val->Traverse(*this, SHOW);
            //ThreeAddress code = ThreeAddress(THREE_OP_FUNC_DEF, std::vector<Address>{Address(THREE_FUNCTION, func)});
            //threeAddressCode->codes.emplace_back(code);
        }
    }

    else if (typeid(*ast) == typeid(FunctionAST))
    {
        FunctionAST *func = (FunctionAST *)ast;
        // definition
        if (func->body)
        {
            std::shared_ptr<ThreeAddress> code = std::make_shared<ThreeAddress>(THREE_OP_FUNC_DEF, std::vector<Address>{Address(THREE_FUNCTION, func)});
            curCode = code;
            threeAddressCode->codes.emplace_back(code);
            func->body->Traverse(*this, THREEADDRESS);
        }
        // function call
        else
        {
            // TODO shoud use func definition address
            std::shared_ptr<ThreeAddress> code = std::make_shared<ThreeAddress>(THREE_OP_FUNC_CALL, std::vector<Address>{Address(THREE_FUNCTION, func)});
            curCode = code;
            threeAddressCode->codes.emplace_back(code);
        }
    }

    else if (typeid(*ast) == typeid(StmtAST))
    {
        StmtAST *stmt = (StmtAST *)ast;
        for (auto child : stmt->children)
            child->Traverse(*this, THREEADDRESS);
        switch (stmt->type)
        {
        case STMT_IF:
            break;
        case STMT_WHILE:
            break;
        case STMT_CONTINUE:
            break;
        case STMT_BREAK:
            break;
        case STMT_RETURN:
            break;
        case STMT_ASSIGN:
            std::vector<Address> addresses;
            stmt->children[0]->Traverse(*this, THREEADDRESS);
            addresses.emplace_back(Address(THREE_EXPR, tmpAddress));
            stmt->children[1]->Traverse(*this, THREEADDRESS);
            addresses.emplace_back(Address(THREE_EXPR, tmpAddress));

            std::shared_ptr<ThreeAddress> code = std::make_shared<ThreeAddress>(THREE_OP_ASSIGN, addresses);
            threeAddressCode->codes.emplace_back(code);
            tmpAddress = stmt;
            break;
        }
    }

    else if (typeid(*ast) == typeid(ExprAST))
    {
        ExprAST *expr = (ExprAST *)ast;
        std::vector<Address> addresses;
        expr->LHS->Traverse(*this, THREEADDRESS);
        addresses.emplace_back(Address(THREE_EXPR, tmpAddress));
        if (expr->RHS)
        {
            expr->RHS->Traverse(*this, THREEADDRESS);
            addresses.emplace_back(Address(THREE_EXPR, tmpAddress));
        }
        std::shared_ptr<ThreeAddress> code = std::make_shared<ThreeAddress>(strOp2ThreeAddressOp[expr->op], addresses);
        threeAddressCode->codes.emplace_back(code);
        tmpAddress = expr;
    }
}

void ThreeAddress::Show()
{
    std::cout << ThreeAddressOp2Str[op] << ' ';
    if(op == THREE_OP_FUNC_DEF)
    {
        std::cout << ((FunctionAST*)addresses[0].address)->name;
    }
    else if(op == THREE_OP_ASSIGN)
    {
        std::cout << ((VariableAST*)addresses[0].address)->name;
    }
    std::cout<<std::endl;
}

void ThreeAddressCode::Show()
{
    for(auto code : codes)
    {
        code->Show();
    }
}