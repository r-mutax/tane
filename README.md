# tane

[![Tests](https://github.com/r-mutax/tane/actions/workflows/test.yml/badge.svg)](https://github.com/r-mutax/tane/actions/workflows/test.yml)

Tane is a small programming language developed as a challenge to achieve self-hosting.

# ENBF
```
file            := top-decl* ;

top-decl        := pub? ( let-decl | fn-decl ) ;
pub             := 'pub' ;

let-decl        := 'mut' 'let' ident-name ':' type-spec ';' ;

fn-decl         := 'fn' ident-name '(' params? ')' return-type block ;
params          := param (',' param)* (',')? ;
param           := ident-name ':' type-spec ;
return-type     := ('->' type-spec)? ;

type-spec       := 'i8' | 'i16' | 'i32' | 'i64' | 'u8' | 'u16' | 'u32' | 'u64' ;

block           := '{' stmts '}' ;
stmts           := stmt* ;
stmt            := let-stmt | assign-stmt | expr-stmt | return-stmt ;

let-stmt        := 'mut' 'let' ident-name ':' type-spec ';' ;
assign-stmt     := lvalue '=' expr ';' ;
expr-stmt       := expr ';' ;
return-stmt     := 'return' expr? ';' ;

lvalue          := ident-name ;

/* ---- expressions ---- */
expr            := logical_or ;

logical_or      := logical_and        ( '||' logical_and )* ;
logical_and     := bitwise_or         ( '&&' bitwise_or )* ;
bitwise_or      := bitwise_xor        ( '|'  bitwise_xor )* ;
bitwise_xor     := bitwise_and        ( '^'  bitwise_and )* ;
bitwise_and     := equality           ( '&'  equality )* ;

equality        := relational         ( ('==' | '!=') relational )* ;
relational      := shift              ( ('<' | '<=' | '>' | '>=') shift )* ;

shift           := additive           ( ('<<' | '>>') additive )* ;
additive        := multiplicative     ( ('+' | '-') multiplicative )* ;
multiplicative  := unary              ( ('*' | '/' | '%') unary )* ;

unary           := postfix
                 | ('+' | '-' | '!' | '~') unary ;

postfix         := primary ( call_suffix )* ;
call_suffix     := '(' args? ')' ;
args            := expr (',' expr)* (',')? ;

primary         := ident-name
                 | int-lit
                 | '(' expr ')' ;

/* ---- lexicals ---- */
ident-name      := /[A-Za-z_][A-Za-z0-9_]*/;
int-lit         := '0' | /[1-9][0-9_]*/ ;
WS              := ( ' ' | '\t' | '\r' | '\n' )+ ; 
LINE_COMMENT    := '//' [^\n]* ;
```