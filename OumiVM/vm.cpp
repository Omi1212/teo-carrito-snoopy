/* ================================================================== */
/* =                          vm.cpp                                = */
/* =             (OPTIMIZADO CON MACRO F())                         = */
/* ================================================================== */

#include "vm.h"
#include "Arduino.h" 

const int AJUSTE_POTENCIA_IZQ = 20; 
const int AJUSTE_POTENCIA_DER = 0;  

const int ENA_PIN = 5;
const int IN1_PIN = 2;
const int IN2_PIN = 3;
const int ENB_PIN = 6;
const int IN3_PIN = 4;
const int IN4_PIN = 7;

void robot_parar() { 
    Serial.println(F("CMD: PARAR")); 
    digitalWrite(IN1_PIN, LOW); digitalWrite(IN2_PIN, LOW); analogWrite(ENA_PIN, 0);
    digitalWrite(IN3_PIN, LOW); digitalWrite(IN4_PIN, LOW); analogWrite(ENB_PIN, 0);
}

void robot_esperar(int ms) { 
    delay(ms); 
}

void robot_motor_izquierdo(int velocidad) {
    int pwm;
    if (velocidad > 0) {
        pwm = constrain(velocidad + AJUSTE_POTENCIA_IZQ, 0, 255); 
        digitalWrite(IN1_PIN, LOW); digitalWrite(IN2_PIN, HIGH);
    } else if (velocidad < 0) {
        pwm = constrain(abs(velocidad) + AJUSTE_POTENCIA_IZQ, 0, 255);
        digitalWrite(IN1_PIN, HIGH); digitalWrite(IN2_PIN, LOW);
    } else {
        pwm = 0;
        digitalWrite(IN1_PIN, LOW); digitalWrite(IN2_PIN, LOW);
    }
    analogWrite(ENA_PIN, pwm);
    // Serial.print(F("M_IZQ: ")); Serial.println(pwm); // Comentar para ahorrar más
}

void robot_motor_derecho(int velocidad) {
    int pwm;
    if (velocidad > 0) {
        pwm = constrain(velocidad + AJUSTE_POTENCIA_DER, 0, 255);
        digitalWrite(IN3_PIN, LOW); digitalWrite(IN4_PIN, HIGH);
    } else if (velocidad < 0) {
        pwm = constrain(abs(velocidad) + AJUSTE_POTENCIA_DER, 0, 255);
        digitalWrite(IN3_PIN, HIGH); digitalWrite(IN4_PIN, LOW);
    } else {
        pwm = 0;
        digitalWrite(IN3_PIN, LOW); digitalWrite(IN4_PIN, LOW);
    }
    analogWrite(ENB_PIN, pwm);
    // Serial.print(F("M_DER: ")); Serial.println(pwm); // Comentar para ahorrar más
}

int robot_leer_sensor(int id_sensor) { 
    int lectura = digitalRead(id_sensor);
    Serial.print(F("S_")); Serial.print(id_sensor); 
    Serial.print(F(": ")); Serial.println(lectura);
    return lectura; 
}

void robot_mover_adelante(int v) {
    Serial.println(F("FWD"));
    robot_motor_izquierdo(v); robot_motor_derecho(v);
}

void robot_mover_atras(int v) {
    Serial.println(F("BWD"));
    robot_motor_izquierdo(-v); robot_motor_derecho(-v);
}

void robot_girar_izquierda(int v) {
    Serial.println(F("LEFT"));
    robot_motor_izquierdo(-v); robot_motor_derecho(v);
}

void robot_girar_derecha(int v) {
    Serial.println(F("RIGHT"));
    robot_motor_izquierdo(v); robot_motor_derecho(-v);
}

void robot_curva_izquierda(int v) {
    Serial.println(F("C_LEFT"));
    robot_motor_izquierdo(v / 3); robot_motor_derecho(v);
}

void robot_curva_derecha(int v) {
    Serial.println(F("C_RIGHT"));
    robot_motor_izquierdo(v); robot_motor_derecho(v / 3);
}

static void runtimeError(VM* vm, const char* msg) {
    Serial.print(F("Err: ")); Serial.println(msg); vm->ip = NULL; 
}
static void push(VM* vm, Value value) {
    if (vm->ip == NULL) return;
    if (vm->stackTop - vm->stack >= STACK_MAX) { runtimeError(vm, "Stack ovf"); return; }
    *vm->stackTop = value; vm->stackTop++;
}
static Value pop(VM* vm) {
    if (vm->ip == NULL) return 0.0;
    if (vm->stackTop == vm->stack) { runtimeError(vm, "Stack emp"); return 0.0; }
    vm->stackTop--; return *vm->stackTop;
}
static Value peek(VM* vm, int distance) {
    if (vm->ip == NULL) return 0.0;
    if (vm->stackTop - vm->stack <= distance) { runtimeError(vm, "Stack und"); return 0.0; }
    return vm->stackTop[-1 - distance];
}
static bool isFalsy(Value value) { return value == 0.0; }

void vm_init(VM* vm) { memset(vm, 0, sizeof(VM)); vm->stackTop = vm->stack; }
void vm_load_bytecode(VM* vm, byte* bytecode) { vm->bytecode_start = bytecode; vm->ip = bytecode; }
int vm_add_constant(VM* vm, Value value) {
    if (vm->constantCount >= CONST_POOL_MAX) return -1; 
    vm->constantPool[vm->constantCount] = value; return vm->constantCount++;
}

InterpretResult vm_interpretar(VM* vm) {
    #define READ_BYTE() (*vm->ip++)
    #define READ_CONST() (vm->constantPool[READ_BYTE()])
    #define READ_SHORT() (vm->ip += 2, (uint16_t)((vm->ip[-2] << 8) | vm->ip[-1]))
    #define BINARY_OP(op) do { Value b = pop(vm); Value a = pop(vm); if (vm->ip == NULL) return INTERPRET_RUNTIME_ERROR; push(vm, a op b); } while (0)
    #define NATIVE_CALL_1_VOID(func) do { Value arg = pop(vm); if (vm->ip == NULL) return INTERPRET_RUNTIME_ERROR; func((int)arg); } while (0)

    for (;;) {
        if(vm->ip == NULL) return INTERPRET_RUNTIME_ERROR;
        byte instruction = READ_BYTE();
        switch (instruction) {
            case OP_HALT: return INTERPRET_OK;
            case OP_PUSH_CONST: push(vm, READ_CONST()); break;
            case OP_POP: pop(vm); break;
            case OP_ADD: BINARY_OP(+); break;
            case OP_SUB: BINARY_OP(-); break;
            case OP_MUL: BINARY_OP(*); break;
            case OP_DIV: BINARY_OP(/); break;
            case OP_NEGATE: { Value val = pop(vm); if(vm->ip) push(vm, -val); break; }
            case OP_NOT: { Value val = pop(vm); if(vm->ip) push(vm, isFalsy(val) ? 1.0 : 0.0); break; }
            case OP_EQUAL: { Value b = pop(vm); Value a = pop(vm); if(vm->ip) push(vm, (a == b) ? 1.0 : 0.0); break; }
            case OP_GREATER: BINARY_OP(>); break;
            case OP_LESS: BINARY_OP(<); break;
            case OP_AND: { Value b = pop(vm); Value a = pop(vm); if(vm->ip) push(vm, (!isFalsy(a) && !isFalsy(b)) ? 1.0 : 0.0); break; }
            case OP_OR: { Value b = pop(vm); Value a = pop(vm); if(vm->ip) push(vm, (!isFalsy(a) || !isFalsy(b)) ? 1.0 : 0.0); break; }
            case OP_SET_GLOBAL: { byte idx = READ_BYTE(); vm->globals[idx] = pop(vm); break; }
            case OP_GET_GLOBAL: { byte idx = READ_BYTE(); push(vm, vm->globals[idx]); break; }
            case OP_SET_LOCAL: { byte slot = READ_BYTE(); vm->frames[vm->frameCount-1].slots[slot] = peek(vm, 0); break; }
            case OP_GET_LOCAL: { byte slot = READ_BYTE(); push(vm, vm->frames[vm->frameCount-1].slots[slot]); break; }
            case OP_JUMP: { uint16_t off = READ_SHORT(); vm->ip += off; break; }
            case OP_JUMP_IF_FALSE: { uint16_t off = READ_SHORT(); if(isFalsy(pop(vm))) vm->ip += off; break; }
            case OP_LOOP: { uint16_t off = READ_SHORT(); vm->ip -= off; break; }
            case OP_CALL: {
                byte args = READ_BYTE(); Value addr = peek(vm, args);
                if(vm->ip == NULL || vm->frameCount == FRAMES_MAX) return INTERPRET_RUNTIME_ERROR;
                CallFrame* f = &vm->frames[vm->frameCount++];
                f->returnAddress = vm->ip; f->slots = vm->stackTop - args - 1;
                vm->ip = vm->bytecode_start + (int)addr; break;
            }
            case OP_RETURN: {
                if (vm->frameCount == 0) { pop(vm); return INTERPRET_OK; }
                CallFrame* f = &vm->frames[--vm->frameCount];
                Value ret = pop(vm); vm->stackTop = f->slots; push(vm, ret);
                vm->ip = f->returnAddress; break;
            }
            case OP_NATIVE_MOVE_FWD: NATIVE_CALL_1_VOID(robot_mover_adelante); break;
            case OP_NATIVE_MOVE_BWD: NATIVE_CALL_1_VOID(robot_mover_atras); break;
            case OP_NATIVE_WAIT: NATIVE_CALL_1_VOID(robot_esperar); break;
            case OP_NATIVE_TURN_LEFT: NATIVE_CALL_1_VOID(robot_girar_izquierda); break;
            case OP_NATIVE_TURN_RIGHT: NATIVE_CALL_1_VOID(robot_girar_derecha); break;
            case OP_NATIVE_CURVE_LEFT: NATIVE_CALL_1_VOID(robot_curva_izquierda); break;
            case OP_NATIVE_CURVE_RIGHT: NATIVE_CALL_1_VOID(robot_curva_derecha); break;
            case OP_NATIVE_MOTOR_RIGHT: NATIVE_CALL_1_VOID(robot_motor_derecho); break;
            case OP_NATIVE_MOTOR_LEFT: NATIVE_CALL_1_VOID(robot_motor_izquierdo); break;
            case OP_NATIVE_STOP: robot_parar(); break;
            case OP_NATIVE_READ_SENSOR: {
                Value id = pop(vm); if(vm->ip) { int res = robot_leer_sensor((int)id); push(vm, (Value)res); }
                break;
            }
            default: runtimeError(vm, "Opcode?"); break;
        }
    }
}