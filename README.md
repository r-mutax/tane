# tane

[![Tests](https://github.com/r-mutax/tane/actions/workflows/test.yml/badge.svg)](https://github.com/r-mutax/tane/actions/workflows/test.yml)

## Introduction

Tane is a small programming language that aims for self-hosting. Currently, the Tane compiler receives a code segment and outputs an x86-64 assembly file.

### Currently Supported Features

- **Expressions**: Arithmetic operators (`+` `-` `*` `/` `%`), bitwise operators (`&` `|` `^` `<<` `>>`), comparison operators (`==` `!=` `<` `<=`), logical operators (`&&` `||`), switch expressions
- **Statements**: Variable declaration (`let`/`mut`), assignment, `return`, compound statements, `if`/`else`, `while`
- **Functions**: Function definitions with parameters (up to 6 arguments), function calls with arguments, `return` statements
- **Variables**: Local variables are currently 8 bytes each and allocated on the stack (first at `[rbp - 8]`, second at `[rbp - 16]`, ...)
- **Strings**: String literals (`"hello world"`) supported
- **Modules**: Import system (`import <module>;`) for external libraries
- **Standard Library**: Basic I/O functions (`print()`, `exit()`)

## Getting Started

### Building

```bash
make
```

### Usage

The Tane compiler can compile source code from either a file or a command-line string.

#### Compile from file

```bash
./build/tane <source.tn>           # Output to out.s
./build/tane <source.tn> -o <out>  # Specify output file
```

**Example:**
```bash
./build/tane example.tn
gcc -o program out.s
./program
```

#### Compile from command-line string

```bash
./build/tane -c "<code>"           # Output to out.s
./build/tane -c "<code>" -o <out>  # Specify output file
```

**Example:**
```bash
./build/tane -c "fn main() { return 42; }"
gcc -o program out.s
./program
echo $?  # Outputs: 42
```

#### Options

- `-c <code>`: Compile code string directly (alternative to file input)
- `-o <file>`: Specify output assembly file (default: `out.s`)

### Running Tests

```bash
make test
```

The test suite compiles various Tane programs, assembles them with `gcc`, and verifies the exit codes.

### Example Programs

#### Hello World (with standard library)

```tane
import io;

fn main() {
    print("Hello, World!\n");
    exit(0);
}
```

#### Function with Arguments

```tane
fn add(a, b) {
    return a + b;
}

fn main() {
    let mut result;
    result = add(3, 4);
    return result;  // Returns 7
}
```

#### Control Flow

```tane
fn factorial(n) {
    let mut result;
    result = 1;
    let mut i;
    i = 1;
    while i <= n {
        result = result * i;
        i = i + 1;
    }
    return result;
}

fn main() {
    return factorial(5);  // Returns 120
}
```

#### Switch Expression

```tane
fn main() {
    let mut x;
    x = 2;
    let mut result;
    result = switch x {
        1 => 10,
        2 => 20,
        3 => 30,
    };
    return result;  // Returns 20
}
```

### Roadmap

- Add `break` / `continue` statements
- Type system (type annotations, type checking)
- Arrays and structures
- For loops
- Comments support

The EBNF below shows the minimal core specification (being updated incrementally).

# EBNF (Current Implementation)

```
/* ---- Top Level ---- */
file            := import-decl* func-decl* ;

import-decl     := 'import' ident-name ';' ;

func-decl       := 'fn' ident-name '(' params? ')' block ;
params          := ident-name ( ',' ident-name )* ;  /* max 6 parameters */

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
                 | string-lit
                 | func-call
                 | ident-name
                 | '(' expr ')'
                 | switch-expr ;

func-call       := ident-name '(' args? ')' ;
args            := expr ( ',' expr )* ;  /* max 6 arguments */

switch-expr     := 'switch' expr '{' switch-arm* '}' ;
switch-arm      := expr '=>' expr ','? ;

/* ---- Lexical ---- */
ident-name      := /[A-Za-z_][A-Za-z0-9_]*/ ;
int-lit         := '0' | /[1-9][0-9_]*/ ;
string-lit      := '"' /[^"]*/ '"' ;
WS              := ( ' ' | '\t' | '\r' | '\n' )+ ; 
```

## Constraints and Limitations

- Functions support up to **6 parameters/arguments** (limited by x86-64 calling convention registers: RDI, RSI, RDX, RCX, R8, R9)
- All variables are currently 8 bytes (no type system yet)
- No type annotations or type checking (planned for future releases)
- Comments are not yet supported

## Contributing

Contributions are welcome! Please feel free to submit issues or pull requests.