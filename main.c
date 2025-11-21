/* ================================================================== */
/* =                         main.c                                 = */
/* =   (MODIFICADO - Genera archivo .bin para tarjeta SD)           = */
/* ================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> // Necesario para tipos de tamaño fijo (int32_t)

#include "ast.h"
#include "semantic_analyzer.h"
#include "compiler.h"
#include "bytecode.h"
#include "vm.h"

extern int yyparse(void);
extern FILE* yyin;
extern Node* ast_root;
extern int hay_errores_semanticos; 

void yyerror(const char* s) {
    fprintf(stderr, "Error Sintactico: %s\n", s);
}

// Función para exportar a binario compatible con Arduino
void guardar_binario(BytecodeChunk* chunk, const char* filename) {
    FILE* file = fopen(filename, "wb");
    if (!file) {
        perror("Error al crear el archivo binario");
        return;
    }

    // --- HEADER DE CONSTANTES ---
    // 1. Guardar Cantidad de Constantes (como int de 32 bits para compatibilidad)
    int32_t const_count = (int32_t)chunk->constants_count;
    fwrite(&const_count, sizeof(int32_t), 1, file);

    // 2. Guardar las Constantes (Convertidas a float de 4 bytes)
    printf("Exportando %d constantes...\n", const_count);
    for (int i = 0; i < chunk->constants_count; i++) {
        // En PC 'double' es 8 bytes, en Arduino es 4. Convertimos a float.
        float val = (float)chunk->constants_pool[i];
        fwrite(&val, sizeof(float), 1, file);
    }

    // --- HEADER DE CÓDIGO ---
    // 3. Guardar Cantidad de Bytes de código
    int32_t code_count = (int32_t)chunk->count;
    fwrite(&code_count, sizeof(int32_t), 1, file);

    // 4. Guardar el Bytecode puro
    printf("Exportando %d bytes de instrucciones...\n", code_count);
    fwrite(chunk->code, sizeof(byte), chunk->count, file);

    fclose(file);
    printf("\n[EXITO] Archivo '%s' generado correctamente.\n", filename);
}

int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <archivo_fuente.txt>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (!file) {
        perror("Error al abrir entrada");
        return 1;
    }
    
    yyin = file;
    
    // --- FASE 1 y 2 ---
    if (yyparse() != 0 || hay_errores_semanticos) {
        fclose(file);
        return 1;
    }
    analizarSemantica(ast_root);
    if (hay_errores_semanticos) {
        fclose(file);
        return 1;
    }

    // --- FASE 3: Compilación ---
    BytecodeChunk chunk;
    initChunk(&chunk);
    
    if (compilar(ast_root, &chunk) != 0) {
        fclose(file);
        return 1;
    }

    // --- FASE 4: Exportación a SD ---
    // Generamos el archivo .bin en lugar de ejecutar la VM localmente
    guardar_binario(&chunk, "program.bin");

    freeChunk(&chunk);
    fclose(file);
    
    return 0;
}