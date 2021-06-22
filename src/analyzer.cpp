#include "AST.hpp"
#include "type.hpp"
#include "symtable.hpp"

void Visitor::Show(BaseAST *ast)
{
    if (typeid(*ast) == typeid(ProgAST))
    {
        indent += "  ";
        ProgAST *prog = (ProgAST *)ast;
        std::cout << indent << typeid(*ast).name() << " " << std::endl;
        for (auto child : prog->children)
            child->Traverse(this, SHOW);
        indent.pop_back();
        indent.pop_back();
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
        if(var->val)
            var->val->Traverse(this, SHOW);
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
        std::cout << indent << typeid(*expr).name() << " op " << expr->op << " " << expr->is_literal << std::endl;
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
        if(var->val)
            var->val->Traverse(this, ANALYZE);
        //变量定义
        if(var->type == INT)
        {
            //已经在符号表中。两种情况：嵌套作用域中的同名变量，同作用域中的重复定义。
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
        if (func->body)
            func->body->Traverse(this, ANALYZE);
        //函数定义
        if(func->returnType != NONE)
        {
            if(!symtable->Insert(func, func->name, FUNCTION))
            {
                fprintf(stderr, "redifinition! %s\n", func->name.c_str());
            }
        }
        //函数调用
        else
        {
            if(!symtable->SearchTable(func->name))
            {
                fprintf(stderr, "use before definition! %s\n", func->name.c_str());
            }
        }
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
    }
    else if (typeid(*ast) == typeid(ExprAST))
    {
        ExprAST *expr = (ExprAST *)ast;
        expr->LHS->Traverse(this, ANALYZE);
        if (expr->RHS)
            expr->RHS->Traverse(this, ANALYZE);
        
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