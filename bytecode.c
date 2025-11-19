/* ================================================================== */
/* =                       bytecode.c                               = */
/* =   Implementación del contenedor de Bytecode (Arrays dinámicos)  = */
/* ================================================================== */

#include "bytecode.h"
#include <stdlib.h>
#include <string.h>

void initChunk(BytecodeChunk* chunk) {
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    chunk->constants_count = 0;
    chunk->constants_capacity = 0;
    chunk->constants_pool = NULL;
}

void freeChunk(BytecodeChunk* chunk) {
    free(chunk->code);
    free(chunk->constants_pool);
    /* Resetea el chunk a un estado inicial válido */
    initChunk(chunk);
}

/**
 * @brief Función interna para crecer un array dinámico.
 */
static void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    if (newSize == 0) {
        free(pointer);
        return NULL;
    }
    /* realloc maneja el caso donde 'pointer' (oldSize) es NULL (0) */
    void* result = realloc(pointer, newSize);
    if (result == NULL) {
        /* ¡Error fatal en un compilador real! */
        exit(1); 
    }
    return result;
}

void writeChunk(BytecodeChunk* chunk, byte b) {
    /* Chequear si necesitamos más capacidad */
    if (chunk->capacity < chunk->count + 1) {
        int oldCapacity = chunk->capacity;
        chunk->capacity = (oldCapacity < 8) ? 8 : oldCapacity * 2;
        chunk->code = (byte*)reallocate(chunk->code, 
                                        oldCapacity * sizeof(byte), 
                                        chunk->capacity * sizeof(byte));
    }
    
    chunk->code[chunk->count] = b;
    chunk->count++;
}

int addConstant(BytecodeChunk* chunk, Value valor) {
    /* Chequear si necesitamos más capacidad en el pool de constantes */
    if (chunk->constants_capacity < chunk->constants_count + 1) {
        int oldCapacity = chunk->constants_capacity;
        chunk->constants_capacity = (oldCapacity < 8) ? 8 : oldCapacity * 2;
        chunk->constants_pool = (Value*)reallocate(chunk->constants_pool,
                                            oldCapacity * sizeof(Value),
                                            chunk->constants_capacity * sizeof(Value));
    }

    chunk->constants_pool[chunk->constants_count] = valor;
    chunk->constants_count++;
    
    /* Devuelve el índice del valor que acabamos de añadir */
    return chunk->constants_count - 1;
}