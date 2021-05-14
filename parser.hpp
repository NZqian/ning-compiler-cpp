#pragma once
#include "global.hpp"
#include "lexer.hpp"
#include "AST.hpp"

class Parser
{
    int depth;
    std::map<std::string, int> binopPrecedence;
    Tok_type curToken;
    std::string curIdentifier;
    int curNumVal;
    Tok_type nextToken; //look ahead
    std::string nextIdentifier;
    int nextNumVal;
    std::shared_ptr<Lexer> lexer;
    int GetNextToken();
    int GetTokenPrecedence();

    void LogError();
    void LogParse(const std::string &str);
    void ParseTypeDef();
    std::shared_ptr<BlockAST> ParseBlock();
    std::shared_ptr<BaseAST> ParseDefinition();
    std::shared_ptr<FunctionAST> ParseFunction(const std::string &type, const std::string &name);
    std::shared_ptr<VariableAST> ParseVariable(const std::string &type, const std::string &name, bool isConst);
    std::shared_ptr<VariableAST> ParseLValue();
    std::shared_ptr<VariableAST> ParseVariable();
    std::vector<std::shared_ptr<VariableAST>> ParseParameter();
    std::shared_ptr<BaseAST> ParseBody();
    std::shared_ptr<BaseAST> ParseDecl();
    std::shared_ptr<BaseAST> ParseSimple();
    std::shared_ptr<BaseAST> ParseStmt();

    std::shared_ptr<StmtAST> ParseIf();
    std::shared_ptr<StmtAST> ParseWhile();
    std::shared_ptr<StmtAST> ParseBreak();
    std::shared_ptr<StmtAST> ParseContinue();
    std::shared_ptr<StmtAST> ParseReturn();

    std::shared_ptr<ExprAST> ParseExpr();
    std::shared_ptr<ExprAST> ParseAddExpr();
    std::shared_ptr<ExprAST> ParseCondExpr();
    std::shared_ptr<ExprAST> ParsePrimaryExpr();
    std::shared_ptr<ExprAST> ParseLValExpr();
    std::shared_ptr<ExprAST> ParseNumberExpr();
    std::shared_ptr<ExprAST> ParseUnaryExpr();
    std::shared_ptr<ExprAST> ParseMulExpr();
    std::shared_ptr<ExprAST> ParseRelExpr();
    std::shared_ptr<ExprAST> ParseEqExpr();
    std::shared_ptr<ExprAST> ParseLOrExpr();
    std::shared_ptr<ExprAST> ParseConstExpr();

public:
    Parser(const std::string &filename);
    std::shared_ptr<ProgAST> Parse();
    static void TraverseAST(std::shared_ptr<BaseAST> &root, int depth);
    static void TraverseAST(BaseAST *root, int depth);
};
