/* ================================================================== */
/* =                         vm.c                                   = */
/* =    (CORREGIDO - Lógica de simulación de sensor arreglada)      = */
/* ================================================================== */

#include "vm.h"
#include <stdio.h> 
#include <string.h> 
#include <stdarg.h> 

/* --- 1. Stubs del Hardware del Robot (Arduino) --- */
void robot_mover_adelante(int velocidad) { printf("[ROBOT_HAL]: Mover adelante (Vel: %d)\n", velocidad); }
void robot_mover_atras(int velocidad) { printf("[ROBOT_HAL]: Mover atras (Vel: %d)\n", velocidad); }
void robot_parar() { printf("[ROBOT_HAL]: Parar motores\n"); }
void robot_esperar(int ms) { printf("[ROBOT_HAL]: Esperar %d ms\n", ms); }
void robot_girar_izquierda(int velocidad) { printf("[ROBOT_HAL]: Girar izquierda (Vel: %d)\n", velocidad); }
void robot_girar_derecha(int velocidad) { printf("[ROBOT_HAL]: Girar derecha (Vel: %d)\n", velocidad); }
void robot_curva_izquierda(int velocidad) { printf("[ROBOT_HAL]: Curva izquierda (Vel: %d)\n", velocidad); }
void robot_curva_derecha(int velocidad) { printf("[ROBOT_HAL]: Curva derecha (Vel: %d)\n", velocidad); }
void robot_motor_derecho(int velocidad) { printf("[ROBOT_HAL]: Motor derecho (Vel: %d)\n", velocidad); }
void robot_motor_izquierdo(int velocidad) { printf("[ROBOT_HAL]: Motor izquierdo (Vel: %d)\n", velocidad); }

/* ================== ¡CAMBIO AQUÍ! ================== */
/* Lógica de simulación arreglada (¡de verdad!) */
int robot_leer_sensor(int id_sensor) { 
    printf("[ROBOT_HAL]: Leyendo sensor %d\n", id_sensor);
    
    /* Usamos un contador estático para los ciclos del *robot* */
    static int contador_ciclos = 0;
    
    /* * ¡EL FIX!
     * El estado solo debe avanzar DESPUÉS de que el ciclo de lectura
     * (lectura de 8 y 9) se complete.
     * Incrementamos el contador solo cuando el sensor 9 (el último) es leído.
     */
    int contador_actual = contador_ciclos;

    if (id_sensor == 9) { // <-- MOVER EL INCREMENTO AL SENSOR 9
        contador_ciclos = (contador_ciclos + 1) % 4; /* 0, 1, 2, 3 */
    }

    /* Estado 0: (Ambos Blanco) */
    /* Estado 1: (Ambos Blanco) */
    
    /* Estado 2: Izquierdo detecta negro */
    /* Usamos 'contador_actual' para la lógica de este turno */
    if (contador_actual == 2 && id_sensor == 8) {
        printf(" -> (Detecto Negro)\n");
        return 1;
    }
    
    /* Estado 3: Derecho detecta negro */
    if (contador_actual == 3 && id_sensor == 9) {
        printf(" -> (Detecto Negro)\n");
        return 1;
    }
    
    return 0; /* Por defecto, devuelve Blanco */
}
/* ================== FIN DEL CAMBIO ================== */


/* --- 2. Funciones de Ayuda (Helpers) de la VM --- */
static void runtimeError(VM* vm, const char* format, ...) {
    va_list args;
    va_start(args, format);
    printf("Error de Runtime: ");
    vprintf(format, args);
    va_end(args);
    printf("\n");
    vm->ip = NULL; 
}

static void push(VM* vm, Value value) {
    if (vm->ip == NULL) return;
    if (vm->stackTop - vm->stack >= STACK_MAX) {
        runtimeError(vm, "Stack overflow (pila de valores llena)");
        return;
    }
    *vm->stackTop = value;
    vm->stackTop++;
}

static Value pop(VM* vm) {
    if (vm->ip == NULL) return 0.0;
    if (vm->stackTop == vm->stack) {
        runtimeError(vm, "Stack empty (intento de pop en pila vacia)");
        return 0.0;
    }
    vm->stackTop--;
    return *vm->stackTop;
}

static Value peek(VM* vm, int distance) {
    if (vm->ip == NULL) return 0.0;
    if (vm->stackTop - vm->stack <= distance) {
         runtimeError(vm, "Stack underflow (intento de peek invalido)");
         return 0.0;
    }
    return vm->stackTop[-1 - distance];
}

static bool isFalsy(Value value) {
    return value == 0.0;
}


/* --- 3. Implementación de Funciones Públicas --- */

void vm_init(VM* vm) {
    memset(vm, 0, sizeof(VM)); 
    vm->stackTop = vm->stack;
    vm->frameCount = 0;
    vm->constantCount = 0;
}

void vm_load_bytecode(VM* vm, byte* bytecode) {
    vm->bytecode_start = bytecode;
    vm->ip = bytecode;
}

int vm_add_constant(VM* vm, Value value) {
    if (vm->constantCount >= CONST_POOL_MAX) {
        return -1; 
    }
    vm->constantPool[vm->constantCount] = value;
    return vm->constantCount++;
}

/* --- 4. El Bucle Principal (Fetch-Decode-Execute) --- */

InterpretResult vm_interpretar(VM* vm) {
    
    #define READ_BYTE() (*vm->ip++)
    #define READ_CONST() (vm->constantPool[READ_BYTE()])
    #define READ_SHORT() \
        (vm->ip += 2, (uint16_t)((vm->ip[-2] << 8) | vm->ip[-1]))
    #define BINARY_OP(op) \
        do { \
            Value b = pop(vm); \
            Value a = pop(vm); \
            if (vm->ip == NULL) return INTERPRET_RUNTIME_ERROR; \
            push(vm, a op b); \
        } while (0)
    #define NATIVE_CALL_1_VOID(func) \
        do { \
            Value arg = pop(vm); \
            if (vm->ip == NULL) return INTERPRET_RUNTIME_ERROR; \
            func((int)arg); \
        } while (0)

    
    for (;;) {
        
        if(vm->ip == NULL) return INTERPRET_RUNTIME_ERROR;
        
        byte instruction = READ_BYTE();
        
        switch (instruction) {
            
            case OP_HALT:
                return INTERPRET_OK;
            case OP_PUSH_CONST:
                push(vm, READ_CONST());
                break;
            case OP_POP:
                pop(vm);
                break;

            case OP_ADD: BINARY_OP(+); break;
            case OP_SUB: BINARY_OP(-); break;
            case OP_MUL: BINARY_OP(*); break;
            case OP_DIV: BINARY_OP(/); break;
            case OP_NEGATE: {
                Value val = pop(vm);
                if(vm->ip == NULL) return INTERPRET_RUNTIME_ERROR;
                push(vm, -val);
                break;
            }
                
            case OP_NOT: {
                Value val = pop(vm);
                if(vm->ip == NULL) return INTERPRET_RUNTIME_ERROR;
                push(vm, isFalsy(val) ? 1.0 : 0.0);
                break;
            }
            case OP_EQUAL: {
                Value b = pop(vm);
                Value a = pop(vm);
                if(vm->ip == NULL) return INTERPRET_RUNTIME_ERROR;
                push(vm, (a == b) ? 1.0 : 0.0);
                break;
            }
            case OP_GREATER: BINARY_OP(>); break;
            case OP_LESS:    BINARY_OP(<); break;
            
            case OP_AND: {
                Value b = pop(vm);
                Value a = pop(vm);
                if(vm->ip == NULL) return INTERPRET_RUNTIME_ERROR;
                push(vm, (!isFalsy(a) && !isFalsy(b)) ? 1.0 : 0.0);
                break;
            }
            case OP_OR: {
                Value b = pop(vm);
                Value a = pop(vm);
                if(vm->ip == NULL) return INTERPRET_RUNTIME_ERROR;
                push(vm, (!isFalsy(a) || !isFalsy(b)) ? 1.0 : 0.0);
                break;
            }

            case OP_SET_GLOBAL: {
                byte index = READ_BYTE();
                vm->globals[index] = pop(vm); 
                if(vm->ip == NULL) return INTERPRET_RUNTIME_ERROR;
                break;
            }
            case OP_GET_GLOBAL: {
                byte index = READ_BYTE();
                push(vm, vm->globals[index]);
                break;
            }
            
            case OP_SET_LOCAL: {
                byte slot = READ_BYTE();
                vm->frames[vm->frameCount - 1].slots[slot] = peek(vm, 0);
                break;
            }
            case OP_GET_LOCAL: {
                byte slot = READ_BYTE();
                push(vm, vm->frames[vm->frameCount - 1].slots[slot]);
                break;
            }
            
            case OP_JUMP: {
                uint16_t offset = READ_SHORT();
                vm->ip += offset;
                break;
            }
            case OP_JUMP_IF_FALSE: {
                uint16_t offset = READ_SHORT();
                Value cond = pop(vm);
                if(vm->ip == NULL) return INTERPRET_RUNTIME_ERROR;
                if (isFalsy(cond)) {
                    vm->ip += offset;
                }
                break;
            }
            case OP_LOOP: {
                uint16_t offset = READ_SHORT();
                vm->ip -= offset;
                break;
            }
            
            case OP_CALL: {
                byte argCount = READ_BYTE();
                Value callee_addr_val = peek(vm, argCount); 
                if(vm->ip == NULL) return INTERPRET_RUNTIME_ERROR;
                
                if (vm->frameCount == FRAMES_MAX) {
                    runtimeError(vm, "Stack overflow (pila de llamadas llena)");
                    return INTERPRET_RUNTIME_ERROR;
                }
                
                CallFrame* frame = &vm->frames[vm->frameCount++];
                frame->returnAddress = vm->ip;
                frame->slots = vm->stackTop - argCount - 1; 
                vm->ip = vm->bytecode_start + (int)callee_addr_val;
                break;
            }
            
            case OP_RETURN: {
                if (vm->frameCount == 0) {
                    pop(vm);
                    return INTERPRET_OK;
                }
                
                CallFrame* frame = &vm->frames[--vm->frameCount];
                Value returnValue = pop(vm); 
                if(vm->ip == NULL) return INTERPRET_RUNTIME_ERROR;
                
                vm->stackTop = frame->slots;
                push(vm, returnValue);
                vm->ip = frame->returnAddress;
                break;
            }

            case OP_NATIVE_MOVE_FWD:   NATIVE_CALL_1_VOID(robot_mover_adelante); break;
            case OP_NATIVE_MOVE_BWD:   NATIVE_CALL_1_VOID(robot_mover_atras); break;
            case OP_NATIVE_WAIT:       NATIVE_CALL_1_VOID(robot_esperar); break;
            case OP_NATIVE_TURN_LEFT:  NATIVE_CALL_1_VOID(robot_girar_izquierda); break;
            case OP_NATIVE_TURN_RIGHT: NATIVE_CALL_1_VOID(robot_girar_derecha); break;
            case OP_NATIVE_CURVE_LEFT: NATIVE_CALL_1_VOID(robot_curva_izquierda); break;
            case OP_NATIVE_CURVE_RIGHT:NATIVE_CALL_1_VOID(robot_curva_derecha); break;
            case OP_NATIVE_MOTOR_RIGHT:NATIVE_CALL_1_VOID(robot_motor_derecho); break;
            case OP_NATIVE_MOTOR_LEFT: NATIVE_CALL_1_VOID(robot_motor_izquierdo); break;
            case OP_NATIVE_STOP:
                robot_parar();
                break;
            case OP_NATIVE_READ_SENSOR: {
                Value sensor_id = pop(vm);
                if(vm->ip == NULL) return INTERPRET_RUNTIME_ERROR;
                int resultado = robot_leer_sensor((int)sensor_id);
                push(vm, (Value)resultado);
                break;
            }
                
            default:
                runtimeError(vm, "Opcode desconocido: %d", instruction);
                break;
        } 
    } 
    
    return INTERPRET_RUNTIME_ERROR;
}