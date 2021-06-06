#pragma once
#include "global.hpp"
#include "type.hpp"

class BaseAST;
class SymTable;

class Visitor
{
    std::string indent;
    std::shared_ptr<SymTable> symtable;
public:
    Visitor();
    void Show(BaseAST *ast);
    void Analyze(BaseAST *ast);
};

enum VisitType
{
    SHOW,
    ANALYZE,

};

class BaseAST
{
public:
    virtual ~BaseAST() {}
    virtual void Traverse(Visitor visitor, VisitType type)
    {
        switch (type)
        {
        case SHOW:
            visitor.Show(this);
            break;
        case ANALYZE:
            visitor.Analyze(this);
        }
    }
    virtual bool IsLiteral()
    {
        return false;
    }
};

class ProgAST : public BaseAST
{
public:
    std::vector<std::shared_ptr<BaseAST>> children;

    ProgAST(std::vector<std::shared_ptr<BaseAST>> children)
        : children(std::move(children)) {}
    ~ProgAST()
    {
        for (auto ptr : children)
        {
            ptr.reset();
        }
    }
};

class BlockAST : public BaseAST
{
public:
    std::vector<std::shared_ptr<BaseAST>> children;

    BlockAST(std::vector<std::shared_ptr<BaseAST>> children)
        : children(std::move(children)) {}
};

class InitValAST : public BaseAST
{
public:
    std::string type;
    std::shared_ptr<BaseAST> val;

    InitValAST(const std::string &type, std::shared_ptr<BaseAST> val)
        : type(type), val(val) {}
};

class LiteralAST : public BaseAST
{
public:
    int val;

    LiteralAST(int val) : val(val) {}
    bool IsLiteral()
    {
        return true;
    }
};

class VariableAST : public BaseAST
{
public:
    BType type;
    std::string name;
    bool isConst;
    std::vector<std::shared_ptr<BaseAST>> dimensions;
    std::shared_ptr<InitValAST> val;

    VariableAST(BType type, const std::string &name, bool isConst)
        : type(type), name(name), isConst(isConst) {}
    VariableAST(BType type, const std::string &name, bool isConst, std::vector<std::shared_ptr<BaseAST>> dimensions)
        : type(type), name(name), isConst(isConst), dimensions(dimensions) {}
};

class FunctionAST : public BaseAST
{
public:
    BType returnType;
    std::string name;
    std::vector<std::shared_ptr<VariableAST>> parameters;
    std::shared_ptr<BlockAST> body;

    FunctionAST(BType returnType, const std::string &name,
                std::vector<std::shared_ptr<VariableAST>> parameters, std::shared_ptr<BlockAST> body)
        : returnType(returnType), name(name), parameters(std::move(parameters)), body(std::move(body)) {}
    FunctionAST(BType returnType, const std::string &name,
                std::vector<std::shared_ptr<VariableAST>> parameters)
        : returnType(returnType), name(name), parameters(std::move(parameters))
    {
        body = nullptr;
    }
};

class StmtAST : public BaseAST
{
public:
    Stmt_type type;
    std::vector<std::shared_ptr<BaseAST>> children;

    StmtAST(Stmt_type type, std::vector<std::shared_ptr<BaseAST>> children)
        : type(type), children(std::move(children)) {}
    StmtAST(Stmt_type type)
        : type(type) {}
};

class ExprAST : public BaseAST
{
public:
    int val;
    std::string op;
    bool is_literal;
    BType type;

    std::shared_ptr<BaseAST> LHS, RHS;
    ExprAST(const std::string &str) : op(str) {}
    bool IsLiteral()
    {
        return is_literal;
    }
};
