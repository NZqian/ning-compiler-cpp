#include "three_address.hpp"


ThreeAddress::ThreeAddress(ThreeAddressOp op, std::vector<std::shared_ptr<Address>> addresses)
    : op(op), addresses(addresses)
{
    label = GenLabel();
}

std::string ThreeAddress::GenLabel()
{
    static int cnt = 0;
    std::string label = "$" + std::to_string(cnt); 
    cnt++;
    return label;
}

std::string ThreeAddress::GenCodeLabel()
{
    static int cnt = 0;
    std::string label = "label_" + std::to_string(cnt); 
    cnt++;
    return label;
}

std::shared_ptr<ThreeAddress> ThreeAddress::MakeLabel()
{
    std::string codeLabel = GenCodeLabel();
    return std::make_shared<ThreeAddress>(THREE_OP_LABEL, std::vector<std::shared_ptr<Address>>{std::make_shared<Address>(THREE_LABEL, codeLabel)});
}

std::shared_ptr<ThreeAddress> ThreeAddress::MakeLabel(const std::string &codeLabel)
{
    return std::make_shared<ThreeAddress>(THREE_OP_LABEL, std::vector<std::shared_ptr<Address>>{std::make_shared<Address>(THREE_LABEL, codeLabel)});
}

std::string ThreeAddress::GetNextLabel(const std::string &label)
{
    int cnt = std::atoi(std::string(label.begin() + 1, label.end()).c_str());
    return "$" + std::to_string(cnt + 1);
}

std::string ThreeAddress::GenTmpVar()
{
    static int cnt = 0;
    std::string var = "@" + std::to_string(cnt); 
    cnt++;
    return var;
}

Address::Address(ThreeAddressType type, std::string address)
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
            child->Traverse(this, THREEADDRESS);
    }

    else if (typeid(*ast) == typeid(BlockAST))
    {
        BlockAST *block = (BlockAST *)ast;
        for (auto child : block->children)
            child->Traverse(this, THREEADDRESS);
    }

    else if (typeid(*ast) == typeid(LiteralAST))
    {
        LiteralAST *literal = (LiteralAST *)ast;
        //std::shared_ptr<ThreeAddress> code = std::make_shared<ThreeAddress>(THREE_OP_ASSIGN, );
        //threeAddressCode->codes.emplace_back(code);
        //tmpAddress = code->selfAddress;
        tmpLabel = "#" + std::to_string(literal->val);
    }


    else if (typeid(*ast) == typeid(VariableAST))
    {
        VariableAST *var = (VariableAST *)ast;
        std::vector<std::shared_ptr<Address>> addresses;
        addresses.emplace_back(std::make_shared<Address>(THREE_VARIABLE, var->name));

        if(var->type == INT)
        {
            ThreeAddressOp op;
            if (var->val)
            {
                var->val->Traverse(this, THREEADDRESS);
                if (var->val->TypeName() == typeid(LiteralAST).name())
                    addresses.emplace_back(std::make_shared<Address>(THREE_LITERAL, tmpLabel));
                else
                    addresses.emplace_back(std::make_shared<Address>(THREE_TMP_VAR, tmpLabel));
                if (var->isConst)
                    op = THREE_OP_CONST_VAR_DEF;
                else
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
        }
    }

    else if (typeid(*ast) == typeid(FunctionAST))
    {
        FunctionAST *func = (FunctionAST *)ast;

        // definition
        if (func->body)
        {
            std::shared_ptr<ThreeAddress> code = std::make_shared<ThreeAddress>(THREE_OP_FUNC_DEF, std::vector<std::shared_ptr<Address>>{std::make_shared<Address>(THREE_FUNCTION, func->name)});
            curCode = code;
            threeAddressCode->codes.emplace_back(code);
            func->body->Traverse(this, THREEADDRESS);
        }
        // function call
        else
        {
            // TODO shoud use func definition address
            std::shared_ptr<ThreeAddress> code = std::make_shared<ThreeAddress>(THREE_OP_FUNC_CALL, std::vector<std::shared_ptr<Address>>{std::make_shared<Address>(THREE_FUNCTION, func->name)});
            curCode = code;
            threeAddressCode->codes.emplace_back(code);
            tmpLabel = "return_value";
        }
    }

    else if (typeid(*ast) == typeid(StmtAST))
    {
        StmtAST *stmt = (StmtAST *)ast;
        std::vector<std::shared_ptr<Address>> addresses{};
        std::shared_ptr<ThreeAddress> code;
        std::string labelPlaceHolder = "unknown";
        /*
        for (auto child : stmt->children)
            child->Traverse(this, THREEADDRESS);
        */
        switch (stmt->type)
        {
        case STMT_IF:
        {
            ExprAST *expr = (ExprAST*)stmt->children[0].get();
            ThreeAddressOp threeOp;
            ThreeAddressOp jumpOp;
            //先生成两个label
            label[0].push_back(ThreeAddress::GenCodeLabel());
            label[1].push_back(ThreeAddress::GenCodeLabel());
            threeOp = strOp2ThreeAddressOp[expr->op];
            //process condition
            stmt->children[0]->Traverse(this, THREEADDRESS);
            //将最顶部表达式修改为条件表达式
            threeAddressCode->codes.back()->op = THREE_OP_CMP;
            //should jump to then
            jumpOp = ExprOp2JumpOp[threeOp];
            threeAddressCode->codes.push_back(std::make_shared<ThreeAddress>(jumpOp, std::vector<std::shared_ptr<Address>>{std::make_shared<Address>(THREE_LABEL, label[1].back())}));

            //have else
            if (stmt->children.size() == 3)
            {
                stmt->children[2]->Traverse(this, THREEADDRESS);
            }
            //should jump to after then
            threeAddressCode->codes.push_back(std::make_shared<ThreeAddress>(THREE_OP_JUMP, std::vector<std::shared_ptr<Address>>{std::make_shared<Address>(THREE_LABEL, label[0].back())}));

            threeAddressCode->codes.push_back(ThreeAddress::MakeLabel(label[1].back()));

            //process then
            stmt->children[1]->Traverse(this, THREEADDRESS);
            threeAddressCode->codes.push_back(ThreeAddress::MakeLabel(label[0].back()));
            for(int i = 0; i < 2; i++)
                label[i].pop_back();
            break;
        }
        case STMT_WHILE:
        {
            ExprAST *expr = (ExprAST*)stmt->children[0].get();
            //先生成两个label
            label[0].push_back(ThreeAddress::GenCodeLabel());
            label[1].push_back(ThreeAddress::GenCodeLabel());

            ThreeAddressOp threeOp;
            ThreeAddressOp jumpOp;
            threeOp = strOp2ThreeAddressOp[expr->op];
            //反转判断条件，不符合跳转
            jumpOp = ExprOp2JumpOpReversed[threeOp];
            //process condition
            stmt->children[0]->Traverse(this, THREEADDRESS);
            //将最顶部表达式修改为条件表达式
            threeAddressCode->codes.back()->op = THREE_OP_CMP;
            auto iter = threeAddressCode->codes.end();
            iter--;
            //判断条件后循环开始前插入一个label
            threeAddressCode->codes.insert(iter, ThreeAddress::MakeLabel(label[0].back()));

            threeAddressCode->codes.push_back(std::make_shared<ThreeAddress>(jumpOp, std::vector<std::shared_ptr<Address>>{std::make_shared<Address>(THREE_LABEL, label[1].back())}));

            stmt->children[1]->Traverse(this, THREEADDRESS);

            //jump to start
            threeAddressCode->codes.push_back(std::make_shared<ThreeAddress>(THREE_OP_JUMP, std::vector<std::shared_ptr<Address>>{std::make_shared<Address>(THREE_LABEL, label[0].back())}));
            //end label
            threeAddressCode->codes.push_back(ThreeAddress::MakeLabel(label[1].back()));

            for(int i = 0; i < 2; i++)
                label[i].pop_back();
            break;
        }
        case STMT_CONTINUE:
        {
            threeAddressCode->codes.push_back(std::make_shared<ThreeAddress>(THREE_OP_JUMP, std::vector<std::shared_ptr<Address>>{std::make_shared<Address>(THREE_LABEL, label[0].back())}));
            break;
        }
        case STMT_BREAK:
        {
            threeAddressCode->codes.push_back(std::make_shared<ThreeAddress>(THREE_OP_JUMP, std::vector<std::shared_ptr<Address>>{std::make_shared<Address>(THREE_LABEL, label[1].back())}));
            break;
        }
        case STMT_RETURN:
        {
            if (stmt->children.size())
            {
                stmt->children[0]->Traverse(this, THREEADDRESS);
                ThreeAddressType type;
                std::string label;
                if(stmt->children[0]->TypeName() == typeid(VariableAST).name())
                {
                    type = THREE_VARIABLE;
                    label = ((VariableAST*)stmt->children[0].get())->name;
                }
                else if(stmt->children[0]->TypeName() == typeid(ExprAST).name())
                {
                    type = THREE_TMP_VAR;
                    label = tmpLabel;
                }
                else if(stmt->children[0]->TypeName() == typeid(LiteralAST).name())
                {
                    type = THREE_LITERAL;
                    label = "#" + std::to_string(((LiteralAST*)stmt->children[0].get())->val);
                }
                addresses.push_back(std::make_shared<Address>(type, label));
                /*
                if (type == THREE_TMP_VAR)
                {
                    symtable->Insert(nullptr, label, )
                }
                */
            }
            code = std::make_shared<ThreeAddress>(THREE_OP_RETURN, addresses);
            threeAddressCode->codes.emplace_back(code);
            tmpLabel = code->label;
            break;
        }
        case STMT_ASSIGN:
        {
            stmt->children[0]->Traverse(this, THREEADDRESS);
            VariableAST *var;
            var = (VariableAST*)symtable->SearchTable(std::dynamic_pointer_cast<VariableAST>(stmt->children[0])->name);
            addresses.emplace_back(std::make_shared<Address>(THREE_VARIABLE, var->name));
            stmt->children[1]->Traverse(this, THREEADDRESS);

            ThreeAddressType addressType;
            std::string label;
            if (stmt->children[1]->TypeName() == typeid(VariableAST).name())
            {
                addressType = THREE_VARIABLE;
                label = ((VariableAST*)stmt->children[1].get())->name;
            }
            else if(stmt->children[1]->TypeName() == typeid(LiteralAST).name())
            {
                addressType = THREE_LITERAL;
                label = "#" + std::to_string(((LiteralAST*)stmt->children[1].get())->val);
            }
            else if(stmt->children[1]->TypeName() == typeid(ExprAST).name())
            {
                addressType = THREE_TMP_VAR;
                label = tmpLabel;
            }
            else if(stmt->children[1]->TypeName() == typeid(FunctionAST).name())
            {
                addressType = THREE_TMP_VAR;
                label = tmpLabel;
            }
            addresses.emplace_back(std::make_shared<Address>(addressType, label));

            code = std::make_shared<ThreeAddress>(THREE_OP_ASSIGN, addresses);
            threeAddressCode->codes.emplace_back(code);
            tmpLabel = code->label;
            break;
        }
        default:
            fprintf(stderr, "this should not happen\n");
        }
            
    }

    else if (typeid(*ast) == typeid(ExprAST))
    {
        ExprAST *expr = (ExprAST *)ast;
        std::vector<std::shared_ptr<Address>> addresses;
        std::string tmpVar;
        expr->LHS->Traverse(this, THREEADDRESS);
        //std::cout << expr->LHS->TypeName() << std::endl;
        ThreeAddressType addressType;
        std::string label;
        if (expr->LHS->TypeName() == typeid(VariableAST).name())
        {
            addressType = THREE_VARIABLE;
            label = ((VariableAST*)expr->LHS.get())->name;
        }
        else if(expr->LHS->TypeName() == typeid(LiteralAST).name())
        {
            addressType = THREE_LITERAL;
            label = "#" + std::to_string(((LiteralAST*)expr->LHS.get())->val);
        }
        else if(expr->LHS->TypeName() == typeid(ExprAST).name())
        {
            addressType = THREE_TMP_VAR;
            label = tmpLabel;
        }
        addresses.emplace_back(std::make_shared<Address>(addressType, label));
        if (expr->RHS)
        {
            expr->RHS->Traverse(this, THREEADDRESS);
            if (expr->RHS->TypeName() == typeid(VariableAST).name())
            {
                addressType = THREE_VARIABLE;
                label = ((VariableAST*)expr->RHS.get())->name;
            }
            else if(expr->RHS->TypeName() == typeid(LiteralAST).name())
            {
                addressType = THREE_LITERAL;
                label = "#" + std::to_string(((LiteralAST*)expr->RHS.get())->val);
            }
            else if(expr->RHS->TypeName() == typeid(ExprAST).name())
            {
                addressType = THREE_TMP_VAR;
                label = tmpLabel;
            }
            addresses.emplace_back(std::make_shared<Address>(addressType, label));
        }
        tmpVar = ThreeAddress::GenTmpVar();
        //symtable->Insert(nullptr, tmpVar, TMPVAR);
        addresses.insert(addresses.begin(), std::make_shared<Address>(THREE_TMP_VAR, tmpVar));
        std::shared_ptr<ThreeAddress> code = std::make_shared<ThreeAddress>(strOp2ThreeAddressOp[expr->op], addresses);
        threeAddressCode->codes.emplace_back(code);
        //tmpLabel = code->label;
        tmpLabel = tmpVar;
    }
}

void ThreeAddress::Show()
{
    std::cout << label << ", " << ThreeAddressOp2Str[op] << ", ";
    for (int i = 0; i < addresses.size(); i++)
    {
        std::cout << addresses[i]->address << ", ";
    }
/*
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
*/
    std::cout<<std::endl;
}

void ThreeAddressCode::Show()
{
    for(auto code : codes)
    {
        code->Show();
    }
}