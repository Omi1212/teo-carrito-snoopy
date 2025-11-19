/* ================================================================== */
/* =                         ast.h                                  = */
/* =    (CORREGIDO - NOMBRES DE OPERADORES EN INGLES UNIFICADOS)    = */
/* ================================================================== */

#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Para copiar cadenas de yytext */
char* strdup(const char* s);

/* --- 1. Tipos de Nodos (NodeType) --- */
typedef enum {
    NODE_PROGRAMA,
    NODE_LISTA,
    NODE_NULO,
    NODE_DEFINICION_FUNCION,
    NODE_DECLARACION,
    NODE_SI,
    NODE_MIENTRAS,
    NODE_PARA,
    NODE_ASIGNACION,
    NODE_LLAMADA_FUNCION,
    NODE_BINARIA,
    NODE_UNARIA,
    NODE_IDENT,
    NODE_NUMERO,
    NODE_CARACTER,
    NODE_BOOLEANO,
    NODE_TIPO,
    NODE_PARAMETRO
} NodeType;

/* --- 2. Tipos de Datos (Unificado) --- */
typedef enum {
    TIPO_NULO,
    TIPO_ERROR,
    TIPO_NUMERO,
    TIPO_CARACTER,
    TIPO_BOOLEANO,
    TIPO_FUNCION
} TipoDato;


/* --- 3. Operadores (Unificados a Inglés) --- */

/* ================== ¡CAMBIO AQUÍ! ================== */
typedef enum {
    /* Binarios (Nombres en Inglés) */
    AST_OP_ADD, AST_OP_SUB, AST_OP_MUL, AST_OP_DIV, AST_OP_MOD,
    AST_OP_POW, /* Potencia */
    AST_OP_EQUAL, AST_OP_NOT_EQUAL, AST_OP_IS_NULL,
    AST_OP_GREATER, AST_OP_LESS, AST_OP_GREATER_EQUAL, AST_OP_LESS_EQUAL,
    AST_OP_AND, AST_OP_OR
} OpBinario;

/* ================== ¡CAMBIO AQUÍ! ================== */
typedef enum {
    /* Unarios (Nombres en Inglés) */
    AST_OP_NOT, /* Negación Lógica */
    AST_OP_NEGATE, /* Negación Aritmética */
    AST_OP_UNARY_PLUS
} OpUnario;

/* --- 4. Estructura base del Nodo --- */
struct Node;

/* --- 5. Estructuras de Datos Específicas por Nodo --- */
typedef struct {
    struct Node* nodo_actual;
    struct Node* nodo_siguiente;
} NodoLista;

typedef struct {
    struct Node* lista_funciones;
    struct Node* bloque_principal;
} NodoPrograma;

typedef struct {
    char* nombre;
    struct Node* parametros;
    struct Node* bloque;
} NodoDefinicionFuncion;

typedef struct {
    struct Node* tipo;
    char* nombre;
} NodoParametro;

typedef struct {
    int es_constante;
    struct Node* tipo;
    char* nombre;
    struct Node* inicializador;
} NodoDeclaracion;

typedef struct {
    struct Node* condicion;
    struct Node* bloque_si;
    struct Node* bloque_sino;
} NodoSi;

typedef struct {
    struct Node* condicion;
    struct Node* bloque;
} NodoMientras;

typedef struct {
    struct Node* inicializacion;
    struct Node* condicion;
    struct Node* incremento;
    struct Node* bloque;
} NodoPara;

typedef struct {
    char* nombre;
    struct Node* expresion;
} NodoAsignacion;

typedef struct {
    char* nombre;
    struct Node* argumentos;
} NodoLlamadaFuncion;

typedef struct {
    OpBinario op;
    struct Node* izquierda;
    struct Node* derecha;
} NodoBinaria;

typedef struct {
    OpUnario op;
    struct Node* operando;
} NodoUnaria;

typedef struct {
    TipoDato tipo;
} NodoTipo;

/* --- 6. Estructura del Nodo Base (Contenedor) --- */
typedef struct Node {
    NodeType type;
    int lineno;

    union {
        NodoPrograma          programa;
        NodoLista             lista;
        NodoDefinicionFuncion def_funcion;
        NodoParametro         parametro;
        NodoDeclaracion       declaracion;
        NodoSi                si;
        NodoMientras          mientras;
        NodoPara              para;
        NodoAsignacion        asignacion;
        NodoLlamadaFuncion    llamada_funcion;
        NodoBinaria           binaria;
        NodoUnaria            unaria;
        NodoTipo              tipo_dato;
        char* ident;
        char* numero;
        char* caracter;
        int   booleano;
    } data;
} Node;

/* --- 7. Prototipos de Funciones Constructoras --- */
Node* crearNodo(NodeType type);
Node* crearNodoPrograma(Node* funciones, Node* bloque);
Node* crearNodoLista(Node* actual, Node* siguiente);
Node* crearNodoNulo(void);
Node* crearNodoDefinicionFuncion(char* nombre, Node* parametros, Node* bloque);
Node* crearNodoParametro(Node* tipo, char* nombre);
Node* crearNodoDeclaracion(int es_const, Node* tipo, char* nombre, Node* init);
Node* crearNodoSi(Node* cond, Node* bloque_si, Node* bloque_sino);
Node* crearNodoMientras(Node* cond, Node* bloque);
Node* crearNodoPara(Node* init, Node* cond, Node* inc, Node* bloque);
Node* crearNodoAsignacion(char* nombre, Node* expr);
Node* crearNodoLlamadaFuncion(char* nombre, Node* args);
Node* crearNodoBinaria(OpBinario op, Node* izq, Node* der);
Node* crearNodoUnaria(OpUnario op, Node* opdo);
Node* crearNodoIdent(char* nombre);
Node* crearNodoNumero(char* valor);
Node* crearNodoCaracter(char* valor);
Node* crearNodoBooleano(int valor);
Node* crearNodoTipo(TipoDato tipo);

void imprimirAST(Node* nodo, int nivel);

#endif /* AST_H */