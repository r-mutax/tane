#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "common_type.h"

enum class PhysReg : uint8_t { None, R10, R11, R12, R13, R14, R15, RAX, RDI, RSI, RDX, RCX, R8, R9 };
enum class VRegKind : uint8_t { Temp, Imm, LVarAddr };

class VReg{
public:
    VRegKind kind = VRegKind::Temp;
    int32_t val;
    PhysReg assigned = PhysReg::None;
};

enum class SymbolKind : uint8_t { Variable, Function };

// Symbol のフラグ
enum class SymbolFlags : uint8_t {
    None     = 0,
    Mutable  = 1 << 0,  // let mut
    Public   = 1 << 1,  // pub fn
    External = 1 << 2,  // import された関数
};

inline SymbolFlags operator|(SymbolFlags a, SymbolFlags b) {
    return static_cast<SymbolFlags>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

inline SymbolFlags operator&(SymbolFlags a, SymbolFlags b) {
    return static_cast<SymbolFlags>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

inline bool hasFlag(SymbolFlags flags, SymbolFlags flag) {
    return (flags & flag) != SymbolFlags::None;
}

class Symbol{
public:
    SymbolKind kind = SymbolKind::Variable;
    std::string name;
    TokenIdx tokenIdx;
    SymbolFlags flags = SymbolFlags::None;
    uint32_t stackOffset = 0;
    std::vector<SymbolIdx> params;
    
    // ヘルパーメソッド
    bool isMut() const { return hasFlag(flags, SymbolFlags::Mutable); }
    bool isPub() const { return hasFlag(flags, SymbolFlags::Public); }
    
    void setMut(bool is_mut) {
        if(is_mut) {
            flags = flags | SymbolFlags::Mutable;
        } else {
            flags = static_cast<SymbolFlags>(static_cast<uint8_t>(flags) & ~static_cast<uint8_t>(SymbolFlags::Mutable));
        }
    }
    void setPub(bool is_pub) {
        if(is_pub) {
            flags = flags | SymbolFlags::Public;
        } else {
            flags = static_cast<SymbolFlags>(static_cast<uint8_t>(flags) & ~static_cast<uint8_t>(SymbolFlags::Public));
        }
    }
};

