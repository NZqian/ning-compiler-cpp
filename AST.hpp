#include "global.hpp"
#include "type.hpp"

class BaseAST;

class Visitor
{
    std::string indent;
public:
    Visitor();
    void Show(BaseAST *ast);
};

enum VisitType
{
    SHOW,
    TYPE_CHECK,

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
        }
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
};

class VariableAST : public BaseAST
{
public:
    std::string type;
    std::string name;
    bool isConst;
    std::vector<std::shared_ptr<BaseAST>> dimensions;
    std::shared_ptr<InitValAST> val;

    VariableAST(const std::string &type, const std::string &name, bool isConst)
        : type(type), name(name), isConst(isConst) {}
    VariableAST(const std::string &type, const std::string &name, bool isConst, std::vector<std::shared_ptr<BaseAST>> dimensions)
        : type(type), name(name), isConst(isConst), dimensions(dimensions) {}
};

class FunctionAST : public BaseAST
{
public:
    std::string returnType;
    std::string name;
    std::vector<std::shared_ptr<VariableAST>> parameters;
    std::shared_ptr<BlockAST> body;

    FunctionAST(const std::string &returnType, const std::string &name,
                std::vector<std::shared_ptr<VariableAST>> parameters, std::shared_ptr<BlockAST> body)
        : returnType(returnType), name(name), parameters(std::move(parameters)), body(std::move(body)) {}
    FunctionAST(const std::string &returnType, const std::string &name,
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

    std::shared_ptr<BaseAST> LHS, RHS;
    ExprAST(const std::string &str) : op(str) {}
};
