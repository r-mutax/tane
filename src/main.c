#include "tane.h"

int main(int argc, char** argv) {
    if(argc != 2){
        return 1;
    }

    Token* token = tokenize(argv[1]);
    print_tokens(token);
}