//
//  cvm.h
//  CVM
//
//  Created by chudanqin on 22/12/2017.
//  Copyright © 2017 chudanqin. All rights reserved.
//

#ifndef cvm_h
#define cvm_h

#include "CVMDefines.h"

typedef enum {
    InitTextSegError = -5,
    InitStackSegError,
    InitDataSegError,
    InitIdTableError,
    EvalError,
    NoError = 0,
} CVMError;

/*
extern CVMError segment_init(void);

extern void segment_deinit(void);

extern void pc_init(WORD *ptr);

extern void stack_init(void);

extern char *data_segment_get(void);

extern void data_segment_set(char *ptr);

extern void data_segment_insert(WORD v);

extern void data_segment_inc(WORD size);

extern WORD *text_segment_get(void);

extern WORD *text_segment_inc_and_get(WORD size);

extern void text_segment_inc(WORD size);

extern void text_segment_set(WORD t);

extern void text_segment_inc_and_set(WORD size, WORD t);

extern void print_text_segment(void);

extern int eval(void);

extern void src_set(char *s);

extern void id_name_copy(IDENTIFIER id, char *str);

extern WORD current_token(void);

extern WORD current_token_value(void);

extern void next(void);
*/

extern void debug_enabled(int enabled);

extern int is_debug_enabled(void);

extern void compile_only_enabled(int enabled);

extern int is_compile_only_enabled(void);

extern CVMError vm_init(void);

extern CVMError vm_eval(char *src);

#include "stdarg.h"

typedef int (*console_output_func)(void *context, const char *restrict format, va_list va_l);

void console_output_set_handler(void *context, console_output_func func);

int console_output(const char *restrict format, ...);

typedef int (*console_assembly_func)(void *context, const char *restrict format, va_list va_l);

void console_assembly_set_handler(void *context, console_assembly_func func);

int console_assembly(const char *restrict format, ...);

typedef void (*exit_signal_func)(void *context, int code);

void exit_signal_set_handler(void *context, exit_signal_func func);

void exit_signal_emit(int code);

#endif /* cvm_h */
