/* ================================================================== */
/* =                       bytecode.h                               = */
/* =    Define la estructura dinámica para almacenar el Bytecode     = */
/* ================================================================== */

#ifndef BYTECODE_H
#define BYTECODE_H

#include "vm.h" /* Incluimos vm.h por 'byte' y 'Value' */

/**
 * @brief Un "chunk" dinámico de bytecode.
 * El compilador usa esto para construir el programa.
 */
typedef struct {
    int count;      /* Número de bytes en uso */
    int capacity;   /* Número de bytes alocados */
    byte* code;     /* El array de bytecode */
    
    /* Pool de Constantes (dinámico para el compilador) */
    int constants_count;
    int constants_capacity;
    Value* constants_pool;

} BytecodeChunk;

/**
 * @brief Inicializa un BytecodeChunk vacío.
 */
void initChunk(BytecodeChunk* chunk);

/**
 * @brief Libera toda la memoria asociada al chunk.
 */
void freeChunk(BytecodeChunk* chunk);

/**
 * @brief Escribe un solo byte al final del chunk.
 */
void writeChunk(BytecodeChunk* chunk, byte b);

/**
 * @brief Añade un valor constante al pool de constantes.
 * @param chunk El chunk.
 * @param valor El valor (double) a añadir.
 * @return El índice (int) donde se añadió la constante.
 */
int addConstant(BytecodeChunk* chunk, Value valor);

#endif /* BYTECODE_H */