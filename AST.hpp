#include "global.hpp"

class BaseAST
{
public:
    virtual ~BaseAST() {}
    virtual void Traverse(int depth)
    {
        std::cout << "Unimplemented" << std::endl;
    }
};

class ProgAST : public BaseAST
{
    std::vector<std::shared_ptr<BaseAST>> children;

public:
    ProgAST(std::vector<std::shared_ptr<BaseAST>> children)
        : children(std::move(children)) {}
    void Traverse(int depth)
    {
        std::string tab;
        for (int i = 0; i < depth; i++)
            tab += "  ";
        std::cout << tab << typeid(this).name() << " " << std::endl;
        for (auto child : children)
        {
            child->Traverse(depth + 1);
        }
    }
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
    std::vector<std::shared_ptr<BaseAST>> children;

public:
    BlockAST(std::vector<std::shared_ptr<BaseAST>> children)
        : children(std::move(children)) {}
    void Traverse(int depth)
    {
        std::string tab;
        for (int i = 0; i < depth; i++)
            tab += "  ";
        std::cout << tab << typeid(this).name() << " " << std::endl;
        for (auto child : children)
        {
            child->Traverse(depth + 1);
        }
    }
};

class InitValAST : public BaseAST
{
    std::string type;
    std::shared_ptr<BaseAST> val;

public:
    InitValAST(const std::string &type, std::shared_ptr<BaseAST> val)
        : type(type), val(val) {}
};

class LiteralAST : public BaseAST
{
    int val;

public:
    LiteralAST(int val) : val(val) {}
    void Traverse(int depth)
    {
        std::string tab;
        for (int i = 0; i < depth; i++)
            tab += "  ";
        std::cout << tab << typeid(this).name() << " val " <<val<< std::endl;
    }
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
        : type(type), name(name), isConst(isConst) {}
    VariableAST(const std::string &type, const std::string &name, bool isConst, std::vector<int> dimensions)
        : type(type), name(name), isConst(isConst), dimensions(dimensions) {}
    void Traverse(int depth)
    {
        std::string tab;
        for (int i = 0; i < depth; i++)
            tab += "  ";
        std::cout << tab << typeid(this).name() << " " <<type<<' '<<name<<' '<<isConst<< std::endl;

    }
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
    void Traverse(int depth)
    {
        std::string tab;
        for (int i = 0; i < depth; i++)
            tab += "  ";
        std::cout << tab << typeid(this).name() << " " <<returnType<<' '<<name<< std::endl;
        if(body)
            body->Traverse(depth+1);
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
    void Traverse(int depth)
    {
        std::string tab;
        for (int i = 0; i < depth; i++)
            tab += "  ";
        std::cout << tab << typeid(this).name()<<" "<<type << " " << std::endl;
        for(auto child : children)
        {
            child->Traverse(depth+1);
        }
    }
};

class ExprAST : public BaseAST
{
    int val;
    std::string op;

public:
    std::shared_ptr<BaseAST> LHS, RHS;
    ExprAST(const std::string &str) : op(str) {}
    void Traverse(int depth)
    {
        
        std::string tab;
        for (int i = 0; i < depth; i++)
            tab += "  ";
        std::cout << tab << typeid(this).name()<<" op "<<op << " " << std::endl;
        LHS->Traverse(depth+1);
        if(RHS)
            RHS->Traverse(depth+1);
    }
};

/*
class ASTVisitor
{
    public:
    void visit(BaseAST ast, Visit_type type)
    {
        switch(type)
        {
            case VISIT_OUTPUT:
            break;
            default:
            fprintf(stderr, "%s", "unknown type\n");
        }
    }
};
*/