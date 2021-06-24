#include "global/global.hpp"
#include "global/type.hpp"
#include "frontend/lexer.hpp"
#include "frontend/parser.hpp"
#include "IR/three_address.hpp"
#include "backend/code_gen.hpp"
#include "backend/baseblock.hpp"
#include "backend/optimize.hpp"

#define LEXER_ONLY false
#define PARSER true
#define TYPECHECK true
#define THREECODE true
#define CODEGEN true


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
#else

#if PARSER
    std::shared_ptr<Parser> parser = std::make_shared<Parser>(filename);
    std::shared_ptr<ProgAST> root = parser->Parse();
#if TYPECHECK
    Visitor *visitor = new Visitor();
    root->Traverse(visitor, SHOW);
    root->Traverse(visitor, ANALYZE);   //type check and construct symtable
#if THREECODE
    root->Traverse(visitor, THREEADDRESS);
    std::shared_ptr<ThreeAddressCode> codes = visitor->threeAddressCode;
    codes->Show();
    //visitor->symtable->Show();
#if CODEGEN
    if(useOptimizr)
    {
        /*
        std::shared_ptr<FlowGraph> flowGraph = std::make_shared<FlowGraph>();
        flowGraph->ConstrctGraph(codes);
        flowGraph->Show();
        */
        std::shared_ptr<Optimizer> optimizer = std::make_shared<Optimizer>(codes, visitor->symtable);
        //optimizer->
        optimizer->RemoveConstVar();
        //optimizer->ShowCode();
        optimizer->WeakenExpr();
        //optimizer->ShowCode();
        optimizer->RemoveTmpVar();
        optimizer->UpdateSymTable();
        optimizer->ShowCode();
        visitor->symtable->Show();
    }
    std::shared_ptr<CodeGener> codeGener = std::make_shared<CodeGener>(outputFilename, codes, visitor->symtable);
    codeGener->GenCode();
#endif  //codegen
#endif  //threecode
#endif  //typecheck
#endif  //parser
#endif  //lexeronly
}