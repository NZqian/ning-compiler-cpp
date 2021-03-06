#include "lexer.hpp"

Lexer::Lexer(std::string filename)
{
    identifier = "";
    numVal = 0;
    fileReader = std::make_shared<FileReader>(filename);
    curChar = fileReader->GetChar();
    nextChar = fileReader->GetChar();
}

void Lexer::GetNextChar()
{
    curChar = nextChar;
    nextChar = fileReader->GetChar();
}

Tok_type Lexer::GetTok()
{
    identifier = "";
    int state = state_start;
    int token;
    while (state != state_end)
    {
        switch (state)
        {
        case state_start:
        {
            while (curChar == ' ' || curChar == '\n' || curChar == '\r' || curChar == '\0' || curChar == '\t') //skip blank
                GetNextChar();
            if (curChar == EOF)
                return TOK_EOF;
            if (isAlpha(curChar))
            {
                identifier += curChar;
                GetNextChar();
                state = state_id;
                continue;
            }
            if (isNum(curChar))
            {
                if (curChar == '0')
                {
                    identifier += curChar;
                    GetNextChar();
                    if(curChar == '0')
                    {
                        identifier += curChar;
                        GetNextChar();
                        return TOK_UNKNOWN;
                    }
                    else if(curChar == 'X' || curChar == 'x')
                    {
                        identifier += curChar;
                        GetNextChar();
                    }
                }
                state = state_num;
            }
            else if (curChar == '/') //commet
            {
                if (nextChar == '/')
                {
                    //fprintf(stdout, "parsing comment\n");
                    while (curChar != '\r' && curChar != '\n')
                        GetNextChar();
                    continue;
                }
                else if (nextChar == '*')
                {
                    //fprintf(stdout, "parsing comment\n");
                    while (curChar != '*' || nextChar != '/')
                        GetNextChar();
                    GetNextChar();
                    GetNextChar();
                    continue;
                }
                else
                {
                    identifier += curChar;
                    GetNextChar();
                    return TOK_OPERATOR;
                }
            }
            else if(curChar == '=')
            {
                identifier += curChar;
                GetNextChar();
                if(curChar == '=')
                {
                    identifier += curChar;
                    GetNextChar();
                }
                return TOK_OPERATOR;
            }
            else
            {
                switch (curChar)
                {
                case '(':
                    identifier += curChar;
                    GetNextChar();
                    return TOK_SYMBOL;
                case ')':
                    identifier += curChar;
                    GetNextChar();
                    return TOK_SYMBOL;
                case '{':
                    identifier += curChar;
                    GetNextChar();
                    return TOK_SYMBOL;
                case '}':
                    identifier += curChar;
                    GetNextChar();
                    return TOK_SYMBOL;
                case '[':
                    identifier += curChar;
                    GetNextChar();
                    return TOK_SYMBOL;
                case ']':
                    identifier += curChar;
                    GetNextChar();
                    return TOK_SYMBOL;
                case ',':
                    identifier += curChar;
                    GetNextChar();
                    return TOK_SYMBOL;
                case ';':
                    identifier += curChar;
                    GetNextChar();
                    return TOK_SYMBOL;
                case '!':
                    identifier += curChar;
                    GetNextChar();
                    if(curChar == '=')
                    {
                        identifier += curChar;
                        GetNextChar();
                    }
                    return TOK_OPERATOR;
                case '%':
                    identifier += curChar;
                    GetNextChar();
                    return TOK_OPERATOR;
                case '-':
                    identifier += curChar;
                    GetNextChar();
                    return TOK_OPERATOR;
                case '+':
                    identifier += curChar;
                    GetNextChar();
                    return TOK_OPERATOR;
                case '*':
                    identifier += curChar;
                    GetNextChar();
                    return TOK_OPERATOR;
                case '<':
                    identifier += curChar;
                    GetNextChar();
                    if(curChar == '=')
                    {
                        identifier += curChar;
                        GetNextChar();
                    }
                    return TOK_OPERATOR;
                case '>':
                    identifier += curChar;
                    GetNextChar();
                    if(curChar == '=')
                    {
                        identifier += curChar;
                        GetNextChar();
                    }
                    return TOK_OPERATOR;
                case '&':
                    identifier += curChar;
                    GetNextChar();
                    if(curChar == '&')
                    {
                        identifier += curChar;
                        GetNextChar();
                        return TOK_OPERATOR;
                    }
                    return TOK_UNKNOWN;
                case '|':
                    identifier += curChar;
                    GetNextChar();
                    if(curChar == '|')
                    {
                        identifier += curChar;
                        GetNextChar();
                        return TOK_OPERATOR;
                    }
                    return TOK_UNKNOWN;
                default:
                    identifier += curChar;
                    GetNextChar();
                    return TOK_UNKNOWN;
                }
            }
            break;
        }
        case state_id:
        {
            if (isAlpha(curChar) || isNum(curChar))
            {
                identifier += curChar;
                GetNextChar();
            }
            else
            {
                if(identifier == "const")
                    return TOK_CONST;
                if(identifier == "int")
                    return TOK_INT;
                else if(identifier == "void")
                    return TOK_VOID;
                else if(identifier == "if")
                    return TOK_IF;
                else if(identifier == "else")
                    return TOK_ELSE;
                else if(identifier == "while")
                    return TOK_WHILE;
                else if(identifier == "continue")
                    return TOK_CONTINUE;
                else if(identifier == "break")
                    return TOK_BREAK;
                else if(identifier == "return")
                    return TOK_RETURN;
                else
                    return TOK_IDENTIFIER;
                state = state_end;
            }
            break;
        }
        case state_num:
        {
            if(isNum(curChar))
            {
                identifier += curChar;
                GetNextChar();
            }
            else
            {
                numVal = stoi(identifier, 0, 0);    //??????0????????????base???0??????????????????
                //std::cout<<"converting "<<identifier<<" to "<<numVal<<std::endl;
                return TOK_NUMBER;
            }
            break;
        }
        }
    }
    return TOK_UNKNOWN;  //this should never happen
}

void Lexer::PrintToken(Tok_type token)
{
    PrintToken(token, identifier, numVal);
}

void Lexer::PrintToken(Tok_type token, const std::string &id, int num)
{
    /*
    if(token == tok_blank)
        return;
        */
    if(token == TOK_NUMBER)
        fprintf(stdout, "%10s: %d\n", tok2string[token].c_str(), num);
    else
        fprintf(stdout, "%10s: %s\n", tok2string[token].c_str(), id.c_str());
}

FileReader::FileReader(std::string filename)
{
    reader.open(filename);
    lineno = 0;
    linepos = -1;
}

char FileReader::GetChar()
{
    char c;
    if (reader.eof())
    {
        return EOF;
    }
    reader.get(c);
    if (c == '\n')
    {
        lineno++;
        linepos = -1;
    }
    else
    {
        linepos++;
    }
    return c;
}

bool Lexer::isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool Lexer::isNum(char c)
{
    return c >= '0' && c <= '9';
}

Tok_type Lexer::LexError(const std::string &str)
{
    fprintf(stderr, "Lex error %s\n", str.c_str());
    return TOK_UNKNOWN;
}
