#pragma once
#include "../global/global.hpp"
#include "../global/type.hpp"


class FileReader
{
    std::fstream reader;

public:
    int lineno, linepos;
    FileReader(std::string filename);
    char GetChar();
};

class Lexer
{
    std::shared_ptr<FileReader> fileReader;
    std::set<std::string> reservedWord;
    std::set<std::string> typeWord;
    void GetNextChar(); //右移curChar与nextChar
    bool isAlpha(char c);
    bool isNum(char c);
public:
    char curChar;
    char nextChar;  //look ahead
    std::string identifier;
    double numVal;
    Lexer(std::string filename);
    Tok_type GetTok();
    void PrintToken(Tok_type token);
    void PrintToken(Tok_type token, const std::string &id, int num);
    Tok_type LexError(const std::string &str);
};
