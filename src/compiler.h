#pragma once

#include <string>
#include "paths.h"
#include "tnlib_loader.h"

class CompileOptions {
public:
    bool emitIR = false;
    bool emitAssembly = true;
    bool bindOnly = false;
    ModulePath& modulePath;
    std::string output_file;
    moduleSet& loadedModules;
};

class Compiler {
    CompileOptions& options;
    std::string targetDir = "";
    std::string getModuleName(const char* filepath);
    std::string readFile(const std::string& filename);
public:
    Compiler(CompileOptions& options) : options(options) {};

    void compileSource(const std::string& srccode, std::string modulename = "");
    void compileFile(const std::string& filepath);
};
