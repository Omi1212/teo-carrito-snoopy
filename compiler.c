/* ================================================================== */
/* =                       compiler.c                               = */
/* =    (CORREGIDO - Compatible con Mac, sin warnings)              = */
/* ================================================================== */

#include "compiler.h"
#include "vm.h" 
#include <stdio.h>
#include <stdlib.h> 
#include <string.h> 

typedef struct {
    BytecodeChunk* chunk;
    int error;
    struct {
        char* nombre;
        bool es_const;
    } globales[GLOBALS_MAX];
    int global_count;
    struct {
        char* nombre;
        int address;
    } functions[FRAMES_MAX];
    int function_count;
} CompilerState;

static CompilerState compiler;
static void compilarNodo(Node* nodo);

static void emitByte(byte b) {
    writeChunk(compiler.chunk, b);
}

static void emitBytes(byte b1, byte b2) {
    emitByte(b1);
    emitByte(b2);
}

static void emitConstant(Value valor) {
    int const_index = addConstant(compiler.chunk, valor);
    if (const_index > 255) {
        fprintf(stderr, "Error de compilacion: Demasiadas constantes.\n");
        compiler.error = 1;
        return;
    }
    emitBytes(OP_PUSH_CONST, (byte)const_index);
}

static int emitJump(byte op) {
    emitByte(op);
    emitByte(0xFF);
    emitByte(0xFF);
    return compiler.chunk->count - 2; 
}

static void patchJump(int offset) {
    int jump = compiler.chunk->count - offset - 2;
    if (jump > UINT16_MAX) {
        fprintf(stderr, "Error de compilacion: Salto demasiado largo.\n");
        compiler.error = 1;
        return;
    }
    compiler.chunk->code[offset] = (jump >> 8) & 0xFF;
    compiler.chunk->code[offset + 1] = jump & 0xFF;
}

static int resolveGlobal(const char* nombre) {
    for (int i = 0; i < compiler.global_count; i++) {
        if (strcmp(compiler.globales[i].nombre, nombre) == 0) {
            return i;
        }
    }
    /* Si no se encuentra, devuelve un índice inválido */
    fprintf(stderr, "Error de compilacion: Variable no resuelta '%s'.\n", nombre);
    compiler.error = 1;
    return -1;
}

static int declareGlobal(const char* nombre, bool es_const) {
    if (compiler.global_count >= GLOBALS_MAX) {
        fprintf(stderr, "Error de compilacion: Demasiadas variables globales.\n");
        compiler.error = 1;
        return -1;
    }
    compiler.globales[compiler.global_count].nombre = (char*)nombre;
    compiler.globales[compiler.global_count].es_const = es_const;
    return compiler.global_count++;
}


static int declareFunction(const char* nombre) {
    if (compiler.function_count >= FRAMES_MAX) {
        fprintf(stderr, "Error de compilacion: Demasiadas funciones.\n");
        compiler.error = 1;
        return -1;
    }
    compiler.functions[compiler.function_count].nombre = (char*)nombre;
    compiler.functions[compiler.function_count].address = -1;
    return compiler.function_count++;
}

static void defineFunction(int index) {
    compiler.functions[index].address = compiler.chunk->count;
}

static int resolveFunction(const char* nombre) {
    for (int i = 0; i < compiler.function_count; i++) {
        if (strcmp(compiler.functions[i].nombre, nombre) == 0) {
            if (compiler.functions[i].address == -1) {
                 fprintf(stderr, "Error: Funcion '%s' declarada pero no definida.\n", nombre);
                 compiler.error = 1;
            }
            return compiler.functions[i].address;
        }
    }
    return -1;
}


static void compilarNodo(Node* nodo) {
    if (nodo == NULL || nodo->type == NODE_NULO) {
        return;
    }

    switch (nodo->type) {
        
        case NODE_PROGRAMA: {
            Node* func_list = nodo->data.programa.lista_funciones;
            while(func_list && func_list->type != NODE_NULO) {
                declareFunction(func_list->data.lista.nodo_actual->data.def_funcion.nombre);
                func_list = func_list->data.lista.nodo_siguiente;
            }
            compilarNodo(nodo->data.programa.lista_funciones);
            compilarNodo(nodo->data.programa.bloque_principal);
            emitByte(OP_HALT);
            break;
        }
        
        case NODE_LISTA:
            compilarNodo(nodo->data.lista.nodo_actual);
            
            if (nodo->data.lista.nodo_actual->type == NODE_LLAMADA_FUNCION) {
                const char* nombre = nodo->data.lista.nodo_actual->data.llamada_funcion.nombre;
                if (strcmp(nombre, "leer_sensor") != 0) {
                     if (resolveFunction(nombre) != -1) {
                         emitByte(OP_POP);
                     }
                } else {
                    emitByte(OP_POP);
                }
            } else if (nodo->data.lista.nodo_actual->type == NODE_BINARIA ||
                       nodo->data.lista.nodo_actual->type == NODE_IDENT ||
                       nodo->data.lista.nodo_actual->type == NODE_NUMERO) {
                emitByte(OP_POP);
            }
            compilarNodo(nodo->data.lista.nodo_siguiente);
            break;
            
        case NODE_DEFINICION_FUNCION: {
            int func_index = -1;
            for(int i=0; i < compiler.function_count; i++) {
                if(strcmp(compiler.functions[i].nombre, nodo->data.def_funcion.nombre) == 0) {
                    func_index = i;
                    break;
                }
            }
            int jump = emitJump(OP_JUMP);
            defineFunction(func_index);
            compilarNodo(nodo->data.def_funcion.bloque);
            emitConstant(0.0);
            emitByte(OP_RETURN);
            patchJump(jump);
            break;
        }

        case NODE_NUMERO:
            /* CAMBIO: Usamos val_num en lugar de numero */
            emitConstant(atof(nodo->data.val_num));
            break;
        case NODE_BOOLEANO:
            emitConstant(nodo->data.booleano ? 1.0 : 0.0);
            break;
        case NODE_CARACTER:
            emitConstant(0.0);
            break;
            
        case NODE_BINARIA:
            compilarNodo(nodo->data.binaria.izquierda);
            compilarNodo(nodo->data.binaria.derecha);
            
            switch (nodo->data.binaria.op) {
                case AST_OP_ADD:    emitByte(OP_ADD); break;
                case AST_OP_SUB:    emitByte(OP_SUB); break;
                case AST_OP_MUL:    emitByte(OP_MUL); break;
                case AST_OP_DIV:    emitByte(OP_DIV); break;
                case AST_OP_EQUAL:  emitByte(OP_EQUAL); break;
                case AST_OP_GREATER:emitByte(OP_GREATER); break;
                case AST_OP_LESS:   emitByte(OP_LESS); break;
                case AST_OP_AND:    emitByte(OP_AND); break;
                case AST_OP_OR:     emitByte(OP_OR); break;
                
                case AST_OP_MOD:
                case AST_OP_POW:
                case AST_OP_NOT_EQUAL:
                case AST_OP_IS_NULL:
                case AST_OP_GREATER_EQUAL:
                case AST_OP_LESS_EQUAL:
                    fprintf(stderr, "Error: Operador binario no soportado por la VM aun (%d).\n", nodo->data.binaria.op);
                    compiler.error = 1;
                    break;
                default:
                    fprintf(stderr, "Error: Operador binario desconocido.\n");
                    compiler.error = 1;
            }
            break;

        case NODE_UNARIA:
            compilarNodo(nodo->data.unaria.operando);
            switch (nodo->data.unaria.op) {
                case AST_OP_NOT:     emitByte(OP_NOT); break;
                case AST_OP_NEGATE:  emitByte(OP_NEGATE); break;
                case AST_OP_UNARY_PLUS: break;
                default:
                    fprintf(stderr, "Error: Operador unario desconocido.\n");
                    compiler.error = 1;
            }
            break;
            
        case NODE_DECLARACION: {
            if (nodo->data.declaracion.inicializador->type != NODE_NULO) {
                compilarNodo(nodo->data.declaracion.inicializador);
            } else {
                emitConstant(0.0); 
            }
            int index = declareGlobal(nodo->data.declaracion.nombre, 
                                      nodo->data.declaracion.es_constante);
            if (index == -1) break; 
            emitBytes(OP_SET_GLOBAL, (byte)index);
            break;
        }

        case NODE_ASIGNACION: {
            compilarNodo(nodo->data.asignacion.expresion);
            int idx_asig = resolveGlobal(nodo->data.asignacion.nombre);
            if (idx_asig == -1) break; 
            emitBytes(OP_SET_GLOBAL, (byte)idx_asig);
            break;
        }
            
        case NODE_IDENT: {
            int idx_get = resolveGlobal(nodo->data.ident);
            if (idx_get == -1) break; 
            emitBytes(OP_GET_GLOBAL, (byte)idx_get);
            break;
        }
            
        case NODE_SI:
            compilarNodo(nodo->data.si.condicion);
            int salto_falso = emitJump(OP_JUMP_IF_FALSE);
            compilarNodo(nodo->data.si.bloque_si);
            
            if (nodo->data.si.bloque_sino->type != NODE_NULO) {
                int salto_fin = emitJump(OP_JUMP);
                patchJump(salto_falso);
                compilarNodo(nodo->data.si.bloque_sino);
                patchJump(salto_fin);
            } else {
                patchJump(salto_falso);
            }
            break;
            
        case NODE_MIENTRAS: {
            int inicio_loop = compiler.chunk->count;
            compilarNodo(nodo->data.mientras.condicion);
            int salto_salida = emitJump(OP_JUMP_IF_FALSE);
            compilarNodo(nodo->data.mientras.bloque);
            int offset_atras = compiler.chunk->count - inicio_loop + 3;
            emitByte(OP_LOOP);
            emitByte((offset_atras >> 8) & 0xFF);
            emitByte(offset_atras & 0xFF);
            patchJump(salto_salida);
            break;
        }

        case NODE_LLAMADA_FUNCION: {
            const char* nombre = nodo->data.llamada_funcion.nombre;
            
            Node* args = nodo->data.llamada_funcion.argumentos;
            int arg_count = 0; 
            while(args && args->type != NODE_NULO) {
                compilarNodo(args->data.lista.nodo_actual);
                arg_count++;
                args = args->data.lista.nodo_siguiente;
            }

            if (strcmp(nombre, "mover_adelante") == 0) emitByte(OP_NATIVE_MOVE_FWD);
            else if (strcmp(nombre, "mover_atras") == 0) emitByte(OP_NATIVE_MOVE_BWD);
            else if (strcmp(nombre, "parar") == 0) emitByte(OP_NATIVE_STOP);
            else if (strcmp(nombre, "esperar") == 0) emitByte(OP_NATIVE_WAIT);
            else if (strcmp(nombre, "girar_izquierda") == 0) emitByte(OP_NATIVE_TURN_LEFT);
            else if (strcmp(nombre, "girar_derecha") == 0) emitByte(OP_NATIVE_TURN_RIGHT);
            else if (strcmp(nombre, "curva_izquierda") == 0) emitByte(OP_NATIVE_CURVE_LEFT);
            else if (strcmp(nombre, "curva_derecha") == 0) emitByte(OP_NATIVE_CURVE_RIGHT);
            else if (strcmp(nombre, "motor_derecho") == 0) emitByte(OP_NATIVE_MOTOR_RIGHT);
            else if (strcmp(nombre, "motor_izquierdo") == 0) emitByte(OP_NATIVE_MOTOR_LEFT);
            else if (strcmp(nombre, "leer_sensor") == 0) emitByte(OP_NATIVE_READ_SENSOR);
            else {
                int func_addr = resolveFunction(nombre);
                if (func_addr == -1) {
                    fprintf(stderr, "Error: Funcion '%s' no encontrada.\n", nombre);
                    compiler.error = 1;
                    break;
                }
                emitConstant((Value)func_addr);
                emitBytes(OP_CALL, (byte)arg_count);
            }
            break;
        }
            
        case NODE_PARAMETRO:
        case NODE_TIPO:
            break;
            
        default:
            fprintf(stderr, "Error de compilacion: Nodo AST desconocido (%d).\n", nodo->type);
            compiler.error = 1;
            break;
    }
}


int compilar(Node* nodo_raiz, BytecodeChunk* chunk) {
    compiler.chunk = chunk;
    compiler.global_count = 0;
    compiler.function_count = 0;
    compiler.error = 0;
    
    printf("--- Iniciando Compilacion a Bytecode ---\n");
    compilarNodo(nodo_raiz);
    printf("--- Compilacion Finalizada ---\n");
    
    return compiler.error; 
}