//
//  cvm.c
//  CVM
//
//  Created by chudanqin on 22/12/2017.
//  Copyright © 2017 chudanqin. All rights reserved.
//

#include "cvm.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#pragma mark - VM

int debug = 1;
int compile_only = 0;

void debug_enabled(int enabled) {
    debug = enabled;
}

int is_debug_enabled(void) {
    return debug;
}

void compile_only_enabled(int enabled) {
    compile_only = enabled;
}

int is_compile_only_enabled(void) {
    return compile_only;
}

// segments {
WORD *text;
WORD *raw_text;
WORD *old_text;
WORD *stack;
WORD *raw_stack;
char *data;
char *raw_data;
// }

size_t text_size = 1024 * 1024;
size_t stack_size = 1024 * 1024;
size_t data_size = 1024 * 1024;

// registers {
WORD *pc;
WORD *bp;
WORD *sp;
WORD ax;
// }

static void register_init(void) {
    bp = sp = stack + stack_size;
    ax = 0;
}

CVMError segment_init(void) {
    size_t m_text_size = text_size * sizeof(text);
    if ((raw_text = text = old_text = malloc(m_text_size)) == NULL) {
        return InitTextSegError;
    }
    
    size_t m_stack_size = stack_size * sizeof(stack);
    if ((raw_stack = stack = malloc(m_stack_size)) == NULL) {
        return InitStackSegError;
    }
    
    size_t m_data_size = data_size * sizeof(data);
    if ((raw_data = data = malloc(m_data_size)) == NULL) {
        return InitDataSegError;
    }
    
    memset(text, 0, m_text_size);
    memset(stack, 0, m_stack_size);
    memset(data, 0, m_data_size);
    
    register_init();
    
    /*int i = 0;
     text[i++] = IMM;
     text[i++] = 10;
     text[i++] = PUSH;
     text[i++] = IMM;
     text[i++] = 20;
     text[i++] = ADD;
     text[i++] = PUSH;
     text[i++] = EXIT;
     pc = text;*/
    
    return NoError;
}

void segment_deinit(void) {
    if (raw_text != NULL) {
        free(raw_text);
        raw_text = text = old_text = NULL;
    }
    if (raw_stack != NULL) {
        free(raw_stack);
        raw_stack = stack = NULL;
    }
    if (raw_data != NULL) {
        free(raw_data);
        raw_data = data = NULL;
    }
}

char *data_segment_get(void) {
    return data;
}

void data_segment_set(char *ptr) {
    data = ptr;
}

void data_segment_insert(WORD v) {
    *data = v;
    data++;
}

void data_segment_inc(WORD size) {
    data = data + size;
}

WORD *text_segment_get(void) {
    return text;
}

WORD *text_segment_inc_and_get(WORD size) {
    text = text + size;
    return text;
}

void text_segment_inc(WORD size) {
    text = text + size;
}

void text_segment_set(WORD t) {
    *text = t;
}

void text_segment_append(WORD t) {
    text = text + 1;
    *text = t;
}

void dump_text_segment(void) {
    while (old_text < text) {
        console_assembly("%8.4s", & "LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,"
               "OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,"
               "OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT"[*++old_text * 5]);
        
        if (*old_text <= ADJ) {
            console_assembly(" %zd\n", *++old_text);
        } else {
            console_assembly("\n");
        }
    }
}

void stack_init(void) {
    // setup stack
    WORD *tmp;
    sp = (WORD *)((WORD)stack + stack_size);
#warning "WHY? TODO last two lines"
    *--sp = EXIT; // call exit if main returns
    *--sp = PUSH;
    tmp = sp;
    *--sp = 1;
    *--sp = (WORD)0;
    *--sp = (WORD)tmp;
}

#define _PC_STEP()    (pc++)
#define _SP_PUSH()    (sp--)
#define _SP_POP()     (sp++)

void _debug_quit(int code) {
    printf("\nvm exit with code: %d\n", code);
    exit(code);
}

int eval(void) {
    IS op;
    WORD *tmp;
    int cycle;
    cycle = 0;
    while (1) {
        op = (IS)*pc;
        _PC_STEP();
        
        if (debug) {
            cycle++;
            console_assembly("%4d> %.4s", cycle,
                   & "LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,"
                   "OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,"
                   "OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT"[op * 5]);
            if (op <= ADJ) {
                console_assembly(" %zd\n", *pc);
            } else {
                console_assembly("\n");
            }
        }
        
        switch (op) {
            case IMM: {
                ax = *pc;
                _PC_STEP();
                break;
            }
            case LC: {
                ax = (WORD)*((char *)ax);
                break;
            }
            case LI: {
                ax = *((WORD *)ax);
                break;
            }
            case SC: {
                /*ax = */*((char *)*sp) = (char)ax;
                _SP_POP();
                break;
            }
            case SI: {
                *((WORD *)*sp) = ax;
                _SP_POP();
                break;
            }
            case PUSH: {
                _SP_PUSH();
                *sp = ax;
                break;
            }
            case JMP: {
                pc = (WORD *)*pc;
                break;
            }
            case JZ: {
                pc = (ax != 0) ? (pc + 1) : (WORD *)*pc;
                break;
            }
            case JNZ: {
                pc = (ax == 0) ? (pc + 1) : (WORD *)*pc;
                break;
            }
            case CALL: {
                _SP_PUSH();
                *sp = (WORD)(pc + 1);
                pc =  (WORD *)*pc;
                break;
            }
                // case RET: { pc = (WORD *)*sp; }
            case ENT: { // enter a function
                _SP_PUSH();
                *(sp) = (WORD)bp;
                bp = sp;
                sp = sp - *pc;
                _PC_STEP();
                break;
            }
            case ADJ: { // adjust the stack
                sp = sp + *pc;
                _PC_STEP();
                break;
            }
            case LEV: { // return from a function
                sp = bp;
                bp = (WORD *)*sp;
                _SP_POP();
                pc = (WORD *)*sp;
                _SP_POP();
                break;
            }
            case LEA: { // load effective address
                ax = (WORD)(bp + *pc);
                _PC_STEP();
                break;
            }
            case OR: {
                ax = *sp | ax;
                _SP_POP();
                break;
            }
            case XOR: {
                ax = *sp ^ ax;
                _SP_POP();
                break;
            }
            case AND: {
                ax = *sp & ax;
                _SP_POP();
                break;
            }
            case EQ: {
                ax = *sp == ax;
                _SP_POP();
                break;
            }
            case NE: {
                ax = *sp != ax;
                _SP_POP();
                break;
            }
            case LT: {
                ax = *sp < ax;
                _SP_POP();
                break;
            }
            case LE: {
                ax = *sp <= ax;
                _SP_POP();
                break;
            }
            case GT: {
                ax = *sp > ax;
                _SP_POP();
                break;
            }
            case GE: {
                ax = *sp >= ax;
                _SP_POP();
                break;
            }
            case SHL: {
                ax = *sp << ax;
                _SP_POP();
                break;
            }
            case SHR: {
                ax = *sp >> ax;
                _SP_POP();
                break;
            }
            case ADD: {
                ax = *sp + ax;
                _SP_POP();
                break;
            }
            case SUB: {
                ax = *sp - ax;
                _SP_POP();
                break;
            }
            case MUL: {
                ax = *sp * ax;
                _SP_POP();
                break;
            }
            case DIV: {
                ax = *sp / ax;
                _SP_POP();
                break;
            }
            case MOD: {
                ax = *sp % ax;
                _SP_POP();
                break;
            }
            case PRTF: {
                tmp = sp + pc[1];
                ax = console_output((char *)tmp[-1], tmp[-2], tmp[-3], tmp[-4], tmp[-5], tmp[-6]);
                break;
            }
            case EXIT: {
                exit_signal_emit((int)*sp);
//                console_output("exit(%d)\n", (int)*sp);
                return NoError;
            }
            default: {
                console_output("unknown instruction:%d\n", op);
                return EvalError;
            }
        }
    }
}

#pragma mark - LexParser

char *src;
char *old_src;
char *last_pos;

void src_set(char *s) {
    src = s;
    old_src = s;
}

WORD id_table_idx = 0;
IDENTIFIER id_table[10 * 1024];
IDENTIFIER *main_entry_id_ptr;

void id_name_copy(IDENTIFIER i, char *str) {
    strncpy(str, src, src - (char *)i.name);
}

IDENTIFIER *current_id_ptr(void) {
    return &id_table[id_table_idx];
}

IDENTIFIER *next_id_ptr(void) {
    id_table_idx++;
    return &id_table[id_table_idx];
}

IDENTIFIER *id_ptr_reset(void) {
    id_table_idx = 0;
    return &id_table[0];
}

int id_table_reset(void) {
//    if (id_table != NULL) {
//        free(id_table);
//        id_table = NULL;
//    }
//    size_t id_table_size = 1024 * sizeof(IDENTIFIER);
//    if ((id_table = malloc(id_table_size)) == NULL) {
//        return InitIdTableError;
//    }
    memset(id_table, 0, sizeof(id_table));
    id_ptr_reset();
    return NoError;
}

WORD token;
WORD current_token(void) {
    return token;
}

WORD token_value;
WORD current_token_value(void) {
    return token_value;
}

WORD line;
WORD current_line(void) {
    return line;
}

WORD index_of_bp; // index of bp pointer on stack
TYPE basetype; // the type of a declaration, make it global for convenience
TYPE expr_type;   // the type of an expression

#define is_lower_case(token) (token >= 'a' && token <= 'z')

#define is_upper_case(token) (token >= 'A' && token <= 'Z')

#define is_letter(token) (is_lower_case(token) || is_upper_case(token))

#define is_numeric(token) (token >= '0' && token <= '9')

#define is_alphanumeric(token) (is_letter(token) || is_numeric(token))

void next(void) {
    WORD hash;
    while ((token = *src) > 0) {
        src++;
        if (token == '\n') {
            if (compile_only) {
                // print compile info
                console_assembly("line %ld: %.*s", line, (int)(src-old_src), old_src);
                old_src = src;
                dump_text_segment();
            }
            line++;
        } else if (token == '#') { // skip macro
            while (*src != '\n' && *src != 0) {
                src++;
            }
        } else if (is_letter(token) || token == '_') {
            last_pos = src - 1;
            hash = token;
            
            while (is_alphanumeric(*src) || token == '_') {
                hash = hash * 147 + *src;
                src++;
            }
            
            IDENTIFIER *id = id_ptr_reset();
            while(id->token > 0) {
                if (id->hash == hash && !memcmp((void *)id->name, last_pos, src - last_pos)) {
                    token = id->token;
                    return;
                }
                id = next_id_ptr();
            }
            
            id->name = (WORD)last_pos;
            id->hash = hash;
            id->token = Id;
            token = Id;
            return;
        } else if (is_numeric(token)) {
            token_value = token - '0';
            if (token_value > 0) { // decimalism
                while (is_numeric(*src)) {
                    token_value = 10 * token_value + (*src - '0');
                    src++;
                }
            } else {
                // starts with 0
                if (*src == 'x' || *src == 'X') { // hex
                    do {
                        src++;
                        token = *src;
                        WORD digit = token & 0xF + (token >= 'A' ? 0 : 9); // ascii('0') = 0x30
                        token_value = token_value * 16 + digit;
                    } while (is_numeric(token) || (token >= 'a' && token <= 'f') || (token >= 'A' && token <= 'F'));
                } else { // oct
                    while (*src >= '0' && *src <= '7') {
                        token_value = token_value * 8 + (*src - '0');
                        src++;
                    }
                }
            }
            token = Num;
            return;
        } else if (token == '/') {
            if (*src == '/') { // comments
                while (*src != 0 && *src != '\n') {
                    src++;
                }
            } else { // op '/'
                token = Div;
                return;
            }
        } else if (token == '"' || token == '\'') { // string or char
            last_pos = data_segment_get();
            while (*src != 0 && *src != token) {
                token_value = *src;
                src++;
                if (token_value == '\\') { // escape
                    token_value = *src;
                    src++;
                    if (token_value == 'n') {
                        token_value = '\n';
                    }
                }
                
                if (token == '"') {
                    data_segment_insert(token_value);
                }
            }
            
            src++;
            if (token == '\'') { // signle char as Num ??
                token = Num;
            } else {
                token_value = (WORD)last_pos;
            }
            return;
        } else if (token == '=') {
            if (*src == '=') {
                src++;
                token = Eq;
            } else {
                token = Assign;
            }
            return;
        } else if (token == '+') {
            if (*src == '+') {
                src++;
                token = Inc;
            } else {
                token = Add;
            }
            return;
        } else if (token == '-') {
            if (*src == '-') {
                src++;
                token = Dec;
            } else {
                token = Sub;
            }
            return;
        } else if (token == '!') {
            if (*src == '=') {
                src++;
                token = Ne;
            }
            return;
        } else if (token == '<') {
            if (*src == '=') {
                src++;
                token = Le;
            } else if (*src == '<') {
                src++;
                token = Shl;
            } else {
                token = Lt;
            }
            return;
        } else if (token == '>') {
            // parse '>=', '>>' or '>'
            if (*src == '=') {
                src++;
                token = Ge;
            } else if (*src == '>') {
                src++;
                token = Shr;
            } else {
                token = Gt;
            }
            return;
        } else if (token == '|') {
            // parse '|' or '||'
            if (*src == '|') {
                src++;
                token = Lor;
            } else {
                token = Or;
            }
            return;
        } else if (token == '&') {
            // parse '&' and '&&'
            if (*src == '&') {
                src++;
                token = Lan;
            } else {
                token = And;
            }
            return;
        }
        else if (token == '^') {
            token = Xor;
            return;
        }
        else if (token == '%') {
            token = Mod;
            return;
        }
        else if (token == '*') {
            token = Mul;
            return;
        }
        else if (token == '[') {
            token = Brak;
            return;
        }
        else if (token == '?') {
            token = Cond;
            return;
        }
        else if (token == '~' || token == ';' || token == '{' || token == '}' || token == '(' || token == ')' || token == ']' || token == ',' || token == ':') {
            // directly return the character as token;
            return;
        }
    }
}

void match(WORD tk) {
    if (token == tk) {
        next();
    } else {
        console_output("%ld: expected token: %ld\n", line, tk);
        _debug_quit(-1);
    }
}

void enum_declaration() {
    // parse enum [id] { a = 1, b = 3, ...}
    WORD i;
    i = 0;
    while (token != '}') {
        if (token != Id) {
            console_output("%ld: bad enum identifier %ld\n", line, token);
            _debug_quit(-1);
        }
        next();
        if (token == Assign) {
            // like {a=10}
            next();
            if (token != Num) {
                console_output("%ld: bad enum initializer\n", line);
                _debug_quit(-1);
            }
            i = token_value;
            next();
        }
        
        IDENTIFIER *id = current_id_ptr();
        id->class = Num;
        id->type = INT;
        id->value = i++;
        
        if (token == ',') {
            next();
        }
    }
}

void expression(TOKEN level) {
    // expressions have various format.
    // but majorly can be divided into two parts: unit and operator
    // for example `(char) *a[10] = (int *) func(b > 0 ? 10 : 20);
    // `a[10]` is an unit while `*` is an operator.
    // `func(...)` in total is an unit.
    // so we should first parse those unit and unary operators
    // and then the binary ones
    //
    // also the expression can be in the following types:
    //
    // 1. unit_unary ::= unit | unit unary_op | unary_op unit
    // 2. expr ::= unit_unary (bin_op unit_unary ...)
    
    // unit_unary()
    IDENTIFIER *id;
    TYPE tmp;
    WORD *addr;
    {
        if (!token) {
            console_output("%ld: unexpected token EOF of expression\n", line);
            _debug_quit(-1);
        }
        if (token == Num) {
            match(Num);
            
            // emit code
            text_segment_append(IMM);
            text_segment_append(token_value);
            expr_type = INT;
        }
        else if (token == '"') {
            // continous string "abc" "abc"
            
            
            // emit code
            text_segment_append(IMM);
            text_segment_append(token_value);
            
            match('"');
            // store the rest strings
            while (token == '"') {
                match('"');
            }
            
            // append the end of string character '\0', all the data are default
            // to 0, so just move data one position forward.
            data_segment_set((char *)(((WORD)data_segment_get() + sizeof(WORD)) & (-sizeof(WORD))));
            expr_type = PTR;
        }
        else if (token == Sizeof) {
            // sizeof is actually an unary operator
            // now only `sizeof(int)`, `sizeof(char)` and `sizeof(*...)` are
            // supported.
            match(Sizeof);
            match('(');
            expr_type = INT;
            
            if (token == Int) {
                match(Int);
            } else if (token == Char) {
                match(Char);
                expr_type = CHAR;
            }
            
            while (token == Mul) {
                match(Mul);
                expr_type = expr_type + PTR;
            }
            
            match(')');
            
            // emit code
            text_segment_append(IMM);
            if (expr_type == CHAR) {
                text_segment_append(sizeof(char));
            } else if (expr_type == INT) {
                text_segment_append(sizeof(int));
            } else {
                text_segment_append(sizeof(WORD));
            }
            
            expr_type = INT;
        }
        else if (token == Id) {
            // there are several type when occurs to Id
            // but this is unit, so it can only be
            // 1. function call
            // 2. Enum variable
            // 3. global/local variable
            match(Id);
            
            id = current_id_ptr();
            
            if (token == '(') {
                // function call
                match('(');
                
                // pass in arguments
                tmp = 0; // number of arguments
                while (token != ')') {
                    expression(Assign);
                    text_segment_append(PUSH);
                    tmp ++;
                    
                    if (token == ',') {
                        match(',');
                    }
                    
                }
                match(')');
                
                // emit code
                if (id->class == Sys) {
                    // system functions
                    text_segment_append(id->value);
                }
                else if (id->class == Fun) {
                    // function call
                    text_segment_append(CALL);
                    text_segment_append(id->value);
                }
                else {
                    console_output("%ld: bad function call\n", line);
                    _debug_quit(-1);
                }
                
                // clean the stack for arguments
                if (tmp > 0) {
                    text_segment_append(ADJ);
                    text_segment_append(tmp);
                }
                expr_type = id->type;
            }
            else if (id->class == Num) {
                // enum variable
                text_segment_append(IMM);
                text_segment_append(id->value);
                expr_type = INT;
            }
            else {
                // variable
                if (id->class == Loc) {
                    text_segment_append(LEA);
                    text_segment_append(index_of_bp - id->value);
                }
                else if (id->class == Glo) {
                    text_segment_append(IMM);
                    text_segment_append(id->value);
                }
                else {
                    console_output("%ld: undefined variable\n", line);
                    _debug_quit(-1);
                }
                
                // emit code, default behaviour is to load the value of the
                // address which is stored in `ax`
                expr_type = id->type;
                text_segment_append((expr_type == /*@ERR*/CHAR) ? LC : LI);
            }
        }
        else if (token == '(') {
            // cast or parenthesis
            match('(');
            if (token == Int || token == Char) {
                tmp = (token == Char) ? CHAR : INT; // cast type
                match(token);
                while (token == Mul) {
                    match(Mul);
                    tmp = tmp + PTR;
                }
                
                match(')');
                
                expression(Inc); // cast has precedence as Inc(++)
                
                expr_type  = tmp;
            } else {
                // normal parenthesis
                expression(Assign);
                match(')');
            }
        }
        else if (token == Mul) {
            // dereference *<addr>
            match(Mul);
            expression(Inc); // dereference has the same precedence as Inc(++)
            
            if (expr_type >= PTR) {
                expr_type = expr_type - PTR;
            } else {
                console_output("%ld: bad dereference\n", line);
                _debug_quit(-1);
            }
            text_segment_append((expr_type == CHAR) ? LC : LI);
        }
        else if (token == And) {
            // get the address of
            match(And);
            expression(Inc); // get the address of
            WORD t = *text_segment_get();
            if (t == LC || t == LI) {
                text_segment_inc(-1);
            } else {
                console_output("%ld: bad address of\n", line);
                _debug_quit(-1);
            }
            
            expr_type = expr_type + PTR;
        }
        else if (token == '!') {
            // not
            match('!');
            expression(Inc);
            
            // emit code, use <expr> == 0
            text_segment_append(PUSH);
            text_segment_append(IMM);
            text_segment_append(0);
            text_segment_append(EQ);
            
            expr_type = INT;
        }
        else if (token == '~') {
            // bitwise not
            match('~');
            expression(Inc);
            
            // emit code, use <expr> XOR -1
            text_segment_append(PUSH);
            text_segment_append(IMM);
            text_segment_append(-1);
            text_segment_append(XOR);
            
            expr_type = INT;
        }
        else if (token == Add) {
            // +var, do nothing
            match(Add);
            expression(Inc);
            
            expr_type = INT;
        }
        else if (token == Sub) {
            // -var
            match(Sub);
            
            if (token == Num) {
                text_segment_append(IMM);
                text_segment_append(-token_value);
                match(Num);
            } else {
                text_segment_append(IMM);
                text_segment_append(-1);
                text_segment_append(PUSH);
                expression(Inc);
                text_segment_append(MUL);
            }
            
            expr_type = INT;
        }
        else if (token == Inc || token == Dec) {
            // @ERR tmp = token;
            match(token);
            expression(Inc);
            WORD t = *text_segment_get();
            if (t == LC) {
                text_segment_append(PUSH); // to duplicate the address
                text_segment_append(LC);
            } else if (t == LI) {
                text_segment_append(PUSH); // to duplicate the address
                text_segment_append(LI);
            } else {
                console_output("%ld: bad lvalue of pre-increment\n", line);
                _debug_quit(-1);
            }
            text_segment_append(PUSH);
            text_segment_append(IMM);
            text_segment_append((expr_type > PTR) ? sizeof(WORD) : sizeof(char));
            text_segment_append((/*@ERR*/token == Inc) ? ADD : SUB);
            text_segment_append((expr_type == CHAR) ? SC : SI);
        }
        else {
            console_output("%ld: bad expression\n", line);
            _debug_quit(-1);
        }
    }
    
    // binary operator and postfix operators.
    {
        while (token >= level) {
            // handle according to current operator's precedence
            tmp = expr_type;
            if (token == Assign) {
                // var = expr;
                match(Assign);
                WORD t = *text_segment_get();
                if (t == LC || t == LI) {
                    // save the lvalue's pointer
                    text_segment_set(PUSH);
                } else {
                    console_output("%ld: bad lvalue in assignment\n", line);
                    _debug_quit(-1);
                }
                expression(Assign);
                
                expr_type = tmp;
                text_segment_append((expr_type == CHAR) ? SC : SI);
            }
            else if (token == Cond) {
                // expr ? a : b;
                match(Cond);
                text_segment_append(JZ);
                addr = text_segment_inc_and_get(1);
                expression(Assign);
                if (token == ':') {
                    match(':');
                } else {
                    console_output("%ld: missing colon in conditional\n", line);
                    _debug_quit(-1);
                }
                *addr = (WORD)(text_segment_get() + 3);
                text_segment_append(JMP);
                addr = text_segment_inc_and_get(1);
                expression(Cond);
                *addr = (WORD)(text_segment_get() + 1);
            }
            else if (token == Lor) {
                // logic or
                match(Lor);
                text_segment_append(JNZ);
                addr = text_segment_inc_and_get(1);
                expression(Lan);
                *addr = (WORD)(text_segment_get() + 1);
                expr_type = INT;
            }
            else if (token == Lan) {
                // logic and
                match(Lan);
                text_segment_append(JZ);
                addr = text_segment_inc_and_get(1);
                expression(Or);
                *addr = (WORD)(text_segment_get + 1);
                expr_type = INT;
            }
            else if (token == Or) {
                // bitwise or
                match(Or);
                text_segment_append(PUSH);
                expression(Xor);
                text_segment_append(OR);
                expr_type = INT;
            }
            else if (token == Xor) {
                // bitwise xor
                match(Xor);
                text_segment_append(PUSH);
                expression(And);
                text_segment_append(XOR);
                expr_type = INT;
            }
            else if (token == And) {
                // bitwise and
                match(And);
                text_segment_append(PUSH);
                expression(Eq);
                text_segment_append(ADD);
                expr_type = INT;
            }
            else if (token == Eq) {
                // equal ==
                match(Eq);
                text_segment_append(PUSH);
                expression(Ne);
                text_segment_append(EQ);
                expr_type = INT;
            }
            else if (token == Ne) {
                // not equal !=
                match(Ne);
                text_segment_append(PUSH);
                expression(Lt);
                text_segment_append(NE);
                expr_type = INT;
            }
            else if (token == Lt) {
                // less than
                match(Lt);
                text_segment_append(PUSH);
                expression(Shl);
                text_segment_append(LT);
                expr_type = INT;
            }
            else if (token == Gt) {
                // greater than
                match(Gt);
                text_segment_append(PUSH);
                expression(Shl);
                text_segment_append(GT);
                expr_type = INT;
            }
            else if (token == Le) {
                // less than or equal to
                match(Le);
                text_segment_append(PUSH);
                expression(Shl);
                text_segment_append(LE);
                expr_type = INT;
            }
            else if (token == Ge) {
                // greater than or equal to
                match(Ge);
                text_segment_append(PUSH);
                expression(Shl);
                text_segment_append(GE);
                expr_type = INT;
            }
            else if (token == Shl) {
                // shift left
                match(Shl);
                text_segment_append(PUSH);
                expression(Add);
                text_segment_append(SHL);
                expr_type = INT;
            }
            else if (token == Shr) {
                // shift right
                match(Shr);
                text_segment_append(PUSH);
                expression(Add);
                text_segment_append(SHR);
                expr_type = INT;
            }
            else if (token == Add) {
                // add
                match(Add);
                text_segment_append(PUSH);
                expression(Mul);
                
                expr_type = tmp;
                if (expr_type > PTR) {
                    // pointer type, and not `char *`
                    text_segment_append(PUSH);
                    text_segment_append(IMM);
                    text_segment_append(sizeof(INT));
                    text_segment_append(MUL);
                }
                text_segment_append(ADD);
            }
            else if (token == Sub) {
                // sub
                match(Sub);
                text_segment_append(PUSH);
                expression(Mul);
                if (tmp > PTR && tmp == expr_type) {
                    // pointer subtraction
                    text_segment_append(SUB);
                    text_segment_append(PUSH);
                    text_segment_append(IMM);
                    text_segment_append(sizeof(WORD));
                    text_segment_append(DIV);
                    expr_type = INT;
                } else if (tmp > PTR) {
                    // pointer movement
                    text_segment_append(PUSH);
                    text_segment_append(IMM);
                    text_segment_append(sizeof(WORD));
                    text_segment_append(MUL);
                    text_segment_append(SUB);
                    expr_type = tmp;
                } else {
                    // numeral subtraction
                    text_segment_append(SUB);
                    expr_type = tmp;
                }
            }
            else if (token == Mul) {
                // multiply
                match(Mul);
                text_segment_append(PUSH);
                expression(Inc);
                text_segment_append(MUL);
                expr_type = tmp;
            }
            else if (token == Div) {
                // divide
                match(Div);
                text_segment_append(PUSH);
                expression(Inc);
                text_segment_append(DIV);
                expr_type = tmp;
            }
            else if (token == Mod) {
                // Modulo
                match(Mod);
                text_segment_append(PUSH);
                expression(Inc);
                text_segment_append(MOD);
                expr_type = tmp;
            }
            else if (token == Inc || token == Dec) {
                // postfix inc(++) and dec(--)
                // we will increase the value to the variable and decrease it
                // on `ax` to get its original value.
                WORD t = *text_segment_get();
                if (t == LI) {
                    text_segment_set(PUSH);
                    text_segment_append(LI);
                }
                else if (t == LC) {
                    text_segment_set(PUSH);
                    text_segment_append(LC);
                }
                else {
                    console_output("%ld: bad value in increment\n", line);
                    _debug_quit(-1);
                }
                
                text_segment_append(PUSH);
                text_segment_append(IMM);
                text_segment_append((expr_type > PTR) ? sizeof(WORD) : sizeof(char));
                text_segment_append((token == Inc) ? ADD : SUB);
                text_segment_append((expr_type == CHAR) ? SC : SI);
                text_segment_append(PUSH);
                text_segment_append(IMM);
                text_segment_append((expr_type > PTR) ? sizeof(WORD) : sizeof(char));
                text_segment_append((token == Inc) ? SUB : ADD);
                
                match(token);
            }
            else if (token == Brak) {
                // array access var[xx]
                match(Brak);
                text_segment_append(PUSH);
                expression(Assign);
                match(']');
                
                if (tmp > PTR) {
                    // pointer, `not char *`
                    text_segment_append(PUSH);
                    text_segment_append(IMM);
                    text_segment_append(sizeof(WORD));
                    text_segment_append(MUL);
                }
                else if (tmp < PTR) {
                    console_output("%ld: pointer type expected\n", line);
                    _debug_quit(-1);
                }
                expr_type = tmp - PTR;
                text_segment_append(ADD);
                text_segment_append((expr_type == CHAR) ? LC : LI);
            }
            else {
                console_output("%ld: compiler error, token = %ld\n", line, token);
                _debug_quit(-1);
            }
        }
    }
}

void statement() {
    // there are 8 kinds of statements here:
    // 1. if (...) <statement> [else <statement>]
    // 2. while (...) <statement>
    // 3. { <statement> }
    // 4. return xxx;
    // 5. <empty statement>;
    // 6. expression; (expression end with semicolon)
    
    WORD *a, *b; // bess for branch control
    
    if (token == If) {
        // if (...) <statement> [else <statement>]
        //
        //   if (...)           <cond>
        //                      JZ a
        //     <statement>      <statement>
        //   else:              JMP b
        // a:
        //     <statement>      <statement>
        // b:                   b:
        //
        //
        match(If);
        match('(');
        expression(Assign);  // parse condition
        match(')');
        
        // emit code for if
        text_segment_append(JZ);
        b = text_segment_inc_and_get(1);
        
        statement();         // parse statement
        if (token == Else) { // parse else
            match(Else);
            
            // emit code for JMP B
            *b = (WORD)(text_segment_get() + 3);
            text_segment_append(JMP);
            b = text_segment_inc_and_get(1);
            
            statement();
        }
        
        *b = (WORD)(text_segment_get() + 1);
    }
    else if (token == While) {
        //
        // a:                     a:
        //    while (<cond>)        <cond>
        //                          JZ b
        //     <statement>          <statement>
        //                          JMP a
        // b:                     b:
        match(While);
        
        a = text_segment_get() + 1;
        
        match('(');
        expression(Assign);
        match(')');
        
        text_segment_append(JZ);
        b = text_segment_inc_and_get(1);
        
        statement();
        
        text_segment_append(JMP);
        text_segment_append((WORD)a);
        *b = (WORD)(text_segment_get() + 1);
    }
    else if (token == '{') {
        // { <statement> ... }
        match('{');
        
        while (token != '}') {
            statement();
        }
        
        match('}');
    }
    else if (token == Return) {
        // return [expression];
        match(Return);
        
        if (token != ';') {
            expression(Assign);
        }
        
        match(';');
        
        // emit code for return
        text_segment_append(LEV);
    }
    else if (token == ';') {
        // empty statement
        match(';');
    }
    else {
        // a = b; or function_call();
        expression(Assign);
        match(';');
    }
}

void function_parameter() {
    TYPE type;
    WORD params;
    params = 0;
    while (token != ')') {
        // int name, ...
        type = INT;
        if (token == Int) {
            match(Int);
        } else if (token == Char) {
            type = CHAR;
            match(Char);
        }
        
        // pointer type
        while (token == Mul) {
            match(Mul);
            type = type + PTR;
        }
        
        // parameter name
        if (token != Id) {
            console_output("%ld: bad parameter declaration\n", line);
            _debug_quit(-1);
        }
        IDENTIFIER *id = current_id_ptr();
        if (id->class == Loc) {
            console_output("%ld: duplicate parameter declaration\n", line);
            _debug_quit(-1);
        }
        
        match(Id);
        // store the local variable
        id->BClass = id->class; id->class = Loc;
        id->BType = id->type; id->type = type;
        id->BValue = id->value; id->value = params++; // index of current parameter
        
        if (token == ',') {
            match(',');
        }
    }
    index_of_bp = params+1;
}

void function_body() {
    // type func_name (...) {...}
    //                   -->|   |<--
    
    // ... {
    // 1. local declarations
    // 2. statements
    // }
    
    WORD pos_local; // position of local variables on the stack.
    TYPE type;
    pos_local = index_of_bp;
    
    while (token == Int || token == Char) {
        // local variable declaration, just like global ones.
        basetype = (token == Int) ? INT : CHAR;
        match(token); // here == next();
        
        while (token != ';') {
            type = basetype;
            while (token == Mul) {
                match(Mul);
                type = type + PTR;
            }
            
            if (token != Id) {
                // invalid declaration
                console_output("%ld: bad local declaration\n", line);
                _debug_quit(-1);
            }
            IDENTIFIER *id = current_id_ptr();
            if (id->class == Loc) {
                // identifier exists
                console_output("%ld: duplicate local declaration\n", line);
                _debug_quit(-1);
            }
            match(Id);
            
            // store the local variable
            id->BClass = id->class; id->class = Loc;
            id->BType = id->type; id->type = type;
            id->BValue = id->value; id->value = ++pos_local; // index of current parameter
            
            if (token == ',') {
                match(',');
            }
        }
        match(';');
    }
    
    // save the stack size for local variables
    text_segment_append(ENT);
    text_segment_append(pos_local - index_of_bp);
    
    // statements
    while (token != '}') {
        statement();
    }
    
    // emit code for leaving the sub function
    text_segment_append(LEV);
}

void function_declaration() {
    // type func_name (...) {...}
    //               | this part
    
    match('(');
    function_parameter();
    match(')');
    match('{');
    function_body();
    //match('}');
    
    // unwind local variable declarations for all local variables.
    IDENTIFIER *id = id_ptr_reset();
    while (id->token > 0) {
        if (id->class == Loc) {
            id->class = id->BClass;
            id->type = id->BType;
            id->value = id->BValue;
        }
        id = next_id_ptr();
    }
}

void global_declaration() {
    // int [*]id [; | (...) {...}]
    
    
    TYPE type; // tmp, actual type for variable
    //int i; // tmp
    
    basetype = INT;
    
    // parse enum, this should be treated alone.
    if (token == Enum) {
        // enum [id] { a = 10, b = 20, ... }
        match(Enum);
        if (token != '{') {
            match(Id); // skip the [id] part
        }
        if (token == '{') {
            // parse the assign part
            match('{');
            enum_declaration();
            match('}');
        }
        
        match(';');
        return;
    }
    
    // parse type information
    if (token == Int) {
        match(Int);
    }
    else if (token == Char) {
        match(Char);
        basetype = CHAR;
    }
    
    // parse the comma seperated variable declaration.
    while (token != ';' && token != '}') {
        type = basetype;
        // parse pointer type, note that there may exist `int ****x;`
        while (token == Mul) {
            match(Mul);
            type = type + PTR;
        }
        
        if (token != Id) {
            // invalid declaration
            console_output("%ld: bad global declaration\n", line);
            _debug_quit(-1);
        }
        IDENTIFIER *id = current_id_ptr();
        if (id->class != 0) {
            // identifier exists
            console_output("%ld: duplicate global declaration\n", line);
            _debug_quit(-1);
        }
        match(Id);
        id->type = type;
        
        if (token == '(') {
            id->class = Fun;
            id->value = (WORD)(text_segment_get() + 1); // the memory address of function
            function_declaration();
        } else {
            // variable declaration
            id->class= Glo; // global variable
            id->value = (WORD)data_segment_get(); // assign memory address
            data_segment_inc(sizeof(WORD));
        }
        
        if (token == ',') {
            match(',');
        }
    }
    next();
}

void program() {
    // get next token
    next();
    while (token > 0) {
        global_declaration();
    }
}

void add_keywords(void) {
    src_set("char else enum if int return sizeof while");
    TOKEN i = Char;
    while (i <= While) {
        next();
        IDENTIFIER *id = current_id_ptr();
        id->token = i++;
    }
}

void add_libraries(void) {
    src_set("open read close printf malloc memset memcmp exit");
    IS i = OPEN;
    while (i <= EXIT) {
        next();
        IDENTIFIER *id = current_id_ptr();
        id->class = Sys;
        id->type = INT;
        id->value = i++;
    }
}

void add_main_func_entry(void) {
    src_set("void");
    next();
    IDENTIFIER *id = current_id_ptr();
    id->token = Char;
    
    src_set("main");
    next();
    //idmain = current_id; // keep track of main
    main_entry_id_ptr = current_id_ptr();
}

void pc_init(void) {
    if (main_entry_id_ptr->value == 0) {
        console_output("main not found, exit(-1)\n");
        _debug_quit(-1);
    }
    pc = (WORD *)main_entry_id_ptr->value;
}

CVMError vm_init(void) {
    CVMError err;
    segment_deinit();
    err = segment_init();
    if (err) {
        return err;
    }
    err = id_table_reset();
    if (err) {
        return err;
    }
    line = 1;
    add_keywords();
    add_libraries();
    add_main_func_entry();
    return NoError;
}

CVMError vm_eval(char *src) {
    src_set(src);
    program();
    if (compile_only) {
        exit_signal_emit(0);
        return NoError;
    }
    pc_init();
    stack_init();
    return eval();
}

#pragma mark - DEBUG

void print_current_id_name(void) {
    IDENTIFIER *id = current_id_ptr();
    char name[1024] = {0, };
    id_name_copy(*id, name);
    printf("%s\n", name);
}

void print_text_segment(void) {
    WORD *t = old_text;
    while (t < text) {
        printf("%8.4s", & "LEA ,IMM ,JMP ,CALL,JZ  ,JNZ ,ENT ,ADJ ,LEV ,LI  ,LC  ,SI  ,SC  ,PUSH,"
                         "OR  ,XOR ,AND ,EQ  ,NE  ,LT  ,GT  ,LE  ,GE  ,SHL ,SHR ,ADD ,SUB ,MUL ,DIV ,MOD ,"
                         "OPEN,READ,CLOS,PRTF,MALC,MSET,MCMP,EXIT"[*++t * 5]);
        
        if (*old_text <= ADJ) {
            printf(" %zd\n", *++t);
        } else {
            printf("\n");
        }
    }
}

void dump(void) {
    printf("pc: %zd, bp: %zd, sp: %zd, ax: %zd, stack:\n", *pc, *bp, *sp, ax);
    for (WORD *p = bp; p >= sp; p--) {
        printf("%zd\n", *p);
    }
}

#pragma mark - outputs

static console_output_func _console_output_func_ptr;
static void *_console_output_context;

void console_output_set_handler(void *context, console_output_func func) {
    _console_output_context = context;
    _console_output_func_ptr = func;
}

int console_output(const char *restrict format, ...) {
    int v = 0;
    va_list argList;
    va_start(argList, format);
    if (_console_output_func_ptr) {
        v = _console_output_func_ptr(_console_output_context, format, argList);
        if (debug) {
            va_end(argList);
            va_start(argList, format);
            vprintf(format, argList);
        }
    } else {
        v = vprintf(format, argList);
    }
    va_end(argList);
    return v;
}

#pragma mark - assembly

static console_assembly_func _console_assembly_func_ptr;
static void *_console_assembly_context;

void console_assembly_set_handler(void *context, console_assembly_func func) {
    _console_assembly_context = context;
    _console_assembly_func_ptr = func;
}

int console_assembly(const char *restrict format, ...) {
    int v = 0;
    va_list argList;
    va_start(argList, format);
    if (_console_assembly_func_ptr) {
        v = _console_assembly_func_ptr(_console_output_context, format, argList);
        if (debug) {
            va_end(argList);
            va_start(argList, format);
            vprintf(format, argList);
        }
    } else {
        v = vprintf(format, argList);
    }
    va_end(argList);
    return v;
}

#pragma mark - exit

static exit_signal_func _exit_signal_func_ptr;
static void *_exit_signal_context;

void exit_signal_set_handler(void *context, exit_signal_func func) {
    _exit_signal_context = context;
    _exit_signal_func_ptr = func;
}

void exit_signal_emit(int code) {
    if (_exit_signal_func_ptr) {
        _exit_signal_func_ptr(_exit_signal_context, code);
    } else {
        printf("exit(%d)\n", code);
    }
}
