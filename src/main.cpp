#include "global.hpp"
#include "lexer.hpp"
#include "type.hpp"
#include "parser.hpp"
#include "three_address.hpp"

#define LEXER_ONLY false
#define PARSER true
#define TYPECHECK true
#define THREECODE true
#define CODEGEN false


std::string filename;
std::string outputFilename = "";
bool useOptimizr = false;

void ParseOptions(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "Usage: " << argv[0] << " [options] input_file" << std::endl;
        exit(0);
    }
    filename = argv[1];
    for(int i = 1; i < argc; i++)
    {
        if(argv[i][0] == '-')
        {
            switch(argv[i][1])
            {
                case 'S':
                //do nothing
                break;
                case 'o':
                    outputFilename = argv[i+1];
                    break;
                case 'O':
                    useOptimizr = true;
            }
        }
    } 
}

int main(int argc, char **argv)
{
    ParseOptions(argc, argv);

#if LEXER_ONLY
    Tok_type tok;
    std::shared_ptr<Lexer> lexer = std::make_shared<Lexer>(filename);
    do
    {
        tok = lexer->GetTok();
        lexer->PrintToken(tok);
    } while (tok != TOK_EOF);
#endif

#if PARSER
    std::shared_ptr<Parser> parser = std::make_shared<Parser>(filename);
    std::shared_ptr<ProgAST> root = parser->Parse();
#if TYPECHECK
    Visitor visitor;
    root->Traverse(visitor, ANALYZE);   //type check and construct symtable
    root->Traverse(visitor, SHOW);
#if THREECODE
    root->Traverse(visitor, THREEADDRESS);
    std::shared_ptr<ThreeAddressCode> codes = visitor.threeAddressCode;
    codes->Show();
#if CODEGEN
    if(useOptimizr)
    {

    }
    lalalaasdfas
#endif  //codegen
#endif  //threecode
#endif  //typecheck
#endif  //parser
}