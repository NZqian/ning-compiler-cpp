#include "parser.hpp"
#define LEXER_DEBUG 0
#define PARSER_DEBUG 1
Parser::Parser(const std::string &filename)
{
    binopPrecedence[""] = 10;
    lexer = std::make_shared<Lexer>(filename);
    std::cout << "constructed parser" << std::endl;
    curToken = TOK_UNKNOWN;
    nextToken = TOK_UNKNOWN;
    curIdentifier = "";
    nextIdentifier = "";
}

int Parser::GetNextToken()
{
    curToken = nextToken;
    curIdentifier = std::move(nextIdentifier);
    curNumVal = nextNumVal;
    nextToken = lexer->GetTok();
    nextIdentifier = lexer->identifier;
    nextNumVal = lexer->numVal;
#if LEXER_DEBUG
    lexer->PrintToken(curToken, curIdentifier);
#endif
    return curToken;
}

int Parser::GetTokenPrecedence()
{
    int precedence = binopPrecedence[lexer->identifier];
    if (precedence == 0)
        return -1;
    return precedence;
}

std::shared_ptr<ProgAST> Parser::Parse()
{
    std::vector<std::shared_ptr<BaseAST>> children;
    GetNextToken();
    GetNextToken();
    while (true)
    {
        //fprintf(stderr, "parsing> ");
        std::shared_ptr<BaseAST> definition = ParseDefinition();
        if (definition)
            children.emplace_back(std::move(definition));
        else
            return nullptr;
        if (curToken == TOK_EOF)
            break;
    }
    std::shared_ptr<ProgAST> root = std::make_shared<ProgAST>(std::move(children));
    return root;
}

std::shared_ptr<BlockAST> Parser::ParseBlock()
{
    std::vector<std::shared_ptr<BaseAST>> children;
    std::shared_ptr<BaseAST> child;
    GetNextToken(); //eat {
    while (curIdentifier != "}")
    {
        if (curToken == TOK_CONST || curToken == TOK_INT)
        {
            child = ParseVariable();
            if (!child)
                return nullptr;
            children.emplace_back(child);
        }
        else
        {
            child = ParseStmt();
            if (!child)
                return nullptr;
            children.emplace_back(child);
        }
    }
    GetNextToken(); //eat }
    std::shared_ptr<BlockAST> block = std::make_shared<BlockAST>(children);
    return block;
    //return std::make_shared<BlockAST>(children);
}

std::shared_ptr<BaseAST> Parser::ParseDefinition()
{
    bool isConst = false;
    if (curToken == TOK_CONST)
    {
        GetNextToken(); //eat const
        isConst = true;
    }
    if (curToken == TOK_INT || curToken == TOK_VOID)
    {
        BType type = NONE;
        std::string name;
        if(curIdentifier == "int")
            type = INT;
        else if(curIdentifier == "void")
            type = VOID;
        GetNextToken(); //eat type
        name = curIdentifier;
        GetNextToken(); //eat name
        if (curIdentifier == "(")
            return ParseFunction(type, name);
        else if (curIdentifier == "=" || curIdentifier == "[" || curIdentifier == ";")
            return ParseVariable(type, name, isConst);
        return nullptr;
    }
    return nullptr;
}

std::shared_ptr<FunctionAST> Parser::ParseFunction(BType returnType, const std::string &name)
{
    std::vector<std::shared_ptr<VariableAST>> parameters = ParseDefParam();
    if (curIdentifier == "{")
    {
        std::shared_ptr<BlockAST> body = ParseBlock();
        LogParse("parsed function definition");
        return std::make_shared<FunctionAST>(returnType, name,
                                             std::move(parameters), std::move(body));
    }
    else
    {
        GetNextToken(); //eat ;
        LogParse("parsed function declearation");
        return std::make_shared<FunctionAST>(returnType, name, std::move(parameters));
    }
}

std::shared_ptr<VariableAST> Parser::ParseVariable(BType type, const std::string &name, bool isConst)
{
    std::vector<std::shared_ptr<BaseAST> > dimensions;
    while (curIdentifier == "[") //数组
    {
        GetNextToken(); //eat [
        std::shared_ptr<BaseAST> dimension;
        dimension = ParseAddExpr();
        if(dimension)
            dimensions.emplace_back(dimension);
        //dimensions.emplace_back(curNumVal);
        GetNextToken(); //eat ]
    }
    if (curIdentifier == "=")
    {
        GetNextToken(); //eat =
        if (curIdentifier == "{")
        {
        }
        GetNextToken(); //eat val
    }
    GetNextToken(); //eat ;
    std::shared_ptr<VariableAST> variable =
        std::make_shared<VariableAST>(type, name, isConst, dimensions);
    LogParse("parsed variable");
    return variable;
}

std::shared_ptr<VariableAST> Parser::ParseVariable()
{
    bool isConst = false;
    std::string name;
    BType type = NONE;
    if (curToken == TOK_CONST)
    {
        isConst = true;
        GetNextToken(); //eat const
    }
    if(curIdentifier == "int")
        type = INT;
    else if(curIdentifier == "void")
        type = VOID;
    GetNextToken(); //eat type
    name = curIdentifier;
    GetNextToken(); //eat name
    return ParseVariable(type, name, isConst);
}

std::shared_ptr<VariableAST> Parser::ParseLValue()
{
    std::string name;
    name = curIdentifier;
    GetNextToken();
    std::vector<std::shared_ptr<BaseAST> > dimensions;
    while (curIdentifier == "[") //数组
    {
        GetNextToken(); //eat [
        std::shared_ptr<BaseAST> dimension;
        dimension = ParseAddExpr();
        if(dimension)
            dimensions.emplace_back(dimension);
        GetNextToken(); //eat ]
    }
    return std::make_shared<VariableAST>(BType::NONE, name, false, dimensions);
}

std::shared_ptr<BaseAST> Parser::ParseStmt()
{
    if (curToken == TOK_IDENTIFIER || curToken == TOK_NUMBER)
    {
        if(nextIdentifier == "=" || nextIdentifier == "[")  //LVal=Exp
        {
            std::shared_ptr<VariableAST> lval = ParseLValue();
            GetNextToken(); //eat =
            std::shared_ptr<BaseAST> expr = ParseExpr();
            GetNextToken(); //eat ;
            std::vector<std::shared_ptr<BaseAST>> children{lval, expr};
            LogParse("parsed assign");
            return std::make_shared<StmtAST>(STMT_ASSIGN, children);
        }
        else    //[Exp]
        {
            std::shared_ptr<BaseAST> stmt = std::make_shared<StmtAST>(STMT_EXPR, std::vector<std::shared_ptr<BaseAST>>{ParseExpr()});
            GetNextToken();
            return stmt;
        }
    }
    else if (curToken == TOK_IF)
    {
        return ParseIf();
    }
    else if (curToken == TOK_WHILE)
    {
        return ParseWhile();
    }
    else if (curToken == TOK_BREAK)
    {
        return ParseBreak();
    }
    else if (curToken == TOK_CONTINUE)
    {
        return ParseContinue();
    }
    else if (curToken == TOK_RETURN)
    {
        return ParseReturn();
    }
    else if (curIdentifier == "{")
    {
        return ParseBlock();
    }
    else
    {
        std::shared_ptr<StmtAST> stmt = std::make_shared<StmtAST>(STMT_BLANK, std::vector<std::shared_ptr<BaseAST>>{});
        GetNextToken();
        return stmt;
    }
    return nullptr;
}

std::shared_ptr<StmtAST> Parser::ParseIf()
{
    GetNextToken(); //eat if
    std::shared_ptr<BaseAST> child;
    std::vector<std::shared_ptr<BaseAST>> children;
    GetNextToken(); //eat (
    std::shared_ptr<BaseAST> cond = ParseLOrExpr();
    GetNextToken(); //eat )
    if (!cond)
        return nullptr;
    children.emplace_back(cond);
    child = ParseStmt();
    if (!child)
        return nullptr;
    children.emplace_back(child);
    if (curToken == TOK_ELSE)
    {
        GetNextToken(); //eat else
        child = ParseStmt();
        if (!child)
            return nullptr;
        children.emplace_back(child);
    }
    return std::make_shared<StmtAST>(STMT_IF, children);
}

std::shared_ptr<StmtAST> Parser::ParseWhile()
{
    GetNextToken(); //eat while
    std::shared_ptr<BaseAST> child;
    std::vector<std::shared_ptr<BaseAST>> children;
    std::shared_ptr<BaseAST> cond = ParseLOrExpr();
    if (!cond)
        return nullptr;
    children.emplace_back(cond);
    child = ParseStmt();
    if (!child)
        return nullptr;
    children.emplace_back(child);
    return std::make_shared<StmtAST>(STMT_WHILE, children);
}

std::shared_ptr<StmtAST> Parser::ParseBreak()
{
    GetNextToken(); //eat break
    GetNextToken(); //eat ;
    std::vector<std::shared_ptr<BaseAST>> children;
    return std::make_shared<StmtAST>(STMT_BREAK, children);
}

std::shared_ptr<StmtAST> Parser::ParseContinue()
{
    GetNextToken(); //eat continue
    GetNextToken(); //eat ;
    std::vector<std::shared_ptr<BaseAST>> children;
    return std::make_shared<StmtAST>(STMT_CONTINUE, children);
}

std::shared_ptr<StmtAST> Parser::ParseReturn()
{
    GetNextToken(); //eat continue
    std::vector<std::shared_ptr<BaseAST>> children;
    if (curIdentifier != ";")
    {
        std::shared_ptr<BaseAST> child = ParseExpr();
        if (!child)
            return nullptr;
        children.emplace_back(child);
    }
    GetNextToken(); //eat ;
    return std::make_shared<StmtAST>(STMT_RETURN, children);
}

std::vector<std::shared_ptr<VariableAST>> Parser::ParseDefParam()
{
    std::vector<std::shared_ptr<VariableAST>> parameters;
    std::shared_ptr<BaseAST> parameter;
    GetNextToken(); //eat (
    while (curIdentifier != ")")
    {
        parameter = ParseDefinition();
        if (!parameter)
            return parameters;
        parameters.emplace_back(ParseVariable());
        if (curIdentifier == ")")
            break;
        GetNextToken(); //eat ,
    }
    GetNextToken(); //eat )
    return parameters;
}

std::shared_ptr<BaseAST> Parser::ParseExpr()
{
    return ParseAddExpr();
}

std::shared_ptr<BaseAST> Parser::ParseConstExpr()
{
    return ParseAddExpr(); //ident must be const;
}

std::shared_ptr<BaseAST> Parser::ParseAddExpr()
{
    std::shared_ptr<BaseAST> LHS = ParseMulExpr();
    std::shared_ptr<ExprAST> expr;
    if (curIdentifier == "+" || curIdentifier == "-")
    {
        expr = std::make_shared<ExprAST>(curIdentifier);
        GetNextToken(); //eat op;
        expr->LHS = LHS;
        expr->RHS = ParseAddExpr();
        return expr;
    }
    else
    {
        return LHS;
    }
    return nullptr;
}

std::shared_ptr<BaseAST> Parser::ParseMulExpr()
{
    std::shared_ptr<BaseAST> LHS = ParseUnaryExpr();
    std::shared_ptr<ExprAST> expr;
    if (curIdentifier == "*" || curIdentifier == "/" || curIdentifier == "%")
    {
        expr = std::make_shared<ExprAST>(curIdentifier);
        GetNextToken(); //eat op;
        expr->LHS = LHS;
        expr->RHS = ParseMulExpr();
        return expr;
    }
    else
    {
        return LHS;
    }
    return nullptr;
}

std::shared_ptr<BaseAST> Parser::ParseUnaryExpr()
{
    if (curIdentifier == "+" || curIdentifier == "-" || curIdentifier == "!")
    {
        std::shared_ptr<ExprAST> expr = std::make_shared<ExprAST>(curIdentifier);
        GetNextToken(); //eat op;
        expr->LHS = ParseUnaryExpr();
        return expr;
    }
    else if (curToken == TOK_IDENTIFIER)
    {
        if (nextIdentifier == "(")
            return ParseFunctionCall();
    }
    return ParsePrimaryExpr();
}

std::shared_ptr<BaseAST> Parser::ParsePrimaryExpr()
{
    if (curIdentifier == "(")
    {
        GetNextToken(); //eat (
        std::shared_ptr<BaseAST> expr = ParseExpr();
        GetNextToken(); //eat )
        return expr;
    }
    else if (curToken == TOK_NUMBER)
    {
        std::shared_ptr<BaseAST> number = std::make_shared<LiteralAST>(curNumVal);
        GetNextToken(); //eat num;
        return number;
    }
    else
    {
        return ParseLValue();
    }
    return nullptr;
}

std::shared_ptr<FunctionAST> Parser::ParseFunctionCall()
{
    std::string name;
    name = curIdentifier;
    GetNextToken(); //eat name;
    std::vector<std::shared_ptr<VariableAST>> params = ParseCallParam();
    std::shared_ptr<FunctionAST> funcCall = std::make_shared<FunctionAST>(BType::NONE, name, params);
    return funcCall;
}

std::vector<std::shared_ptr<VariableAST>> Parser::ParseCallParam()
{
    std::vector<std::shared_ptr<VariableAST>> params;
    std::shared_ptr<VariableAST> param;
    GetNextToken(); //eat (
    while (curIdentifier != ")")
    {
        std::string name = curIdentifier;
        GetNextToken(); //eat name
        param = std::make_shared<VariableAST>(BType::NONE, name, false);
        params.emplace_back(param);
        if (curIdentifier == ",")
            GetNextToken(); //eat ,
    }
    GetNextToken(); //eat )
    return params;
}

std::shared_ptr<BaseAST> Parser::ParseLOrExpr()
{
    std::shared_ptr<BaseAST> LHS = ParseLAndExpr();
    std::shared_ptr<ExprAST> expr;
    if (curIdentifier == "||")
    {
        expr = std::make_shared<ExprAST>(curIdentifier);
        GetNextToken(); //eat op;
        expr->LHS = LHS;
        expr->RHS = ParseLOrExpr();
        return expr;
    }
    else
    {
        return LHS;
    }
    return nullptr;
}

std::shared_ptr<BaseAST> Parser::ParseLAndExpr()
{
    std::shared_ptr<BaseAST> LHS = ParseEqExpr();
    std::shared_ptr<ExprAST> expr;
    if (curIdentifier == "&&")
    {
        expr = std::make_shared<ExprAST>(curIdentifier);
        GetNextToken(); //eat op;
        expr->LHS = LHS;
        expr->RHS = ParseLAndExpr();
        return expr;
    }
    else
    {
        return LHS;
    }
    return nullptr;
}
std::shared_ptr<BaseAST> Parser::ParseEqExpr()
{
    std::shared_ptr<BaseAST> LHS = ParseRelExpr();
    std::shared_ptr<ExprAST> expr;
    if (curIdentifier == "==" || curIdentifier == "!=")
    {
        expr = std::make_shared<ExprAST>(curIdentifier);
        GetNextToken(); //eat op;
        expr->LHS = LHS;
        expr->RHS = ParseEqExpr();
        return expr;
    }
    else
    {
        return LHS;
    }
    return nullptr;
}
std::shared_ptr<BaseAST> Parser::ParseRelExpr()
{
    std::shared_ptr<BaseAST> LHS = ParseAddExpr();
    std::shared_ptr<ExprAST> expr;
    if (curIdentifier == "<" || curIdentifier == ">" ||
        curIdentifier == "<=" || curIdentifier == ">=")
    {
        expr = std::make_shared<ExprAST>(curIdentifier);
        GetNextToken(); //eat op;
        expr->LHS = LHS;
        expr->RHS = ParseRelExpr();
        return expr;
    }
    else
    {
        return LHS;
    }
    return nullptr;
}

void Parser::LogError()
{
#if PARSER_DEBUG
    std::string indent = "";
    for (int i = 0; i < depth; i++)
        indent += "  ";
    fprintf(stderr, "%sSyntax error\n", indent.c_str());
#endif
}

void Parser::LogParse(const std::string &str)
{
#if PARSER_DEBUG
    std::string indent = "";
    for (int i = 0; i < depth; i++)
        indent += "  ";
    fprintf(stderr, "%s%s\n", indent.c_str(), str.c_str());
#endif
}
