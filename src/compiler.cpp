#include "compiler.h"
#include "tokenizer.h"
#include "parse.h"
#include "gen_ir.h"
#include "gen_x86-64.h"

#include <fstream>
#include <sstream>

void Compiler::compileSource(const std::string& srccode, std::string modulename) {

    // Tokenize
    Tokenizer tokenizer;
    Tokenizer::TokenStream& ts = tokenizer.scan(const_cast<char*>(srccode.c_str()));

    // Parse
    Parser parser(ts);
    ASTIdx root = parser.parseFile();

    // Generate IR
    IRGenerator irgen(root, parser, options.modulePath, options.loadedModules);
    IRModule& mod = irgen.run();

    if(modulename.empty()) {
        modulename = "module";
    }
    mod.outputSymbols(targetDir, modulename);

    if(options.bindOnly){
        // if bind only, stop here
        return;
    }
    // Emit IR
    X86Generator x86gen(mod);
    x86gen.setOutputFile(options.output_file);
    x86gen.emit();
}

void Compiler::compileFile(const std::string& filepath){
    std::string srccode = readFile(filepath);

    // get file directory
    targetDir = ".";
    size_t lastSlash = filepath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        targetDir = filepath.substr(0, lastSlash);
    }

    compileSource(srccode, getModuleName(filepath.c_str()));
}

std::string Compiler::getModuleName(const char* filepath) {
    std::string pathStr(filepath);
    size_t lastSlash = pathStr.find_last_of("/\\");
    size_t lastDot = pathStr.find_last_of('.');
    if (lastDot == std::string::npos || (lastSlash != std::string::npos && lastDot < lastSlash)) {
        lastDot = pathStr.length();
    }
    size_t start = (lastSlash == std::string::npos) ? 0 : lastSlash + 1;
    return pathStr.substr(start, lastDot - start);
}

std::string Compiler::readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        fprintf(stderr, "Error: Cannot open file '%s'\n", filename.c_str());
        exit(1);
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}
