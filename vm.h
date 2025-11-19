/* ================================================================== */
/* =                         vm.h                                   = */
/* =    (CORREGIDO - Añadidos OP_AND y OP_OR)                       = */
/* ================================================================== */

#ifndef ROBOT_VM_H
#define ROBOT_VM_H

#include <stdint.h> 
#include <stdbool.h> 

typedef double Value;
typedef uint8_t byte;

#define STACK_MAX 256
#define GLOBALS_MAX 128
#define CONST_POOL_MAX 96
#define FRAMES_MAX 64

typedef enum {
    OP_PUSH_CONST,
    OP_POP,
    OP_HALT,

    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_NEGATE,

    OP_NOT,
    OP_EQUAL,
    OP_GREATER,
    OP_LESS,
    
    OP_AND,
    OP_OR,

    OP_SET_GLOBAL,
    OP_GET_GLOBAL,
    
    OP_SET_LOCAL,
    OP_GET_LOCAL,

    OP_JUMP,
    OP_JUMP_IF_FALSE,
    OP_LOOP,

    OP_CALL,
    OP_RETURN,

    OP_NATIVE_MOVE_FWD,
    OP_NATIVE_MOVE_BWD,
    OP_NATIVE_WAIT,
    OP_NATIVE_TURN_LEFT,
    OP_NATIVE_TURN_RIGHT,
    OP_NATIVE_CURVE_LEFT,
    OP_NATIVE_CURVE_RIGHT,
    OP_NATIVE_MOTOR_RIGHT,
    OP_NATIVE_MOTOR_LEFT,
    OP_NATIVE_STOP,
    OP_NATIVE_READ_SENSOR
    
} OpCode;

typedef struct {
    byte* returnAddress;
    Value* slots;
} CallFrame;

typedef struct {
    byte* ip;
    Value stack[STACK_MAX];
    Value* stackTop;
    Value constantPool[CONST_POOL_MAX];
    int constantCount;
    Value globals[GLOBALS_MAX];
    CallFrame frames[FRAMES_MAX];
    int frameCount;
    byte* bytecode_start;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_RUNTIME_ERROR
} InterpretResult;

void vm_init(VM* vm);
void vm_load_bytecode(VM* vm, byte* bytecode);
int vm_add_constant(VM* vm, Value value);
InterpretResult vm_interpretar(VM* vm);

#endif /* ROBOT_VM_H */