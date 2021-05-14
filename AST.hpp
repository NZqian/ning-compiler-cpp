#include "global.hpp"


class BaseAST
{
public:
    virtual ~BaseAST() {}
    virtual void Traverse(int depth)
    {
        std::cout<<"Unimplemented"<<std::endl;
    }
};

class ProgAST : public BaseAST
{
    std::vector<std::shared_ptr<BaseAST>> children;

public:
    ProgAST(std::vector<std::shared_ptr<BaseAST>> children)
        : children(std::move(children)) {}
    void Traverse(int depth)
    {}
};

class BlockAST : public BaseAST
{
    std::vector<std::shared_ptr<BaseAST>> children;

public:
    BlockAST(std::vector<std::shared_ptr<BaseAST>> children)
        : children(std::move(children)) {}
    void Traverse(int depth);
};

class InitValAST: public BaseAST
{
    std::string type;
    std::shared_ptr<BaseAST> val;
    public:
    InitValAST(const std::string &type, std::shared_ptr<BaseAST> val)
    : type(type), val(val){}
};

class LiteralAST: public BaseAST
{
    int val;
    public:
    LiteralAST(int val):val(val){}
};

class VariableAST : public BaseAST
{
    std::string type;
    std::string name;
    bool isConst;
    std::vector<int> dimensions;
    
    std::shared_ptr<InitValAST> val; 

public:
    VariableAST(const std::string &type, const std::string &name, bool isConst)
    :type(type), name(name), isConst(isConst){}
    VariableAST(const std::string &type, const std::string &name, bool isConst, std::vector<int> dimensions)
    :type(type), name(name), isConst(isConst), dimensions(dimensions){}
    virtual void Traverse(int depth)
    {}
};

class TypeDefAST : public BaseAST
{

public:
};

class FunctionAST : public BaseAST
{
    std::string returnType;
    std::string name;
    std::vector<std::shared_ptr<VariableAST>> parameters;
    std::shared_ptr<BlockAST> body;

public:
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
    Stmt_type type;
    std::vector<std::shared_ptr<BaseAST>> children;

public:
    StmtAST(Stmt_type type, std::vector<std::shared_ptr<BaseAST>> children)
        : type(type), children(std::move(children)) {}
    StmtAST(Stmt_type type)
        : type(type) {}
};

class ExprAST : public BaseAST
{
    int val;
    public:
    ExprAST(int val):val(val){}
};