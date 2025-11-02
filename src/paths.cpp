#include "paths.h"

std::string ModulePath::resolve(const std::string& moduleName){
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
