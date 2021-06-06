#include "AST.hpp"
#include "type.hpp"
#include "symtable.hpp"

Visitor::Visitor()
{
    indent = "";
    symtable = std::make_shared<SymTable>();
}

void Visitor::Show(BaseAST *ast)
{
    if (typeid(*ast) == typeid(ProgAST))
    {
        indent += "  ";
        ProgAST *prog = (ProgAST *)ast;
        std::cout << indent << typeid(*ast).name() << " " << std::endl;
        for (auto child : prog->children)
            child->Traverse(*this, SHOW);
        indent.pop_back();
        indent.pop_back();
    }
    else if (typeid(*ast) == typeid(BlockAST))
    {
        indent += "  ";
        BlockAST *block = (BlockAST *)ast;
        std::cout << indent << typeid(*ast).name() << " " << std::endl;
        for (auto child : block->children)
            child->Traverse(*this, SHOW);
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
        indent.pop_back();
        indent.pop_back();
    }
    else if (typeid(*ast) == typeid(FunctionAST))
    {
        indent += "  ";
        FunctionAST *func = (FunctionAST *)ast;
        std::string returnType = BType2Str[func->returnType];
        std::cout << indent << typeid(*ast).name() << " " << returnType << ' ' << func->name << std::endl;
        if (func->body)
            func->body->Traverse(*this, SHOW);
        indent.pop_back();
        indent.pop_back();
    }
    else if (typeid(*ast) == typeid(StmtAST))
    {
        indent += "  ";
        StmtAST *stmt = (StmtAST *)ast;
        std::cout << indent << typeid(*stmt).name() << " " << Stmt_type2str[stmt->type] << " " << std::endl;
        for (auto child : stmt->children)
            child->Traverse(*this, SHOW);
        indent.pop_back();
        indent.pop_back();
    }
    else if (typeid(*ast) == typeid(ExprAST))
    {
        indent += "  ";
        ExprAST *expr = (ExprAST *)ast;
        std::cout << indent << typeid(*expr).name() << " op " << expr->op << " " << expr->is_literal << std::endl;
        expr->LHS->Traverse(*this, SHOW);
        if (expr->RHS)
            expr->RHS->Traverse(*this, SHOW);
        indent.pop_back();
        indent.pop_back();
    }
}

void Visitor::Analyze(BaseAST *ast)
{
    if (typeid(*ast) == typeid(ProgAST))
    {
        ProgAST *prog = (ProgAST *)ast;
        symtable->AddTable();
        for (auto child : prog->children)
            child->Traverse(*this, ANALYZE);
        symtable->DeleteTable();
    }
    else if (typeid(*ast) == typeid(BlockAST))
    {
        BlockAST *block = (BlockAST *)ast;
        symtable->AddTable();
        for (auto child : block->children)
            child->Traverse(*this, ANALYZE);
        symtable->DeleteTable();
    }
    else if (typeid(*ast) == typeid(LiteralAST))
    {
        LiteralAST *literal = (LiteralAST *)ast;
    }
    else if (typeid(*ast) == typeid(VariableAST))
    {
        VariableAST *var = (VariableAST *)ast;
        if(var->type == INT)
        {
            if(!symtable->Insert(var, var->name, VARIABLE))
            {
                fprintf(stderr, "redifinition! %s\n", var->name.c_str());
            }
        }
        else if(var->type == NONE)
        {
            if(!symtable->SearchTable(var->name, VARIABLE))
            {
                fprintf(stderr, "use before definition! %s\n", var->name.c_str());
            }
        }
    }
    else if (typeid(*ast) == typeid(FunctionAST))
    {
        FunctionAST *func = (FunctionAST *)ast;
        if (func->body)
            func->body->Traverse(*this, ANALYZE);
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
    else if (typeid(*ast) == typeid(StmtAST))
    {
        StmtAST *stmt = (StmtAST *)ast;
        for (auto child : stmt->children)
            child->Traverse(*this, ANALYZE);
    }
    else if (typeid(*ast) == typeid(ExprAST))
    {
        ExprAST *expr = (ExprAST *)ast;
        expr->LHS->Traverse(*this, ANALYZE);
        if (expr->RHS)
            expr->RHS->Traverse(*this, ANALYZE);
        
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