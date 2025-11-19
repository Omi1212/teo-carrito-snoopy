/* ================================================================== */
/* =                         main.c                                 = */
/* =    (CORREGIDO - Ahora comprueba el error de la Fase 3)         = */
/* ================================================================== */

#include <stdio.h>
#include <stdlib.h>

#include "ast.h"
#include "semantic_analyzer.h" /* Para 'analizarSemantica' */
#include "compiler.h"          /* Para 'compilar' */
#include "bytecode.h"          /* Para 'BytecodeChunk' */
#include "vm.h"                /* Para la 'VM' */

/* --- Prototipos de Bison y Flex --- */
extern int yyparse(void);       // Función de Bison
extern FILE* yyin;              // Entrada de Flex
extern Node* ast_root;          // Variable global de parser.y
extern int yylineno;            // Contador de línea de Flex
extern void yyerror(const char* s); // Función de error

/* Variable global de semantic_analyzer.c */
extern int hay_errores_semanticos; 

/* Implementación simple de yyerror (requerida por Bison/Flex) */
void yyerror(const char* s) {
    fprintf(stderr, "Error Sintactico: %s en la linea %d\n", s, yylineno);
}


int main(int argc, char** argv) {
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <archivo_fuente.txt>\n", argv[0]);
        return 1;
    }

    FILE* file = fopen(argv[1], "r");
    if (!file) {
        perror("Error al abrir el archivo");
        return 1;
    }
    
    /* 1. Apunta el lexer (Flex) al archivo de entrada */
    yyin = file;
    
    printf("--- FASE 1: Parseo y creacion de AST ---\n");
    if (yyparse() != 0) {
        fprintf(stderr, "Error de parseo. Compilacion detenida.\n");
        fclose(file);
        return 1;
    }
    printf("Parseo exitoso. AST creado.\n\n");
    
    /* 2. Análisis Semántico (usando el AST de 'ast_root') */
    printf("--- FASE 2: Analisis Semantico ---\n");
    analizarSemantica(ast_root);
    
    if (hay_errores_semanticos) {
        printf("Compilacion detenida debido a errores semanticos.\n");
        fclose(file);
        return 1;
    }
    printf("Analisis semantico completado.\n\n");

    /* 3. Compilación a Bytecode (recorriendo el AST) */
    BytecodeChunk chunk;
    initChunk(&chunk);
    printf("--- FASE 3: Generacion de Bytecode ---\n");
    
    if (compilar(ast_root, &chunk) != 0) {
        fprintf(stderr, "Error de compilacion. Generacion detenida.\n");
        fclose(file);
        return 1; /* ¡Detener si la Fase 3 reporta un error! */
    }
    printf("Bytecode generado (%d bytes).\n\n", chunk.count);

    /* 4. Ejecución en la Máquina Virtual (VM) */
    VM vm;
    vm_init(&vm);
    
    /* Cargar el pool de constantes del chunk a la VM */
    for(int i=0; i < chunk.constants_count; i++) {
        vm_add_constant(&vm, chunk.constants_pool[i]);
    }
    
    /* Cargar el bytecode en la VM */
    vm_load_bytecode(&vm, chunk.code);

    printf("--- FASE 4: Ejecutando VM del Robot ---\n");
    InterpretResult result = vm_interpretar(&vm);
    
    if (result == INTERPRET_RUNTIME_ERROR) {
         printf("\n--- Error en tiempo de ejecucion ---\n");
    } else {
         printf("\n--- Ejecucion finalizada ---\n");
    }

    /* 5. Limpieza */
    freeChunk(&chunk);
    fclose(file);
    
    return 0;
}