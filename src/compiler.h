#pragma once

#include <string>
#include "paths.h"

class CompileOptions {
public:
    bool emitIR = false;
    bool emitAssembly = true;
    ModulePath& modulePath;
    std::string output_file;
};

class Compiler {
    CompileOptions& options;
    std::string getModuleName(const char* filepath);
    std::string readFile(const std::string& filename);
public:
    Compiler(CompileOptions& options) : options(options) {};

    void compileSource(const std::string& srccode, std::string modulename = "");
    void compileFile(const std::string& filepath);
};
