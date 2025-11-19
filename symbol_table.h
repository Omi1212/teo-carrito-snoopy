/* ================================================================== */
/* =                   symbol_table.h                               = */
/* =    Interfaz para la Tabla de Símbolos con Alcances Apilados    = */
/* ================================================================== */

#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "ast.h" /* Incluimos la definición maestra de TipoDato */


/* --- 1. Estructura de un Símbolo --- */
/* Representa una entrada individual en la tabla (variable, función, etc.) */
typedef struct Symbol {
    char* nombre;
    TipoDato tipo;
    int es_constante; /* 1 si es CONSTANTE, 0 si no [fuente: 10] */
    int es_funcion;
    int num_params;   /* Para chequeo de argumentos de funciones */

    struct Symbol* siguiente; /* Siguiente símbolo en el MISMO alcance */
} Symbol;

/* --- 2. Estructura de un Alcance (Scope) --- */
/* Un nivel de anidamiento (ej: un bloque {}, una función) */
typedef struct Scope {
    Symbol* simbolos;       /* Lista enlazada de símbolos en este alcance */
    struct Scope* padre;    /* Alcance que encierra a este (NULL si es global) */
} Scope;

/* --- 3. Estructura de la Tabla de Símbolos --- */
/* El manejador principal que apunta al alcance actual */
typedef struct {
    Scope* alcance_actual;
} SymbolTable;

/* --- 4. Prototipos de Funciones --- */

/**
 * @brief Crea e inicializa una nueva Tabla de Símbolos.
 * @return Puntero a la nueva SymbolTable.
 */
SymbolTable* crearTabla();

/**
 * @brief Libera toda la memoria asociada con la tabla y sus alcances.
 * @param tabla La tabla de símbolos a destruir.
 */
void destruirTabla(SymbolTable* tabla);

/**
 * @brief Entra en un nuevo alcance (ej: al inicio de un bloque o función).
 * @param tabla La tabla de símbolos.
 */
void entrarAlcance(SymbolTable* tabla);

/**
 * @brief Sale del alcance actual, liberando sus símbolos.
 * @param tabla La tabla de símbolos.
 */
void salirAlcance(SymbolTable* tabla);

/**
 * @brief Declara un nuevo símbolo en el alcance actual.
 * Reporta un error si el símbolo ya existe EN ESTE ALCANCE.
 * @param tabla La tabla de símbolos.
 * @param nombre El nombre del identificador.
 * @param tipo El TipoDato del símbolo.
 * @param es_const Si es una constante.
 * @param es_func Si es una función.
 * @param num_params Número de parámetros (si es función).
 * @return Puntero al nuevo símbolo, o NULL si falla.
 */
Symbol* declararSimbolo(SymbolTable* tabla, const char* nombre, TipoDato tipo, int es_const, int es_func, int num_params);

/**
 * @brief Busca un símbolo por nombre.
 * Comienza en el alcance actual y busca hacia afuera, hasta el global.
 * @param tabla La tabla de símbolos.
 * @param nombre El nombre del símbolo a buscar.
 * @return Puntero al Symbol encontrado, o NULL si no se encuentra.
 */
Symbol* buscarSimbolo(SymbolTable* tabla, const char* nombre);

#endif /* SYMBOL_TABLE_H */