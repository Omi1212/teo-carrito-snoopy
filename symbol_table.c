/* ================================================================== */
/* =                 symbol_table.c                                 = */
/* =  Implementación de la Tabla de Símbolos (Lógica de Alcance)    = */
/* ================================================================== */

#include "symbol_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Prototipo de una función de error (definida en semantic_analyzer.c) */
extern void reportarErrorSemantico(const char* mensaje, int lineno);

/* strdup es necesario (no es C99 estándar pero es universal en POSIX) */
/* Si no, debe implementarse manualmente como en el ejemplo anterior */
#ifndef _WIN32
extern char* strdup(const char*);
#endif


SymbolTable* crearTabla() {
    // 1. Crear la tabla
    SymbolTable* tabla = (SymbolTable*)malloc(sizeof(SymbolTable));
    if (!tabla) {
        perror("Error al crear SymbolTable");
        exit(1);
    }
    
    // 2. Crear el alcance global
    Scope* alcance_global = (Scope*)malloc(sizeof(Scope));
    if (!alcance_global) {
        perror("Error al crear Scope global");
        exit(1);
    }
    
    alcance_global->simbolos = NULL;
    alcance_global->padre = NULL; /* El alcance global no tiene padre */
    
    // 3. Apuntar la tabla al alcance global
    tabla->alcance_actual = alcance_global;
    
    return tabla;
}

void destruirTabla(SymbolTable* tabla) {
    /* Destruye todos los alcances, desde el actual hasta el global */
    while (tabla->alcance_actual != NULL) {
        salirAlcance(tabla);
    }
    free(tabla);
}

void entrarAlcance(SymbolTable* tabla) {
    Scope* nuevo_alcance = (Scope*)malloc(sizeof(Scope));
    if (!nuevo_alcance) {
        perror("Error al crear nuevo Scope");
        exit(1);
    }
    
    nuevo_alcance->simbolos = NULL;
    nuevo_alcance->padre = tabla->alcance_actual; /* El padre es el alcance anterior */
    tabla->alcance_actual = nuevo_alcance;      /* El actual es el nuevo */
}

void salirAlcance(SymbolTable* tabla) {
    if (tabla->alcance_actual == NULL) {
        return; /* Ya no hay alcances */
    }

    Scope* alcance_a_borrar = tabla->alcance_actual;
    
    /* 1. Mover la tabla al alcance del padre */
    tabla->alcance_actual = alcance_a_borrar->padre;
    
    /* 2. Liberar todos los símbolos en el alcance que se va */
    Symbol* simbolo = alcance_a_borrar->simbolos;
    while (simbolo != NULL) {
        Symbol* siguiente = simbolo->siguiente;
        free(simbolo->nombre); /* Liberar la copia del nombre */
        free(simbolo);
        simbolo = siguiente;
    }
    
    /* 3. Liberar el alcance en sí */
    free(alcance_a_borrar);
}

Symbol* declararSimbolo(SymbolTable* tabla, const char* nombre, TipoDato tipo, int es_const, int es_func, int num_params) {
    
    Scope* alcance = tabla->alcance_actual;
    
    /* 1. Verificar redeclaración EN EL MISMO ALCANCE */
    Symbol* simbolo_actual = alcance->simbolos;
    while (simbolo_actual != NULL) {
        if (strcmp(simbolo_actual->nombre, nombre) == 0) {
            char msg[256];
            sprintf(msg, "Redeclaracion del simbolo '%s'", nombre);
            reportarErrorSemantico(msg, -1); /* -1 = sin línea específica */
            return NULL;
        }
        simbolo_actual = simbolo_actual->siguiente;
    }

    /* 2. Crear el nuevo símbolo */
    Symbol* nuevo_simbolo = (Symbol*)malloc(sizeof(Symbol));
    if (!nuevo_simbolo) {
        perror("Error al crear nuevo Symbol");
        exit(1);
    }
    
    nuevo_simbolo->nombre = strdup(nombre);
    nuevo_simbolo->tipo = tipo;
    nuevo_simbolo->es_constante = es_const;
    nuevo_simbolo->es_funcion = es_func;
    nuevo_simbolo->num_params = num_params;
    
    /* 3. Añadirlo al inicio de la lista del alcance actual */
    nuevo_simbolo->siguiente = alcance->simbolos;
    alcance->simbolos = nuevo_simbolo;
    
    return nuevo_simbolo;
}

Symbol* buscarSimbolo(SymbolTable* tabla, const char* nombre) {
    Scope* alcance_temp = tabla->alcance_actual;
    
    /* Itera del alcance actual hacia afuera (hacia el global) */
    while (alcance_temp != NULL) {
        
        Symbol* simbolo = alcance_temp->simbolos;
        /* Busca en la lista de símbolos del alcance actual */
        while (simbolo != NULL) {
            if (strcmp(simbolo->nombre, nombre) == 0) {
                return simbolo; /* ¡Encontrado! */
            }
            simbolo = simbolo->siguiente;
        }
        
        /* No se encontró en este alcance, pasar al padre */
        alcance_temp = alcance_temp->padre;
    }
    
    return NULL; /* No se encontró en ningún alcance */
}