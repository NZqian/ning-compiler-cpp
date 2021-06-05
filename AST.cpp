#include "AST.hpp"

Visitor::Visitor()
{
    indent = "";
}

void Visitor::Show(BaseAST *ast)
{
    if (typeid(*ast) == typeid(ProgAST))
    {
        indent += "  ";
        ProgAST *prog = (ProgAST*)ast;
        std::cout << indent << typeid(*ast).name() << " " << std::endl;
        for (auto child : prog->children)
            child->Traverse(*this, SHOW);
        indent.pop_back();
        indent.pop_back();
    }
    else if (typeid(*ast) == typeid(BlockAST))
    {
        indent += "  ";
        BlockAST *block = (BlockAST*)ast;
        std::cout << indent << typeid(*ast).name() << " " << std::endl;
        for (auto child : block->children)
            child->Traverse(*this, SHOW);
        indent.pop_back();
        indent.pop_back();
    }
    else if (typeid(*ast) == typeid(LiteralAST))
    {
        indent += "  ";
        LiteralAST *literal = (LiteralAST*)ast;
        std::cout << indent << typeid(*ast).name() << " " << literal->val << std::endl;
        indent.pop_back();
        indent.pop_back();
    }
    else if (typeid(*ast) == typeid(VariableAST))
    {
        indent += "  ";
        VariableAST *var = (VariableAST*)ast;
        std::cout << indent << typeid(*ast).name() << " " << var->type << ' ' << var->name << ' ';
        if(var->isConst)
            std::cout << "const" << std::endl;
        else
            std::cout << std::endl;
        indent.pop_back();
        indent.pop_back();
    }
    else if (typeid(*ast) == typeid(FunctionAST))
    {
        indent += "  ";
        FunctionAST *func = (FunctionAST*)ast;
        std::cout << indent << typeid(*ast).name() << " " << func->returnType << ' ' << func->name << std::endl;
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
        std::cout << indent << typeid(*expr).name() << " op " << expr->op << " " << std::endl;
        expr->LHS->Traverse(*this, SHOW);
        if (expr->RHS)
            expr->RHS->Traverse(*this, SHOW);
        indent.pop_back();
        indent.pop_back();
    }
}