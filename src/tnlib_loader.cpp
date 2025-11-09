#include "tnlib_loader.h"
#include "tokenizer.h"
#include "compiler.h"

std::vector<Symbol> TnlibLoader::loadTnlib(const std::string& moduleName)
{
    // Check if the module has already been loaded
    if (loadedModules.count(moduleName))
    {
        return {};  // Return an empty vector if already loaded
    }

    std::string fullPath = modulePath.resolveTnlib(moduleName);
    if (fullPath == "")
    {
        fullPath = modulePath.resolveTn(moduleName);
        if (fullPath == "")
        {
            fprintf(stderr, "Failed to resolve tnlib file: %s\n", moduleName.c_str());
            exit(1);
        }

        // try compile .tnlib from .tn
        CompileOptions comp {
            .emitIR = false,
            .emitAssembly = false,
            .bindOnly = true,
            .modulePath = modulePath,
            .output_file = "",
            .loadedModules = loadedModules
        };
        Compiler compiler(comp);
        compiler.compileFile(fullPath);
        fullPath = modulePath.resolveTnlib(moduleName);
    }
    std::string content = readfile(fullPath);

    Tokenizer tokenizer(true);
    Tokenizer::TokenStream& ts = tokenizer.scan(const_cast<char*>(content.c_str()) );

    // Load the tnlib file
    std::vector<Symbol> symbols;

    ts.expect(TokenKind::Tnlib);
    ts.expect(TokenKind::Num);  // version number

    ts.expect(TokenKind::Module);
    ts.expectIdent();

    while(!ts.consume(TokenKind::End)){
        ts.expect(TokenKind::Fn);
        TokenIdx fnIdent = ts.expectIdent();
        ts.expect(TokenKind::LParen);

        Symbol fnSym;
        Token fnToken = ts.getToken(fnIdent);
        fnSym.name = std::string(fnToken.pos, fnToken.len);
        fnSym.kind = SymbolKind::Function;
        fnSym.setMut(false);

        if(!ts.peekKind(TokenKind::RParen)){
            do {
                TokenIdx paramIdent = ts.expectIdent();
                Token pToken = ts.getToken(paramIdent);

                Symbol paramSym;
                paramSym.name = std::string(pToken.pos, pToken.len);
                paramSym.kind = SymbolKind::Variable;
                paramSym.setMut(false);

                SymbolIdx paramIdx = static_cast<SymbolIdx>(symbols.size());
                symbols.push_back(paramSym);
                fnSym.params.push_back(paramIdx);
            } while(ts.consume(TokenKind::Comma));
        }
        ts.expect(TokenKind::RParen);
        ts.expect(TokenKind::Semicolon);

        symbols.push_back(fnSym);
    }

    loadedModules.insert(moduleName);
    return symbols;
}

std::string TnlibLoader::readfile(const std::string& filepath)
{
    FILE* fp = fopen(filepath.c_str(), "r");
    if (!fp)
    {
        fprintf(stderr, "Cannot open file: %s\n", filepath.c_str());
        exit(1);
    }

    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    std::string content;
    content.resize(fsize);
    fread(content.data(), 1, fsize, fp);
    fclose(fp);

    return content;
}