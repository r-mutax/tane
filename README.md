# tane

[![Tests](https://github.com/r-mutax/tane/actions/workflows/test.yml/badge.svg)](https://github.com/r-mutax/tane/actions/workflows/test.yml)

## Introduction

Tane is a small programming language that aims for self-hosting. Currently, the Tane compiler receives a code segment and outputs an x86-64 assembly file (out.s).

### Currently Supported Features

- **Expressions**: Arithmetic operators (`+` `-` `*` `/` `%`), bitwise operators (`&` `|` `^` `<<` `>>`), comparison operators (`==` `!=` `<` `<=`), logical operators (`&&` `||`), switch expressions
- **Statements**: Variable declaration (`let`/`mut`), assignment, `return`, compound statements, `if`/`else`
- **Variables**: Local variables are currently 8 bytes each and allocated on the stack (first at `[rbp - 8]`, second at `[rbp - 16]`, ...)

### Roadmap

- Add `break` / `continue` statements
- Function definition and invocation, type system expansion

The EBNF below shows the minimal core specification (being updated incrementally).

# EBNF (Current Implementation)

```
/* ---- Top Level ---- */
file            := '{' stmts '}' ;

/* ---- Statements ---- */
block           := '{' stmts '}' ;
stmts           := stmt* ;
stmt            := let-stmt 
                 | assign-stmt 
                 | if-stmt
                 | while-stmt
                 | block
                 | return-stmt 
                 | expr-stmt ;

let-stmt        := 'let' 'mut'? ident-name ';' ;
assign-stmt     := ident-name '=' expr ';' ;
return-stmt     := 'return' expr? ';' ;
expr-stmt       := expr ';' ;

if-stmt         := 'if' expr block else-clause? ;
else-clause     := 'else' ( if-stmt | block ) ;

while-stmt      := 'while' expr block ;

/* ---- Expressions ---- */
expr            := logical_or ;

logical_or      := logical_and ( '||' logical_and )* ;
logical_and     := bitwise_or ( '&&' bitwise_or )* ;
bitwise_or      := bitwise_xor ( '|' bitwise_xor )* ;
bitwise_xor     := bitwise_and ( '^' bitwise_and )* ;
bitwise_and     := equality ( '&' equality )* ;

equality        := relational ( ('==' | '!=') relational )* ;
relational      := shift ( ('<' | '<=') shift )* ;

shift           := additive ( ('<<' | '>>') additive )* ;
additive        := multiplicative ( ('+' | '-') multiplicative )* ;
multiplicative  := unary ( ('*' | '/' | '%') unary )* ;

unary           := ('+' | '-') unary
                 | primary ;

primary         := int-lit
                 | ident-name
                 | '(' expr ')'
                 | switch-expr ;

switch-expr     := 'switch' expr '{' switch-arm* '}' ;
switch-arm      := expr '=>' expr ','? ;

/* ---- Lexical ---- */
ident-name      := /[A-Za-z_][A-Za-z0-9_]*/ ;
int-lit         := '0' | /[1-9][0-9_]*/ ;
WS              := ( ' ' | '\t' | '\r' | '\n' )+ ; 
```

**Note**: Function definitions, type annotations, and many advanced features shown above are planned but not yet implemented. The grammar reflects the current working subset.