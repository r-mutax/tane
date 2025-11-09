#pragma once
#include <unordered_set>
#include <string>

#include "paths.h"
#include "symbol.h"

using moduleSet = std::unordered_set<std::string>;

class TnlibLoader
{
    // loaded module paths
    moduleSet& loadedModules;

    // module path resolver
    ModulePath modulePath;

    std::string readfile(const std::string& filepath);
public:
    TnlibLoader(ModulePath mPath, moduleSet& loadedModules) : loadedModules(loadedModules), modulePath(mPath) {}
    std::vector<Symbol> loadTnlib(const std::string& moduleName);
};

