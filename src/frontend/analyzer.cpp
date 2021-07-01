#include "../global/AST.hpp"
#include "../global/type.hpp"
#include "symtable.hpp"

void Visitor::Show(BaseAST *ast)
{
    if (typeid(*ast) == typeid(ProgAST))
    {
        ProgAST *prog = (ProgAST *)ast;
        std::cout << indent << typeid(*ast).name() << " " << std::endl;
        for (auto child : prog->children)
            child->Traverse(this, SHOW);
    }
    else if (typeid(*ast) == typeid(BlockAST))
    {
        indent += "  ";
        BlockAST *block = (BlockAST *)ast;
        std::cout << indent << typeid(*ast).name() << " " << std::endl;
        for (auto child : block->children)
            child->Traverse(this, SHOW);
        indent.pop_back();
        indent.pop_back();
    }
    else if (typeid(*ast) == typeid(LiteralAST))
    {
        indent += "  ";
        LiteralAST *literal = (LiteralAST *)ast;
        std::cout << indent << typeid(*ast).name() << " " << literal->val << std::endl;
        indent.pop_back();
        indent.pop_back();
    }
    else if (typeid(*ast) == typeid(VariableAST))
    {
        indent += "  ";
        VariableAST *var = (VariableAST *)ast;
        std::string type = BType2Str[var->type];
        std::cout << indent << typeid(*ast).name() << " " << type << ' ' << var->name << ' ';
        if (var->isConst)
            std::cout << "const" << std::endl;
        else
            std::cout << std::endl;
        if (var->dimensions.size())
        {
            std::cout << indent << "dimensions: \n";
            for (int i = 0; i < var->dimensions.size(); i++)
            {
                var->dimensions[i]->Traverse(this, SHOW);
                //std::cout << var->dimensions[i]->GetVal() << ", ";
            }
        }
        if(var->val)
        {
            std::cout << indent << "vals: \n";
            var->val->Traverse(this, SHOW);
        }
        indent.pop_back();
        indent.pop_back();
    }
    else if (typeid(*ast) == typeid(ArrayAST))
    {
        indent += "  ";
        ArrayAST *arr = (ArrayAST *)ast;
        std::cout << indent << "array: \n";
        for (auto item : arr->items)
        {
            item->Traverse(this, SHOW);
        }
        indent.pop_back();
        indent.pop_back();
    }
    else if (typeid(*ast) == typeid(FunctionAST))
    {
        indent += "  ";
        FunctionAST *func = (FunctionAST *)ast;
        std::string returnType = BType2Str[func->returnType];
        std::cout << indent << typeid(*ast).name() << " " << returnType << ' ' << func->name << std::endl;
        indent += "  ";
        std::cout << indent << "params" << std::endl;
        for (auto param : func->parameters)
        {
            param->Traverse(this, SHOW);
        }
        indent.pop_back();
        indent.pop_back();
        if (func->body)
            func->body->Traverse(this, SHOW);
        indent.pop_back();
        indent.pop_back();
    }
    else if (typeid(*ast) == typeid(StmtAST))
    {
        indent += "  ";
        StmtAST *stmt = (StmtAST *)ast;
        std::cout << indent << typeid(*stmt).name() << " " << Stmt_type2str[stmt->type] << " " << std::endl;
        for (auto child : stmt->children)
            child->Traverse(this, SHOW);
        indent.pop_back();
        indent.pop_back();
    }
    else if (typeid(*ast) == typeid(ExprAST))
    {
        indent += "  ";
        ExprAST *expr = (ExprAST *)ast;
        std::cout << indent << typeid(*expr).name() << " op " << expr->op << " " << expr->is_literal << " " << expr->val << std::endl;
        expr->LHS->Traverse(this, SHOW);
        if (expr->RHS)
            expr->RHS->Traverse(this, SHOW);
        indent.pop_back();
        indent.pop_back();
    }
}

std::string NameChanger::GetNewVarName(std::shared_ptr<SymTable> symtable)
{
    //name exists
    while (symtable->SearchTable(curNewName))
    {
        if(curNewName.back() < 'z') 
        {
            curNewName.back() ++;
        }
        else
        {
            curNewName.push_back('a');
        }
    }
    return curNewName;
}

void NameChanger::EnterNameScope()
{
    namescope.push_back(std::vector<std::pair<std::string, std::string> >());
}

void NameChanger::ExitNameScope()
{
    namescope.pop_back();
}

void NameChanger::AddChange(std::string oldName, std::shared_ptr<SymTable> symtable)
{
    GetNewVarName(symtable);
    namescope.back().push_back(std::make_pair(oldName, curNewName));
}

std::string NameChanger::UseChange(std::string curName)
{
    for (int i = namescope.size() - 1; i >= 0; i--)
    {
        std::vector<std::pair<std::string, std::string>> curScope = namescope[i];
        for (auto p : curScope)
        {
            if(p.first == curName)
                return p.second;
        }

    }
    fprintf(stderr, "what the fuck\n");
    exit(-1);
}

bool NameChanger::InNameScope(std::string curName)
{
    for (int i = namescope.size() - 1; i >= 0; i--)
    {
        std::vector<std::pair<std::string, std::string>> curScope = namescope[i];
        for (auto p : curScope)
        {
            if(p.first == curName)
                return true;
        }
    }
    return false;
}

bool NameChanger::InCurNameScope(std::string curName)
{
    for (auto p : namescope.back())
    {
        if(p.first == curName)
            return true;
    }
    return false;
}

std::string NameChanger::GetNewFuncName(std::shared_ptr<SymTable> symtable, std::string curName)
{
    std::string newName = curName + "_" + GetNewVarName(symtable);
    funcNameMap[curName] = newName;
    return newName;
}

std::string NameChanger::ReplaceFuncName(std::string curName)
{
    auto iter = funcNameMap.find(curName);
    //没找到，不改名
    if (iter == funcNameMap.end())
    {
        /*
        fprintf(stderr, "new func name not found\n");
        exit(-1);
        */
        return curName;
    }
    return funcNameMap[curName];
}

void Visitor::Analyze(BaseAST *ast)
{
    if (typeid(*ast) == typeid(ProgAST))
    {
        ProgAST *prog = (ProgAST *)ast;
        nameChanger->EnterNameScope();
        for (auto child : prog->children)
            child->Traverse(this, ANALYZE);
        nameChanger->ExitNameScope();
    }
    else if (typeid(*ast) == typeid(BlockAST))
    {
        BlockAST *block = (BlockAST *)ast;
        nameChanger->EnterNameScope();
        for (auto child : block->children)
            child->Traverse(this, ANALYZE);
        nameChanger->ExitNameScope();
    }
    else if (typeid(*ast) == typeid(LiteralAST))
    {
        LiteralAST *literal = (LiteralAST *)ast;
    }
    else if (typeid(*ast) == typeid(VariableAST))
    {
        VariableAST *var = (VariableAST *)ast;
        if (var->dimensions.size())
        {
            for (auto dimension : var->dimensions)
            {
                dimension->Traverse(this, ANALYZE);
            }
        }
        if (var->val)
        {
            var->val->Traverse(this, ANALYZE);
        }
        //变量定义
        if(var->type == INT)
        {
            //已经在符号表中。三种情况：与函数重名，嵌套作用域中的同名变量，同作用域中的重复定义。
            if(!symtable->Insert(var, var->name, VARIABLE))
            {
                //已经在名字替换向量中，为重复定义
                if(nameChanger->InCurNameScope(var->name))
                {
                    fprintf(stderr, "redifinition! %s\n", var->name.c_str());
                    exit(-1);
                }
                //将名字添加到名字替换向量中
                else
                {
                    nameChanger->AddChange(var->name, symtable);
                    var->name = nameChanger->UseChange(var->name);
                    symtable->Insert(var, var->name, VARIABLE);
                }
            }
            //数组
            if (var->dimensions.size() && var->val)
            {
                //有初值
                if (var->val)
                {
                    var->ReconstructArr();
                }
                //初始化为0
                else
                {
                    std::shared_ptr<ArrayAST> arr = std::dynamic_pointer_cast<ArrayAST>(var->val);
                    arr->items.clear();
                    FillArray(arr, var->dimensions, 0);
                }
            }
        }
        //变量使用
        else if(var->type == NONE)
        {
            if(nameChanger->InNameScope(var->name))
            {
                var->name = nameChanger->UseChange(var->name);
            }
            if(!symtable->SearchTable(var->name))
            {
                fprintf(stderr, "use before definition! %s\n", var->name.c_str());
                exit(0);
            }
            if (((VariableAST*)symtable->SearchTable(var->name))->isConst)
            {
                var->isConst = true;
                var->val = std::make_shared<LiteralAST>(symtable->SearchTable(var->name)->GetVal());
            }
        }
    }
    //函数可能与变量重名。
    else if (typeid(*ast) == typeid(FunctionAST))
    {
        FunctionAST *func = (FunctionAST *)ast;
        nameChanger->EnterNameScope();
        for (auto param : func->parameters)
            param->Traverse(this, ANALYZE);
        //函数定义
        if(func->returnType != NONE)
        {
            if(!symtable->Insert(func, func->name, FUNCTION))
            {
                //和函数重名，即重复定义
                if(symtable->SearchTableDefinition(func->name)->type == FUNCTION)
                {
                    fprintf(stderr, "redifinition! %s\n", func->name.c_str());
                    exit(-1);
                }
                //和变量重名
                else
                {
                    func->name = nameChanger->GetNewFuncName(symtable, func->name);
                    symtable->Insert(func, func->name, FUNCTION);
                }
            }
        }
        //函数调用
        else
        {
            func->name = nameChanger->ReplaceFuncName(func->name);
            Definition *searchResult = symtable->SearchTableDefinition(func->name);
            if(!searchResult || searchResult->type != FUNCTION)
            {
                fprintf(stderr, "use before definition! %s\n", func->name.c_str());
                exit(-1);
            }
        }
        if (func->body)
            func->body->Traverse(this, ANALYZE);
        nameChanger->ExitNameScope();
    }
    else if (typeid(*ast) == typeid(StmtAST))
    {
        StmtAST *stmt = (StmtAST *)ast;
        Stmt_type type = stmt->type;
        bool inLoopChangFlag = false;

        if(type == STMT_WHILE)
        {
            if(!inLoop)
            {
                inLoopChangFlag = true;
            }
            inLoop = true;
        }
        else if((type == STMT_BREAK || type == STMT_CONTINUE) && !inLoop)
        {
            fprintf(stderr, "break or coninue out of loop!\n");
            exit(-1);
        }
        for (auto child : stmt->children)
        {
            child->Traverse(this, ANALYZE);
        }
        //改变了状态时退出while设false，即从一层while进入另一层while不设false
        if(type == STMT_WHILE && inLoopChangFlag)
        {
            inLoop = false;
        }

        if (type == STMT_ASSIGN)
        {
            std::string varName = ((VariableAST*)stmt->children[0].get())->name;
            VariableAST *varDef = (VariableAST*)symtable->SearchTable(varName);
            if (varDef->isConst)
            {
                fprintf(stderr, "cannot assign const var %s\n", varName.c_str());
                exit(-1);
            }
        }
    }
    else if (typeid(*ast) == typeid(ExprAST))
    {
        ExprAST *expr = (ExprAST *)ast;
        expr->LHS->Traverse(this, ANALYZE);
        if (expr->RHS)
            expr->RHS->Traverse(this, ANALYZE);
        
        int lval, rval;
        if (expr->LHS->IsLiteral())
        {
            lval = expr->LHS->GetVal();
            if(expr->RHS)
            {
                rval = expr->RHS->GetVal();
                if(expr->RHS->IsLiteral())
                {
                    if(expr->op == "+")
                    {
                        expr->val = lval + rval;
                    }
                    if(expr->op == "-")
                    {
                        expr->val = lval - rval;
                    }
                    if(expr->op == "*")
                    {
                        expr->val = lval * rval;
                    }
                    if(expr->op == "/")
                    {
                        expr->val = lval / rval;
                    }
                    if(expr->op == "%")
                    {
                        expr->val = lval % rval;
                    }
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
                expr->val = lval;
            }
        }
        else
        {
            expr->is_literal = false;
        }
    }
}