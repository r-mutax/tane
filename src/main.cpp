#include "tane.hpp"
#include "compiler.h"
#include "tnlib_loader.h"

#include <fstream>
#include <sstream>

void printUsage(const char* progName) {
    fprintf(stderr, "Usage: %s [options] <input>\n", progName);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "  -c <code>     : Compile code string directly\n");
    fprintf(stderr, "  -o <output.s> : Output assembly file (default: out.s)\n");
    fprintf(stderr, "  -i <tnlibdir>: Specify tnlib directory\n");
    fprintf(stderr, "Examples:\n");
    fprintf(stderr, "  %s source.tn              # Compile file\n", progName);
    fprintf(stderr, "  %s -c \"fn main() {...}\"   # Compile string\n", progName);
}

int main(int argc, char** argv) {
    if(argc < 2){
        printUsage(argv[0]);
        return 1;
    }

    const char* inputFile = nullptr;
    const char* codeString = nullptr;
    const char* outputFile = "out.s";

    ModulePath modulePath;
    modulePath.addDirPath("."); // current directory
    modulePath.addDirPath("/workspaces/tane/std/lib");

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
        } else if(strcmp(argv[i], "-i") == 0){
            if(i + 1 >= argc){
                fprintf(stderr, "Error: -i requires an argument\n");
                printUsage(argv[0]);
                return 1;
            }
            modulePath.addDirPath(argv[++i]);
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

    moduleSet loadedModules;

    CompileOptions options{
        .modulePath = modulePath,
        .output_file = std::string(outputFile),
        .loadedModules = loadedModules
    };

    Compiler compiler(options);

    if(codeString != nullptr){
        compiler.compileSource(codeString);
    } else {
        compiler.compileFile(inputFile);
    }

    return 0;
}