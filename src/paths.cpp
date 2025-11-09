#include "paths.h"

std::string ModulePath::resolveTnlib(const std::string& moduleName){
    for(const auto& dir : tnlibDirs){
        std::string fullPath = dir + "/" + moduleName + ".tnlib";
        FILE* f = fopen(fullPath.c_str(), "r");
        if(f != nullptr){
            fclose(f);
            return fullPath;
        }
    }
    return "";
}

std::string ModulePath::resolveTn(const std::string& moduleName){
    for(const auto& dir : tnlibDirs){
        std::string fullPath = dir + "/" + moduleName + ".tn";
        FILE* f = fopen(fullPath.c_str(), "r");
        if(f != nullptr){
            fclose(f);
            return fullPath;
        }
    }
    return "";
}
