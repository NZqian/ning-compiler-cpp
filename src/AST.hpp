#pragma once
#include "global.hpp"
#include "type.hpp"

class BaseAST;
class SymTable;
class NameChanger;
class ThreeAddress;
class ThreeAddressCode;

class Visitor
{
    std::string indent;
    std::shared_ptr<SymTable> symtable;
    std::shared_ptr<ThreeAddress> curCode;
    std::shared_ptr<NameChanger> nameChanger;
    std::string tmpLabel;
    bool inLoop;
    std::vector<std::string> label[2];   

public:
    std::shared_ptr<ThreeAddressCode> threeAddressCode;
    Visitor();
    void Show(BaseAST *ast);
    void Analyze(BaseAST *ast);
    std::string GetNewVarName();
    void GenThreeAddress(BaseAST *ast);
};

enum VisitType
{
    SHOW,
    ANALYZE,
    THREEADDRESS,
};

class BaseAST
{
public:
    virtual ~BaseAST() {}

    virtual void Traverse(Visitor *visitor, VisitType type)
    {
        switch (type)
        {
        case SHOW:
            visitor->Show(this);
            break;
        case ANALYZE:
            visitor->Analyze(this);
            break;
        case THREEADDRESS:
            visitor->GenThreeAddress(this);
            break;
        }
    }

    virtual bool IsLiteral()
    {
        return false;
    }

    virtual std::string TypeName()
    {
        return typeid(*this).name();
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

    std::string TypeName()
    {
        return typeid(*this).name();
    }
};

class VariableAST : public BaseAST
{
public:
    BType type;
    std::string name;
    bool isConst;
    bool isGlobal;
    std::vector<std::shared_ptr<BaseAST>> dimensions;
    std::shared_ptr<BaseAST> val;

    VariableAST(BType type, const std::string &name, bool isConst, bool isGlobal)
        : type(type), name(name), isConst(isConst), isGlobal(isGlobal) {}
    VariableAST(BType type, const std::string &name, bool isConst, bool isGlobal, std::vector<std::shared_ptr<BaseAST>> dimensions)
        : type(type), name(name), isConst(isConst), isGlobal(isGlobal), dimensions(dimensions) {}
    VariableAST(BType type, const std::string &name, bool isConst, bool isGlobal, std::vector<std::shared_ptr<BaseAST>> dimensions, std::shared_ptr<BaseAST> val)
        : type(type), name(name), isConst(isConst), isGlobal(isGlobal), dimensions(dimensions), val(val) {}

    std::string TypeName()
    {
        return typeid(*this).name();
    }
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

    std::string TypeName()
    {
        return typeid(*this).name();
    }
};
