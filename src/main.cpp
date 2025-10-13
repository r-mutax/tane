#include "tane.hpp"

int main(int argc, char** argv) {
    if(argc != 2){
        return 1;
    }

    Tokenizer tokenizer;
    Tokenizer::TokenStream& ts = tokenizer.scan(argv[1]);
    tokenizer.printTokens();

    Parser parser(ts);
    ASTIdx root = parser.parseFile();
    parser.printAST(root);

    IRGenerator irgen(root, parser);
    IRModule& mod = irgen.run();

    X86Generator x86gen(mod);
    x86gen.setOutputFile("out.s");
    x86gen.emit();

    return 0;
}