#pragma once
#include <string_view>
#include <string>
#include <cstdio>
#include <iostream>
#include <format>

/// Output context interface
class OutputContext{
public:
    virtual ~OutputContext() {}
    virtual void write(std::string_view str) = 0;
    virtual void flush() = 0;
};

/// Standard output context
class StdioContext : public OutputContext{
public:
    void write(std::string_view str) override {
        std::cout << str;
    }
    void flush() override {
        std::cout.flush();
    }
};

/// @brief File output context
class FileContext : public OutputContext{
    FILE* fp;
public:
    FileContext(const std::string& filename) : fp(nullptr) {
        fp = fopen(filename.c_str(), "w");
        if(!fp) {
            fprintf(stderr, "Cannot open file: %s\n", filename.c_str());
            exit(1);
        }
    }
    void write(std::string_view str) override {
        if(fp) {
            fputs(str.data(), fp);
        }
    }
    void flush() override {
        if(fp) {
            fflush(fp);
        }
    }
};

class NullContext : public OutputContext{
public:
    void write(std::string_view) override {}
    void flush() override {}
};

class Output {
    OutputContext* ctx;
public:
    Output(OutputContext* ctx_) : ctx(ctx_) {}
    Output() : ctx(new NullContext()) {}
    template <class... Ts>
    void print(std::format_string<Ts...> fmt, Ts&&... args) {
        std::string buf;
        buf.reserve(128);
        std::format_to(std::back_inserter(buf), fmt, std::forward<Ts>(args)...);
        ctx->write(buf);
    }
    void flush() {
        ctx->flush();
    }
    void setFileContext(const std::string& filename) {
        delete ctx;
        ctx = new FileContext(filename.c_str());
    }
    void setStdioContext() {
        delete ctx;
        ctx = new StdioContext();
    }
};

