/* ================================================================== */
/* =                       OumiVM/vm.h                              = */
/* =             (OPTIMIZADO PARA AHORRAR RAM)                      = */
/* ================================================================== */

#ifndef ROBOT_VM_H
#define ROBOT_VM_H

#include <stdint.h> 
#include <stdbool.h> 
#include "Arduino.h"

typedef double Value; // 4 bytes en Arduino Uno
typedef uint8_t byte;

// --- REDUCCIÓN DE MEMORIA ---
// Antes: 64, 32, 32, 16
// Ahora: Ajustado al límite para que quepa con la librería SD
#define STACK_MAX 32      
#define GLOBALS_MAX 20    
#define CONST_POOL_MAX 32 // Tu programa usa unas 30 constantes, no bajar de 30
#define FRAMES_MAX 8

typedef enum {
    OP_PUSH_CONST, OP_POP, OP_HALT,
    OP_ADD, OP_SUB, OP_MUL, OP_DIV, OP_NEGATE,
    OP_NOT, OP_EQUAL, OP_GREATER, OP_LESS, OP_AND, OP_OR,
    OP_SET_GLOBAL, OP_GET_GLOBAL, OP_SET_LOCAL, OP_GET_LOCAL,
    OP_JUMP, OP_JUMP_IF_FALSE, OP_LOOP,
    OP_CALL, OP_RETURN,
    OP_NATIVE_MOVE_FWD, OP_NATIVE_MOVE_BWD, OP_NATIVE_WAIT,
    OP_NATIVE_TURN_LEFT, OP_NATIVE_TURN_RIGHT, OP_NATIVE_CURVE_LEFT,
    OP_NATIVE_CURVE_RIGHT, OP_NATIVE_MOTOR_RIGHT, OP_NATIVE_MOTOR_LEFT,
    OP_NATIVE_STOP, OP_NATIVE_READ_SENSOR
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

#endif