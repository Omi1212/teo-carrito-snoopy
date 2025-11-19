/* ================================================================== */
/* =                 semantic_analyzer.c                            = */
/* =    (CORREGIDO - NOMBRES DE OPERADORES EN INGLES UNIFICADOS)    = */
/* ================================================================== */

#include "semantic_analyzer.h"
#include <stdio.h>
#include <stdlib.h>

/* Variable global simple para rastrear errores */
int hay_errores_semanticos = 0;

/* --- Prototipo de la función recursiva principal --- */
TipoDato analizar(Node* nodo, SymbolTable* tabla);

/* --- Implementación de la función de reporte de errores --- */
void reportarErrorSemantico(const char* mensaje, int lineno) {
    if (lineno > 0) {
        fprintf(stderr, "Error Semantico (Linea %d): %s\n", lineno, mensaje);
    } else {
        fprintf(stderr, "Error Semantico: %s\n", mensaje);
    }
    hay_errores_semanticos = 1;
}

/* --- Funciones "Built-in" (Funciones de robot) --- */
void inicializarTablaConBuiltins(SymbolTable* tabla) {
    declararSimbolo(tabla, "mover_adelante", TIPO_NULO, 0, 1, 0);
    declararSimbolo(tabla, "mover_atras", TIPO_NULO, 0, 1, 0);
    declararSimbolo(tabla, "parar", TIPO_NULO, 0, 1, 0);
    declararSimbolo(tabla, "girar_izquierda", TIPO_NULO, 0, 1, 0);
    declararSimbolo(tabla, "girar_derecha", TIPO_NULO, 0, 1, 0);
    declararSimbolo(tabla, "curva_izquierda", TIPO_NULO, 0, 1, 0);
    declararSimbolo(tabla, "curva_derecha", TIPO_NULO, 0, 1, 0);
    declararSimbolo(tabla, "esperar", TIPO_NULO, 0, 1, 1);
    declararSimbolo(tabla, "motor_derecho", TIPO_NULO, 0, 1, 1);
    declararSimbolo(tabla, "motor_izquierdo", TIPO_NULO, 0, 1, 1);
    declararSimbolo(tabla, "leer_sensor", TIPO_NUMERO, 0, 1, 1);
}


/* --- Función de Arranque --- */
void analizarSemantica(Node* nodo_raiz) {
    if (nodo_raiz == NULL) return;
    
    hay_errores_semanticos = 0;
    SymbolTable* tabla = crearTabla();
    inicializarTablaConBuiltins(tabla);
    analizar(nodo_raiz, tabla);
    destruirTabla(tabla);
    
    if (hay_errores_semanticos) {
        printf("\nAnalisis semantico finalizado con errores.\n");
    } else {
        printf("\nAnalisis semantico finalizado exitosamente.\n");
    }
}

/* --- Función de Ayuda: Conteo de Lista --- */
int contarNodosLista(Node* nodo_lista) {
    int count = 0;
    while (nodo_lista != NULL && nodo_lista->type != NODE_NULO) {
        count++;
        nodo_lista = nodo_lista->data.lista.nodo_siguiente;
    }
    return count;
}


/* --- Función Recursiva Principal de Análisis --- */
TipoDato analizar(Node* nodo, SymbolTable* tabla) {
    
    if (nodo == NULL) {
        return TIPO_NULO;
    }
    
    switch (nodo->type) {
        
        case NODE_PROGRAMA:
            analizar(nodo->data.programa.lista_funciones, tabla);
            analizar(nodo->data.programa.bloque_principal, tabla);
            break;
            
        case NODE_LISTA:
            analizar(nodo->data.lista.nodo_actual, tabla);
            analizar(nodo->data.lista.nodo_siguiente, tabla);
            break;

        case NODE_NULO:
            return TIPO_NULO;
        
        case NODE_DEFINICION_FUNCION: {
            int num_params = contarNodosLista(nodo->data.def_funcion.parametros);
            declararSimbolo(tabla, nodo->data.def_funcion.nombre, 
                            TIPO_FUNCION, 0, 1, num_params);
            entrarAlcance(tabla);
            analizar(nodo->data.def_funcion.parametros, tabla);
            analizar(nodo->data.def_funcion.bloque, tabla);
            salirAlcance(tabla);
            break;
        }

        case NODE_PARAMETRO: {
            TipoDato tipo_param = analizar(nodo->data.parametro.tipo, tabla);
            declararSimbolo(tabla, nodo->data.parametro.nombre, tipo_param, 0, 0, 0);
            break;
        }

        case NODE_DECLARACION: {
            TipoDato tipo_declarado = analizar(nodo->data.declaracion.tipo, tabla);
            
            if (nodo->data.declaracion.inicializador->type != NODE_NULO) {
                TipoDato tipo_inicializador = analizar(nodo->data.declaracion.inicializador, tabla);
                if (tipo_declarado != tipo_inicializador) {
                    reportarErrorSemantico("Tipo de inicializador no coincide con el tipo declarado", nodo->lineno);
                }
            }
            declararSimbolo(tabla, nodo->data.declaracion.nombre, 
                            tipo_declarado, nodo->data.declaracion.es_constante, 0, 0);
            break;
        }
        
        case NODE_SI:
        case NODE_MIENTRAS: {
            TipoDato tipo_cond;
            if (nodo->type == NODE_SI)
                tipo_cond = analizar(nodo->data.si.condicion, tabla);
            else
                tipo_cond = analizar(nodo->data.mientras.condicion, tabla);

            if (tipo_cond != TIPO_BOOLEANO) {
                reportarErrorSemantico("La condicion debe ser de tipo Booleano", nodo->lineno);
            }
            
            entrarAlcance(tabla);
            if (nodo->type == NODE_SI)
                analizar(nodo->data.si.bloque_si, tabla);
            else
                analizar(nodo->data.mientras.bloque, tabla);
            salirAlcance(tabla);
            
            if (nodo->type == NODE_SI && nodo->data.si.bloque_sino->type != NODE_NULO) {
                entrarAlcance(tabla);
                analizar(nodo->data.si.bloque_sino, tabla);
                salirAlcance(tabla);
            }
            break;
        }
        
        case NODE_PARA:
            entrarAlcance(tabla);
            analizar(nodo->data.para.inicializacion, tabla);
            TipoDato tipo_cond = analizar(nodo->data.para.condicion, tabla);
            if (tipo_cond != TIPO_BOOLEANO) {
                reportarErrorSemantico("La condicion del 'para' debe ser de tipo Booleano", nodo->lineno);
            }
            analizar(nodo->data.para.incremento, tabla);
            analizar(nodo->data.para.bloque, tabla);
            salirAlcance(tabla);
            break;
            
        case NODE_ASIGNACION: {
            Symbol* sym = buscarSimbolo(tabla, nodo->data.asignacion.nombre);
            if (sym == NULL) {
                reportarErrorSemantico("Variable no declarada", nodo->lineno);
                return TIPO_ERROR;
            }
            if (sym->es_constante) {
                reportarErrorSemantico("Intento de asignacion a una CONSTANTE", nodo->lineno);
            }
            TipoDato tipo_expr = analizar(nodo->data.asignacion.expresion, tabla);
            if (sym->tipo != tipo_expr && tipo_expr != TIPO_ERROR) {
                reportarErrorSemantico("Tipo de asignacion incompatible", nodo->lineno);
            }
            return sym->tipo;
        }

        case NODE_LLAMADA_FUNCION: {
            Symbol* sym = buscarSimbolo(tabla, nodo->data.llamada_funcion.nombre);
            if (sym == NULL) {
                reportarErrorSemantico("Funcion no declarada", nodo->lineno);
                return TIPO_ERROR;
            }
            if (!sym->es_funcion) {
                reportarErrorSemantico("El simbolo no es una funcion", nodo->lineno);
                return TIPO_ERROR;
            }
            
            int num_args_llamada = contarNodosLista(nodo->data.llamada_funcion.argumentos);
            if (num_args_llamada != sym->num_params) {
                char msg[256];
                sprintf(msg, "La funcion '%s' esperaba %d argumentos, pero recibio %d", 
                        sym->nombre, sym->num_params, num_args_llamada);
                reportarErrorSemantico(msg, nodo->lineno);
            }
            
            analizar(nodo->data.llamada_funcion.argumentos, tabla);
            return sym->tipo;
        }
        
        case NODE_BINARIA: {
            TipoDato tipo_izq = analizar(nodo->data.binaria.izquierda, tabla);
            TipoDato tipo_der = analizar(nodo->data.binaria.derecha, tabla);
            
            if (tipo_izq == TIPO_ERROR || tipo_der == TIPO_ERROR) return TIPO_ERROR;
            
            /* ================== ¡CAMBIO AQUÍ! ================== */
            /* Estos 'case' ahora coinciden con ast.h y parser.y */
            switch (nodo->data.binaria.op) {
                /* Operadores Aritméticos: Nro op Nro -> Nro */
                case AST_OP_ADD:
                case AST_OP_SUB:
                case AST_OP_MUL:
                case AST_OP_DIV:
                case AST_OP_MOD:
                case AST_OP_POW:
                    if (tipo_izq != TIPO_NUMERO || tipo_der != TIPO_NUMERO) {
                        reportarErrorSemantico("Operadores aritmeticos requieren tipo Numero", nodo->lineno);
                        return TIPO_ERROR;
                    }
                    return TIPO_NUMERO;

                /* Operadores Lógicos: Bool op Bool -> Bool */
                case AST_OP_AND:
                case AST_OP_OR:
                    if (tipo_izq != TIPO_BOOLEANO || tipo_der != TIPO_BOOLEANO) {
                        reportarErrorSemantico("Operadores logicos requieren tipo Booleano", nodo->lineno);
                        return TIPO_ERROR;
                    }
                    return TIPO_BOOLEANO;

                /* Operadores Relacionales: Tipo op Tipo -> Bool */
                case AST_OP_EQUAL:
                case AST_OP_NOT_EQUAL:
                case AST_OP_IS_NULL:
                    if (tipo_izq != tipo_der) {
                        reportarErrorSemantico("Comparacion de igualdad entre tipos incompatibles", nodo->lineno);
                        return TIPO_ERROR;
                    }
                    return TIPO_BOOLEANO;
                    
                case AST_OP_GREATER:
                case AST_OP_LESS:
                case AST_OP_GREATER_EQUAL:
                case AST_OP_LESS_EQUAL:
                    if (tipo_izq != TIPO_NUMERO || tipo_der != TIPO_NUMERO) {
                        reportarErrorSemantico("Operadores de comparacion requieren tipo Numero", nodo->lineno);
                        return TIPO_ERROR;
                    }
                    return TIPO_BOOLEANO;
                
                default:
                    /* Este 'default' captura cualquier 'case' que falte */
                    reportarErrorSemantico("Operador binario desconocido en el analizador semantico", nodo->lineno);
                    return TIPO_ERROR;
            }
            break; /* Inalcanzable, pero es buena práctica */
        }

        case NODE_UNARIA: {
            TipoDato tipo_op = analizar(nodo->data.unaria.operando, tabla);
            if (tipo_op == TIPO_ERROR) return TIPO_ERROR;
            
            /* ================== ¡CAMBIO AQUÍ! ================== */
            /* Estos 'case' ahora coinciden con ast.h y parser.y */
            switch (nodo->data.unaria.op) {
                case AST_OP_NOT:
                    if (tipo_op != TIPO_BOOLEANO) {
                        reportarErrorSemantico("Operador '!' requiere tipo Booleano", nodo->lineno);
                        return TIPO_ERROR;
                    }
                    return TIPO_BOOLEANO;
                case AST_OP_NEGATE:
                case AST_OP_UNARY_PLUS:
                    if (tipo_op != TIPO_NUMERO) {
                        reportarErrorSemantico("Operador unario (+/-) requiere tipo Numero", nodo->lineno);
                        return TIPO_ERROR;
                    }
                    return TIPO_NUMERO;
                
                default:
                    reportarErrorSemantico("Operador unario desconocido en el analizador semantico", nodo->lineno);
                    return TIPO_ERROR;
            }
            break; /* Inalcanzable */
        }
        
        case NODE_IDENT: {
            Symbol* sym = buscarSimbolo(tabla, nodo->data.ident);
            if (sym == NULL) {
                char msg[256];
                sprintf(msg, "Simbolo '%s' no ha sido declarado", nodo->data.ident);
                reportarErrorSemantico(msg, nodo->lineno);
                return TIPO_ERROR;
            }
            return sym->tipo;
        }
        
        case NODE_NUMERO:   return TIPO_NUMERO;
        case NODE_CARACTER: return TIPO_CARACTER;
        case NODE_BOOLEANO: return TIPO_BOOLEANO;
        case NODE_TIPO:     return nodo->data.tipo_dato.tipo;
            
        default:
            fprintf(stderr, "Alerta: Nodo de AST no manejado en el analizador semantico (Tipo: %d)\n", nodo->type);
            break;
    }
    
    return TIPO_ERROR;
}