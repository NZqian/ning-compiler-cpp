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
    std::shared_ptr<FunctionAST> ParseFunction(BType type, const std::string &name);
    std::shared_ptr<FunctionAST> ParseFunctionCall();
    std::shared_ptr<VariableAST> ParseVariable(BType type, const std::string &name, bool isConst);
    std::shared_ptr<VariableAST> ParseLValue();
    std::shared_ptr<VariableAST> ParseVariable();
    std::vector<std::shared_ptr<VariableAST>> ParseDefParam();
    std::vector<std::shared_ptr<VariableAST>> ParseCallParam();
    std::shared_ptr<BaseAST> ParseBody();
    std::shared_ptr<BaseAST> ParseDecl();
    std::shared_ptr<BaseAST> ParseSimple();
    std::shared_ptr<BaseAST> ParseStmt();

    std::shared_ptr<StmtAST> ParseIf();
    std::shared_ptr<StmtAST> ParseWhile();
    std::shared_ptr<StmtAST> ParseBreak();
    std::shared_ptr<StmtAST> ParseContinue();
    std::shared_ptr<StmtAST> ParseReturn();

    std::shared_ptr<BaseAST> ParseExpr();
    std::shared_ptr<BaseAST> ParseAddExpr();
    std::shared_ptr<BaseAST> ParseCondExpr();
    std::shared_ptr<BaseAST> ParsePrimaryExpr();
    std::shared_ptr<BaseAST> ParseLValExpr();
    std::shared_ptr<BaseAST> ParseNumberExpr();
    std::shared_ptr<BaseAST> ParseUnaryExpr();
    std::shared_ptr<BaseAST> ParseMulExpr();
    std::shared_ptr<BaseAST> ParseRelExpr();
    std::shared_ptr<BaseAST> ParseEqExpr();
    std::shared_ptr<BaseAST> ParseLOrExpr();
    std::shared_ptr<BaseAST> ParseLAndExpr();
    std::shared_ptr<BaseAST> ParseConstExpr();

public:
    Parser(const std::string &filename);
    std::shared_ptr<ProgAST> Parse();
    static void TraverseAST(std::shared_ptr<BaseAST> &root, int depth);
    static void TraverseAST(BaseAST *root, int depth);
};
