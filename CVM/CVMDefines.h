//
//  CVMDefines.h
//  CVM
//
//  Created by chudanqin on 22/12/2017.
//  Copyright © 2017 chudanqin. All rights reserved.
//

#ifndef CVMDefines_h
#define CVMDefines_h

// instruction set
typedef enum {
    LEA,
    IMM, // load immediate value to ax
    JMP,
    CALL,
    JZ,
    JNZ,
    ENT, // enter ?
    ADJ,
    LEV,
    LI, // load integer to ax, address in ax
    LC = 10, // load character to ax, address in ax
    SI, // save integer to address, value in ax, address on stack
    SC, // save character to address, value in ax, address on stack
    PUSH,
    OR,
    XOR,
    AND,
    EQ,
    NE,
    LT,
    GT = 20,
    LE,
    GE,
    SHL,
    SHR,
    ADD,
    SUB,
    MUL,
    DIV,
    MOD,
    OPEN = 30,
    READ,
    CLOS,
    PRTF,
    MALC,
    MSET,
    MCMP,
    EXIT,
} IS;

typedef long WORD; // TODO test arch, using NSInteger on iOS

typedef enum {
    _Undefined = 0,
    Num = 128, Fun, Sys, Glo, Loc, Id,
    Char = 134, Else, Enum, If, Int, Return, Sizeof, While,
    Assign = 142, Cond, Lor, Lan, Or, Xor, And, Eq, Ne = 150, Lt, Gt, Le, Ge, Shl, Shr, Add, Sub, Mul, Div = 160, Mod, Inc, Dec, Brak
} TOKEN;

typedef enum {
    CHAR,
    INT,
    PTR
} TYPE;

typedef struct {
    TOKEN token;
    WORD hash;
    WORD name;
    WORD class;
    TYPE type;
    WORD value;
    WORD BClass;
    TYPE BType;
    WORD BValue;
} IDENTIFIER;

#endif /* CVMDefines_h */
