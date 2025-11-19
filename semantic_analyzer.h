/* ================================================================== */
/* =                 semantic_analyzer.h                            = */
/* =    Interfaz para el Analizador Semántico (Recorrido del AST)   = */
/* ================================================================== */

#ifndef SEMANTIC_ANALYZER_H
#define SEMANTIC_ANALYZER_H

/* Incluimos las definiciones del AST (struct Node, etc.) */
#include "ast.h"
/* Incluimos la definición de la Tabla de Símbolos */
#include "symbol_table.h"

/**
 * @brief Función principal para iniciar el análisis semántico.
 * Prepara la tabla de símbolos (con 'built-ins') y comienza el
 * recorrido recursivo del AST.
 *
 * @param nodo_raiz El nodo raíz del AST generado por Bison.
 */
void analizarSemantica(Node* nodo_raiz);


/**
 * @brief Reporta un error semántico de forma centralizada.
 *
 * @param mensaje El mensaje de error a mostrar.
 * @param lineno El número de línea donde ocurrió el error (del nodo del AST).
 */
void reportarErrorSemantico(const char* mensaje, int lineno);

#endif /* SEMANTIC_ANALYZER_H */