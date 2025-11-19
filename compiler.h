/* ================================================================== */
/* =                       compiler.h                               = */
/* =      (CORREGIDO - Cambiado a 'int' para conteo de errores)     = */
/* ================================================================== */

#ifndef COMPILER_H
#define COMPILER_H

#include "ast.h"
#include "bytecode.h"
#include <stdbool.h> 

/**
 * @return 0 si la compilación fue exitosa, 1 si hubo errores.
 */
int compilar(Node* nodo_raiz, BytecodeChunk* chunk);

#endif /* COMPILER_H */