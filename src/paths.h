#pragma once

#include <vector>
#include <string>

class ModulePath{
    std::vector<std::string> tnlibDirs;
public:
    ModulePath() = default;
    void addDirPath(const std::string& path){
        tnlibDirs.push_back(path);
    }
    std::string resolveTnlib(const std::string& moduleName);
    std::string resolveTn(const std::string& moduleName);};
