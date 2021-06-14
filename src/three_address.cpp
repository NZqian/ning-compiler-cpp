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
        symtable->AddTable();
        for (auto child : prog->children)
            child->Traverse(this, THREEADDRESS);
        symtable->DeleteTable();
    }

    else if (typeid(*ast) == typeid(BlockAST))
    {
        BlockAST *block = (BlockAST *)ast;
        symtable->AddTable();
        for (auto child : block->children)
            child->Traverse(this, THREEADDRESS);
        symtable->DeleteTable();
    }

    else if (typeid(*ast) == typeid(LiteralAST))
    {
        LiteralAST *literal = (LiteralAST *)ast;
        //std::shared_ptr<ThreeAddress> code = std::make_shared<ThreeAddress>(THREE_OP_ASSIGN, );
        //threeAddressCode->codes.emplace_back(code);
        //tmpAddress = code->selfAddress;
        tmpAddress = literal;
    }

    else if (typeid(*ast) == typeid(VariableAST))
    {
        VariableAST *var = (VariableAST *)ast;
        tmpAddress = var;
        std::vector<Address> addresses;
        addresses.emplace_back(Address(THREE_VARIABLE, tmpAddress));

        if(var->type == INT)
        {
            if(!symtable->Insert(var, var->name, VARIABLE))
            {
                fprintf(stderr, "redifinition! %s\n", var->name.c_str());
                exit(-1);
            }

            ThreeAddressOp op;
            if (var->val)
            {
                var->val->Traverse(this, THREEADDRESS);
                if (var->val->TypeName() == typeid(LiteralAST).name())
                    addresses.emplace_back(Address(THREE_LITERAL, tmpAddress));
                else
                    addresses.emplace_back(Address(THREE_TMP_VAR, tmpAddress));
                op = THREE_OP_VAR_DEF;
            }
            else
            {
                op = THREE_OP_VAR_DECL;
            }
            std::shared_ptr<ThreeAddress> code = std::make_shared<ThreeAddress>(op, addresses);
            threeAddressCode->codes.emplace_back(code);
        }
        else if(var->type == NONE)
        {
            if(!symtable->SearchTable(var->name, VARIABLE))
            {
                fprintf(stderr, "undefined variable %s\n", var->name.c_str());
                exit(-1);
            }
        }
    }

    else if (typeid(*ast) == typeid(FunctionAST))
    {
        FunctionAST *func = (FunctionAST *)ast;

        //type check
        {
            if(func->returnType == INT)
            {
                if(!symtable->Insert(func, func->name, FUNCTION))
                {
                    fprintf(stderr, "redifinition! %s\n", func->name.c_str());
                }
            }
            else if(func->returnType == NONE)
            {
                if(!symtable->SearchTable(func->name, FUNCTION))
                {
                    fprintf(stderr, "use before definition! %s\n", func->name.c_str());
                }
            }
        }

        // definition
        if (func->body)
        {
            std::shared_ptr<ThreeAddress> code = std::make_shared<ThreeAddress>(THREE_OP_FUNC_DEF, std::vector<Address>{Address(THREE_FUNCTION, func)});
            curCode = code;
            threeAddressCode->codes.emplace_back(code);
            func->body->Traverse(this, THREEADDRESS);
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
            child->Traverse(this, THREEADDRESS);
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
            stmt->children[0]->Traverse(this, THREEADDRESS);
            VariableAST *var;
            var = (VariableAST*)symtable->SearchTable(std::dynamic_pointer_cast<VariableAST>(stmt->children[0])->name, VARIABLE);
            addresses.emplace_back(Address(THREE_VARIABLE, var));
            stmt->children[1]->Traverse(this, THREEADDRESS);

            ThreeAddressType pointerType;
            if (stmt->children[1]->TypeName() == typeid(VariableAST).name())
            {
                pointerType = THREE_VARIABLE;
            }
            else if(stmt->children[1]->TypeName() == typeid(LiteralAST).name())
            {
                pointerType = THREE_LITERAL;
            }
            else if(stmt->children[1]->TypeName() == typeid(ExprAST).name())
            {
                pointerType = THREE_TMP_VAR;
            }

            addresses.emplace_back(Address(pointerType, tmpAddress));

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
        expr->LHS->Traverse(this, THREEADDRESS);
        //std::cout << expr->LHS->TypeName() << std::endl;
        ThreeAddressType pointerType;
        if (expr->LHS->TypeName() == typeid(VariableAST).name())
        {
            pointerType = THREE_VARIABLE;
        }
        else if(expr->LHS->TypeName() == typeid(LiteralAST).name())
        {
            pointerType = THREE_LITERAL;
        }
        else if(expr->LHS->TypeName() == typeid(ExprAST).name())
        {
            pointerType = THREE_TMP_VAR;
        }
        addresses.emplace_back(Address(pointerType, tmpAddress));
        if (expr->RHS)
        {
            expr->RHS->Traverse(this, THREEADDRESS);
            if (expr->RHS->TypeName() == typeid(VariableAST).name())
            {
                pointerType = THREE_VARIABLE;
            }
            else if(expr->RHS->TypeName() == typeid(LiteralAST).name())
            {
                pointerType = THREE_LITERAL;
            }
            else if(expr->RHS->TypeName() == typeid(ExprAST).name())
            {
                pointerType = THREE_TMP_VAR;
            }
            addresses.emplace_back(Address(pointerType, tmpAddress));
        }
        std::shared_ptr<ThreeAddress> code = std::make_shared<ThreeAddress>(strOp2ThreeAddressOp[expr->op], addresses);
        threeAddressCode->codes.emplace_back(code);
        tmpAddress = code->selfAddress;

        //type check
        {
            if (expr->LHS->IsLiteral())
            {
                if(expr->RHS)
                {
                    if(expr->RHS->IsLiteral())
                    {
                        expr->is_literal = true;
                    }
                    else
                    {
                        expr->is_literal = false;
                    }
                }
                else
                {
                    expr->is_literal = true;
                }
            }
            else
            {
                expr->is_literal = false;
            }
        }
    }
}

void ThreeAddress::Show()
{
    std::cout << selfAddress << ", " << ThreeAddressOp2Str[op] << ", ";

    if(op == THREE_OP_FUNC_DEF)
    {
        std::cout << ((FunctionAST*)addresses[0].address)->name;
    }

    else if(op == THREE_OP_VAR_DEF)
    {
        std::cout << ((VariableAST*)addresses[0].address)->name << ", ";
        if(addresses[1].type == THREE_LITERAL)
            std::cout << ((LiteralAST*)addresses[1].address)->val;
        else
            std::cout << addresses[1].address;
    }

    else if(op == THREE_OP_VAR_DECL)
    {
        std::cout << ((VariableAST*)addresses[0].address)->name << ", " << addresses[0].address;
    }

    else if(op == THREE_OP_ASSIGN)
    {
        std::cout << ((VariableAST*)addresses[0].address)->name << "(" << addresses[0].address << "), ";
        if(addresses[1].type == THREE_VARIABLE)
            std::cout << ((VariableAST*)addresses[1].address)->name;
        else if(addresses[1].type == THREE_LITERAL)
            std::cout << ((LiteralAST*)addresses[1].address)->val;
        else
            std::cout << addresses[1].address;
    }

    else if(op == THREE_OP_MUL || op == THREE_OP_DIV || op == THREE_OP_ADD || op == THREE_OP_MINUS)
    {
        for (int i = 0; i < 2; i++)
        {
            if(addresses[i].type == THREE_VARIABLE)
                std::cout << ((VariableAST*)addresses[i].address)->name << "(" << addresses[i].address << ")";
            else if(addresses[i].type == THREE_LITERAL)
                std::cout << ((LiteralAST*)addresses[i].address)->val;
            else
                std::cout << addresses[i].address;
            if(i == 0)
                std::cout << ", ";
        }
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