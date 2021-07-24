#include "three_address.hpp"

ThreeAddress::ThreeAddress(ThreeAddressOp op, std::vector<std::shared_ptr<Address>> addresses)
    : op(op), addresses(addresses)
{
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

std::string ThreeAddress::GenTmpVar()
{
    static int cnt = 0;
    std::string var = "@" + std::to_string(cnt);
    cnt++;
    return var;
}

Address::Address(ThreeAddressType type, std::string address)
    : type(type), address(address){}

Address::Address(ThreeAddressType type, BaseAST *pointer)
    : type(type), pointer(pointer){}

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
        tmpLabel = "#" + std::to_string(literal->val);
        tmpType = THREE_LITERAL;
    }

    else if (typeid(*ast) == typeid(VariableAST))
    {
        VariableAST *var = (VariableAST *)ast;
        std::string varName = var->name;
        std::vector<std::shared_ptr<Address>> addresses;
        addresses.emplace_back(std::make_shared<Address>(THREE_VARIABLE, var->name));

        if (var->type == INT)
        {
            ThreeAddressOp op;
            if (var->val)
            {
                if (var->dimensions.size())
                {
                    if (var->isConst)
                        op = THREE_OP_CONST_ARRAY_DEF;
                    else
                        op = THREE_OP_ARRAY_DEF;
                }
                else
                {
                    var->val->Traverse(this, THREEADDRESS);
                    if (var->val->TypeName() == typeid(LiteralAST).name())
                        addresses.emplace_back(std::make_shared<Address>(THREE_LITERAL, tmpLabel));
                    else
                        addresses.emplace_back(std::make_shared<Address>(THREE_VARIABLE, tmpLabel));
                    if (var->isConst)
                        op = THREE_OP_CONST_VAR_DEF;
                    else
                        op = THREE_OP_VAR_DEF;
                }
            }
            else
            {
                op = THREE_OP_VAR_DECL;
            }
            std::shared_ptr<ThreeAddress> code = std::make_shared<ThreeAddress>(op, addresses);
            threeAddressCode->codes.emplace_back(code);
        }
        else if (var->type == NONE)
        {
            if (var->dimensions.size())
            {
                varName = var->name;
                for (auto dimension : var->dimensions)
                {
                    varName += "$";
                    if (dimension->TypeName() == typeid(LiteralAST).name())
                    {
                        varName += std::to_string(dimension->GetVal());
                    }
                    else if (dimension->TypeName() == typeid(VariableAST).name())
                    {
                        varName += ((VariableAST *)dimension.get())->name;
                    }
                    else
                    {
                        dimension->Traverse(this, THREEADDRESS);
                        varName += tmpLabel;
                    }
                }
                tmpLabel = varName;
            }
            tmpType = THREE_VARIABLE;
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
            for (auto param : func->parameters)
            {
                std::string varName = ((VariableAST *)param.get())->name;
                code = std::make_shared<ThreeAddress>(THREE_OP_PARAM, std::vector<std::shared_ptr<Address>>{std::make_shared<Address>(THREE_VARIABLE, varName)});
                threeAddressCode->codes.emplace_back(code);
            }
            func->body->Traverse(this, THREEADDRESS);
        }
        // function call
        else
        {
            for (auto param : func->parameters)
            {
                std::shared_ptr<ThreeAddress> code;
                ThreeAddressType addressType;
                param->Traverse(this, THREEADDRESS);
                switch (tmpLabel[0])
                {
                case '@':
                    addressType = THREE_TMP_VAR;
                    break;
                case '#':
                    addressType = THREE_LITERAL;
                    break;
                default:
                    addressType = THREE_VARIABLE;
                    break;
                }
                code = std::make_shared<ThreeAddress>(THREE_OP_PUSH_STACK, std::vector<std::shared_ptr<Address>>{std::make_shared<Address>(addressType, tmpLabel)});
                threeAddressCode->codes.emplace_back(code);
            }

            std::shared_ptr<ThreeAddress> code;
            code = std::make_shared<ThreeAddress>(THREE_OP_FUNC_CALL, std::vector<std::shared_ptr<Address>>{std::make_shared<Address>(THREE_FUNCTION, func->name)});
            curCode = code;
            threeAddressCode->codes.emplace_back(code);
            std::string tmpVar = ThreeAddress::GenTmpVar();
            code = std::make_shared<ThreeAddress>(THREE_OP_ASSIGN, std::vector<std::shared_ptr<Address>>{std::make_shared<Address>(THREE_TMP_VAR, tmpVar), std::make_shared<Address>(THREE_TMP_VAR, "return_value")});
            threeAddressCode->codes.emplace_back(code);
            tmpLabel = tmpVar;
        }
        tmpType = THREE_TMP_VAR;
    }

    else if (typeid(*ast) == typeid(StmtAST))
    {
        StmtAST *stmt = (StmtAST *)ast;
        std::vector<std::shared_ptr<Address>> addresses{};
        std::shared_ptr<ThreeAddress> code;
        std::string labelPlaceHolder = "unknown";
        switch (stmt->type)
        {
        case STMT_IF:
        {
            ExprAST *expr = (ExprAST *)stmt->children[0].get();
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
            threeAddressCode->codes.back()->addresses.erase(threeAddressCode->codes.back()->addresses.begin());
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
            for (int i = 0; i < 2; i++)
                label[i].pop_back();
            break;
        }
        case STMT_WHILE:
        {
            ExprAST *expr = (ExprAST *)stmt->children[0].get();
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
            threeAddressCode->codes.back()->addresses.erase(threeAddressCode->codes.back()->addresses.begin());
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

            for (int i = 0; i < 2; i++)
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
                addresses.push_back(std::make_shared<Address>(tmpType, tmpLabel));
            }
            code = std::make_shared<ThreeAddress>(THREE_OP_RETURN, addresses);
            threeAddressCode->codes.emplace_back(code);
            break;
        }
        case STMT_ASSIGN:
        {
            stmt->children[0]->Traverse(this, THREEADDRESS);
            VariableAST *var;
            var = (VariableAST *)symtable->SearchTable(std::dynamic_pointer_cast<VariableAST>(stmt->children[0])->name);
            addresses.emplace_back(std::make_shared<Address>(THREE_VARIABLE, tmpLabel));
            stmt->children[1]->Traverse(this, THREEADDRESS);

            addresses.emplace_back(std::make_shared<Address>(tmpType, tmpLabel));

            code = std::make_shared<ThreeAddress>(THREE_OP_ASSIGN, addresses);
            threeAddressCode->codes.emplace_back(code);
            tmpLabel = var->name;
            break;
        }
        case STMT_EXPR:
        {
            stmt->children[0]->Traverse(this, THREEADDRESS);
        }
        break;
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
        addresses.emplace_back(std::make_shared<Address>(tmpType, tmpLabel));
        if (expr->RHS)
        {
            expr->RHS->Traverse(this, THREEADDRESS);
            addresses.emplace_back(std::make_shared<Address>(tmpType, tmpLabel));
        }
        tmpVar = ThreeAddress::GenTmpVar();
        addresses.insert(addresses.begin(), std::make_shared<Address>(THREE_TMP_VAR, tmpVar));
        std::shared_ptr<ThreeAddress> code = std::make_shared<ThreeAddress>(strOp2ThreeAddressOp[expr->op], addresses);
        threeAddressCode->codes.emplace_back(code);
        tmpLabel = tmpVar;
        tmpType = THREE_TMP_VAR;
    }
}

void ThreeAddress::Show()
{
    std::cout << ThreeAddressOp2Str[op] << ", ";
    for (int i = 0; i < addresses.size(); i++)
    {
        std::cout << addresses[i]->address << ", ";
    }
    std::cout << std::endl;
}

void ThreeAddressCode::Show()
{
    for (auto code : codes)
    {
        code->Show();
    }
}