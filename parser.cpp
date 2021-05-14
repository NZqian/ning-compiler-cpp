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
    return nullptr;
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
        std::string type, name;
        type = curIdentifier;
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

std::shared_ptr<FunctionAST> Parser::ParseFunction(const std::string &returnType, const std::string &name)
{
    std::vector<std::shared_ptr<VariableAST>> parameters = ParseParameter();
    if (curIdentifier == "{")
    {
        std::shared_ptr<BlockAST> body = ParseBlock();
        std::cout << "parsed function definition" << std::endl;
        return std::make_shared<FunctionAST>(returnType, name,
                                             std::move(parameters), std::move(body));
    }
    else
    {
        GetNextToken(); //eat ;
        std::cout << "parsed function decleartion" << std::endl;
        return std::make_shared<FunctionAST>(returnType, name, std::move(parameters));
    }
}
std::shared_ptr<VariableAST> Parser::ParseVariable(const std::string &type, const std::string &name, bool isConst)
{
    std::vector<int> dimensions;
    while (curIdentifier == "[") //数组
    {
        GetNextToken(); //eat [
        dimensions.emplace_back(curNumVal);
        GetNextToken(); //eat num
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
    std::cout<<"parsed variable "<<name<<std::endl;
    return variable;
}
std::shared_ptr<VariableAST> Parser::ParseVariable()
{
    bool isConst = false;
    std::string name, type;
    if (curToken == TOK_CONTINUE)
    {
        isConst = true;
        GetNextToken();
    }
    type = curIdentifier;
    GetNextToken();
    name = curIdentifier;
    GetNextToken();
    return ParseVariable(type, name, isConst);
}

std::shared_ptr<VariableAST> Parser::ParseLValue()
{
    std::string name;
    name = curIdentifier;
    GetNextToken();
    std::vector<int> dimensions;
    while (curIdentifier == "[") //数组
    {
        GetNextToken(); //eat [
        dimensions.emplace_back(curNumVal);
        GetNextToken(); //eat num
        GetNextToken(); //eat ]
    }
    return std::make_shared<VariableAST>("", name, false, dimensions);
}

std::shared_ptr<BaseAST> Parser::ParseStmt()
{
    if (curToken == TOK_IDENTIFIER)
    {
        std::shared_ptr<VariableAST> lval = ParseLValue();
        GetNextToken(); //eat =
        std::shared_ptr<ExprAST> expr = ParseExpr();
        GetNextToken(); //eat ;
        std::vector<std::shared_ptr<BaseAST>> children{lval, expr};
        std::cout<<"parsed assign"<<std::endl;
        return std::make_shared<StmtAST>(STMT_ASSIGN, children);
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
    else if(curIdentifier == "{")
    {
        return ParseBlock();
    }
    else if(curIdentifier == ";")
    {
        return std::make_shared<StmtAST>(STMT_BLANK);
    }
    return nullptr;
}

std::shared_ptr<StmtAST> Parser::ParseIf()
{
    return nullptr;
}

std::shared_ptr<StmtAST> Parser::ParseWhile()
{

    return nullptr;
}
std::shared_ptr<StmtAST> Parser::ParseBreak()
{

    return nullptr;
}
std::shared_ptr<StmtAST> Parser::ParseContinue()
{

    return nullptr;
}
std::shared_ptr<StmtAST> Parser::ParseReturn()
{

    return nullptr;
}

std::vector<std::shared_ptr<VariableAST>> Parser::ParseParameter()
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
    return std::move(parameters);
}

std::shared_ptr<ExprAST> Parser::ParseExpr()
{
    return ParseAddExpr();
}

std::shared_ptr<ExprAST> Parser::ParseConstExpr()
{
    return ParseAddExpr();  //ident must be const;
}

std::shared_ptr<ExprAST> Parser::ParseAddExpr()
{
    
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

/*
std::shared_ptr<BaseAST> Parser::ParseBody()
{
    GetNextToken(); //eat {
    while (curToken == tok_type)
    {
        ParseDecl();
    }
    while(curToken != tok_rbrace)
    {
        ParseStmt();
    }

    GetNextToken(); //eat }
    return nullptr;
}
std::shared_ptr<BaseAST> Parser::ParseDecl()
{
    LogParse("Parsing decl");
    depth++;

    std::string type, name;
    bool isArray = false;
    bool isPointer = false;
    int size = 0;
    int integer = 0;
    bool boolean = 0;
    char character = '\0';
    char *str = NULL;

    type = curIdentifier;
    GetNextToken(); //eat type
    if (curIdentifier == "*")
    {
        isPointer = true;
        GetNextToken(); //eat *
    }
    name = curIdentifier;
    GetNextToken(); //eat name
    if (curToken == tok_lbracket)
    {
        isArray = true;
        GetNextToken(); //eat [
        size = lexer->numVal;
        LogParse("VecSize: " + curIdentifier);
        GetNextToken(); //eat num
        GetNextToken(); //eat ]
    }
    if (curIdentifier == "=")
    {
        GetNextToken(); //eat =
        if (curToken == tok_number)
        {
            if (type != "int")
                LogError();
            integer = curNumVal;
            depth--;
            LogParse("Parsed decl with val");
            GetNextToken(); //eat val
            GetNextToken(); //eat semi
            return std::make_shared<VariableAST>(type, name, integer);
        }
        else if (curToken == tok_char)
        {
            if (type != "char")
                LogError();
            character = curIdentifier[0];
            depth--;
            LogParse("Parsed decl with val");
            GetNextToken(); //eat val
            GetNextToken(); //eat semi
            return std::make_shared<VariableAST>(type, name, character);
        }
        else if (curToken == tok_tf)
        {
            if (type != "bool")
                LogError();
            if (curIdentifier == "true")
                boolean = true;
            else
                boolean = false;
            depth--;
            LogParse("Parsed decl with val");
            GetNextToken(); //eat val
            GetNextToken(); //eat semi
            return std::make_shared<VariableAST>(type, name, boolean);
        }
        else if (curToken == tok_str)
        {
            if (type != "string")
                LogError();
            depth--;
            LogParse("Parsed decl with val");
            GetNextToken(); //eat val
            GetNextToken(); //eat semi
            return std::make_shared<VariableAST>(type, name, curIdentifier);
        }
    }
    else
    {
        depth--;
        LogParse("Parsed decl");
        GetNextToken(); //eat semi
        return std::make_shared<VariableAST>(type, name);
    }
    GetNextToken(); //eat semi
    depth--;
    LogParse("Parsed decl");
    return nullptr;
}



std::shared_ptr<BaseAST> Parser::ParseExpr()
{
    return nullptr;
}

std::shared_ptr<BaseAST> Parser::ParseSimple()
{
    return nullptr;
}


void ProgAST::Traverse(int depth)
{
    std::cout << "Traversing program" << std::endl;
    std::cout << "size " << children.size() << std::endl;
    for (int i = 0; i < children.size(); i++)
    {
        children[i]->Traverse(depth + 1);
    }
}

std::shared_ptr<StmtAST> Parser::ParseStmt()
{
    LogParse("Parsing Stmt");
    depth++;
    std::string type = "unknown";
    std::shared_ptr<StmtAST> stmt = nullptr;
    std::shared_ptr<BaseAST> child;
    std::vector<std::shared_ptr<BaseAST>> children;
    if (curIdentifier == "if")
    {
        type = "if";
        GetNextToken(); //eat if
        GetNextToken(); //eat (
        child = ParseExpr();
        if(child)
            children.emplace_back(std::move(child));
        GetNextToken(); //eat )
        child = ParseStmt();
        if(child)
            children.emplace_back(std::move(child));
        if(curIdentifier == "else")
        {
            child = ParseStmt();
            if(child)
                children.emplace_back(std::move(child));
        }
        stmt = std::make_shared<StmtAST>(type, children);
    }
    else if (curIdentifier == "while")
    {
        type = "while";
        GetNextToken(); //eat while
        GetNextToken(); //eat (
        child = ParseExpr();
        if(child)
            children.emplace_back(std::move(child));
        GetNextToken(); //eat )
        child = ParseStmt();
        if(child)
            children.emplace_back(std::move(child));
        stmt = std::make_shared<StmtAST>(type, children);
    }
    else if (curIdentifier == "for")
    {
        type = "for";
        GetNextToken(); //eat for
        GetNextToken(); //eat (
        child = ParseSimple();
        if(child)
            children.emplace_back(std::move(child));
        GetNextToken(); //eat ;
        child = ParseExpr();
        if(child)
            children.emplace_back(std::move(child));
        GetNextToken(); //eat ;
        child = ParseSimple();
        if(child)
            children.emplace_back(std::move(child));
        GetNextToken(); //eat )
        if(curIdentifier == "else")
        {
            child = ParseExpr();
            if(child)
                children.emplace_back(std::move(child));
        }
        stmt = std::make_shared<StmtAST>(type, children);
    }
    else if (curIdentifier == "continue")
    {
        type = "continue";
        GetNextToken(); //eat continue
        GetNextToken(); //eat ;
        stmt = std::make_shared<StmtAST>(type);
    }
    else if (curIdentifier == "break")
    {
        type = "break";
        GetNextToken(); //eat break;
        GetNextToken(); //eat ;
        stmt = std::make_shared<StmtAST>(type);
    }
    else if (curIdentifier == "return")
    {
        type = "return";
        GetNextToken(); //eat return
        if(curToken != tok_semi)
        {
            child = ParseExpr();
            if(child)
                children.emplace_back(std::move(child));
        }
        GetNextToken(); //eat ;
        stmt = std::make_shared<StmtAST>(type, children);
    }
    else if (curToken == tok_lbrace)
    {
        type = "body";
        GetNextToken(); //eat {
        child = ParseBody();
        if(child)
            children.emplace_back(child);
        GetNextToken(); //eat }
        stmt = std::make_shared<StmtAST>(type, children);
    }
    else if (curIdentifier == "assert")
    {
        type = "assert";
        GetNextToken(); //eat assert
        GetNextToken(); //eat (
        child = ParseExpr();
        if(child)
            children.emplace_back(std::move(child));
        GetNextToken(); //eat ,
        child = ParseExpr();
        if(child)
            children.emplace_back(std::move(child));
        GetNextToken(); //eat )
        GetNextToken(); //eat ;
        stmt = std::make_shared<StmtAST>(type, children);
    }
    else if (curToken == tok_semi)
    {
        LogParse("Empty Stmt");
    }
    depth--;
    LogParse("Parsed Stmt");
    return stmt;
}
*/