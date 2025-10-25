#include "tane.hpp"
#include <fstream>
#include <sstream>

void printUsage(const char* progName) {
    fprintf(stderr, "Usage: %s [options] <input>\n", progName);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -c <code>     : Compile code string directly\n");
    fprintf(stderr, "  -o <output.s> : Output assembly file (default: out.s)\n");
    fprintf(stderr, "Examples:\n");
    fprintf(stderr, "  %s source.tn              # Compile file\n", progName);
    fprintf(stderr, "  %s -c \"fn main() {...}\"   # Compile string\n", progName);
}

std::string readFile(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename);
        exit(1);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string getModuleName(const char* filepath) {
    std::string pathStr(filepath);
    size_t lastSlash = pathStr.find_last_of("/\\");
    size_t lastDot = pathStr.find_last_of('.');
    if (lastDot == std::string::npos || (lastSlash != std::string::npos && lastDot < lastSlash)) {
        lastDot = pathStr.length();
    }
    size_t start = (lastSlash == std::string::npos) ? 0 : lastSlash + 1;
    return pathStr.substr(start, lastDot - start);
}

int main(int argc, char** argv) {
    if(argc < 2){
        printUsage(argv[0]);
        return 1;
    }

    const char* inputFile = nullptr;
    const char* codeString = nullptr;
    const char* outputFile = "out.s";

    // Parse arguments
    for(int i = 1; i < argc; i++){
        if(strcmp(argv[i], "-o") == 0){
            if(i + 1 >= argc){
                fprintf(stderr, "Error: -o requires an argument\n");
                printUsage(argv[0]);
                return 1;
            }
            outputFile = argv[++i];
        } else if(strcmp(argv[i], "-c") == 0){
            if(i + 1 >= argc){
                fprintf(stderr, "Error: -c requires an argument\n");
                printUsage(argv[0]);
                return 1;
            }
            codeString = argv[++i];
        } else if(argv[i][0] == '-'){
            fprintf(stderr, "Error: Unknown option '%s'\n", argv[i]);
            printUsage(argv[0]);
            return 1;
        } else {
            if(inputFile != nullptr){
                fprintf(stderr, "Error: Multiple input files specified\n");
                printUsage(argv[0]);
                return 1;
            }
            inputFile = argv[i];
        }
    }

    // Check input source
    if(inputFile == nullptr && codeString == nullptr){
        fprintf(stderr, "Error: No input specified\n");
        printUsage(argv[0]);
        return 1;
    }

    if(inputFile != nullptr && codeString != nullptr){
        fprintf(stderr, "Error: Cannot specify both file and code string\n");
        printUsage(argv[0]);
        return 1;
    }

    // Get source code
    std::string source;
    if(codeString != nullptr){
        source = codeString;
    } else {
        source = readFile(inputFile);
    }
    
    // Tokenize
    Tokenizer tokenizer;
    Tokenizer::TokenStream& ts = tokenizer.scan(const_cast<char*>(source.c_str()));

    // Parse
    Parser parser(ts);
    ASTIdx root = parser.parseFile();

    // Generate IR
    IRGenerator irgen(root, parser);
    IRModule& mod = irgen.run();

    // get module name
    std::string moduleName;
    if(inputFile != nullptr){
        moduleName = getModuleName(inputFile);
    } else {
        moduleName = "module";
    }
    mod.outputSymbols(moduleName);

    // Generate assembly
    X86Generator x86gen(mod);
    x86gen.setOutputFile(outputFile);
    x86gen.emit();

    return 0;
}