#include "global.hpp"
#include "lexer.hpp"
#include "type.hpp"
#include "parser.hpp"

#define LEXER_ONLY flase
#define PARSER_ONLY true

int main(int argc, char **argv)
{
    std::string filename;
    if (argc < 2)
    {
        std::cout << "input file name" << std::endl;
    }
    else
    {
        filename = argv[1];
        std::cout << filename << std::endl;
    }

#if LEXER_ONLY
    Tok_type tok;
    std::shared_ptr<Lexer> lexer = std::make_shared<Lexer>(filename);
    do
    {
        tok = lexer->GetTok();
        lexer->PrintToken(tok);
    } while (tok != TOK_EOF);
#endif

#if PARSER_ONLY
    std::shared_ptr<Parser> parser = std::make_shared<Parser>(filename);
    std::shared_ptr<ProgAST> root = parser->Parse();
    root->Traverse(0);
#endif
}