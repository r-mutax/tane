#pragma once
#include <unordered_set>
#include <string>

#include "paths.h"
#include "symbol.h"

class TnlibLoader
{
    // loaded module paths
    std::unordered_set<std::string> loadedModules;

    // module path resolver
    ModulePath modulePath;

    std::string readfile(const std::string& filepath);
public:
    TnlibLoader(ModulePath& mPath) : modulePath(mPath) {};
    std::vector<Symbol> loadTnlib(const std::string& filepath);
};

