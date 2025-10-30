#include "tane.hpp"

std::vector<Symbol> TnlibLoader::loadTnlib(const std::string& filepath)
{
    // Check if the module has already been loaded
    if (loadedModules.count(filepath))
    {
        return {};  // Return an empty vector if already loaded
    }

    std::string fullPath = modulePath.resolve(filepath);
    if (fullPath == "")
    {
        fprintf(stderr, "Failed to resolve tnlib file: %s\n", filepath.c_str());
        exit(1);
    }

    std::string content = readfile(fullPath);
    Tokenizer tokenizer(true);
    Tokenizer::TokenStream& ts = tokenizer.scan(const_cast<char*>(content.c_str()) );

    // Load the tnlib file
    std::vector<Symbol> symbols;

    ts.expect(TokenKind::Tnlib);
    ts.expect(TokenKind::Num);  // version number

    ts.expect(TokenKind::Module);
    TokenIdx moduleIdent = ts.expectIdent();

    while(!ts.consume(TokenKind::End)){
        ts.expect(TokenKind::Fn);
        TokenIdx fnIdent = ts.expectIdent();
        ts.expect(TokenKind::LParen);

        Symbol fnSym;
        Token fnToken = ts.getToken(fnIdent);
        fnSym.name = std::string(fnToken.pos, fnToken.len);
        fnSym.kind = SymbolKind::Function;
        fnSym.isMut = false;

        if(!ts.peekKind(TokenKind::RParen)){
            do {
                TokenIdx paramIdent = ts.expectIdent();
                Token pToken = ts.getToken(paramIdent);

                Symbol paramSym;
                paramSym.name = std::string(pToken.pos, pToken.len);
                paramSym.kind = SymbolKind::Variable;
                paramSym.isMut = false;

                SymbolIdx paramIdx = static_cast<SymbolIdx>(symbols.size());
                symbols.push_back(paramSym);
                fnSym.params.push_back(paramIdx);
            } while(ts.consume(TokenKind::Comma));
        }
        ts.expect(TokenKind::RParen);

        symbols.push_back(fnSym);
    }

    loadedModules.insert(filepath);
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