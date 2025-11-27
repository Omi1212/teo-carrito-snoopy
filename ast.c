/* ================================================================== */
/* =                         ast.c                                  = */
/* =      Implementación de las funciones constructoras del AST       = */
/* ================================================================== */

#include "ast.h"

extern int yylineno;

Node* crearNodo(NodeType type) {
    Node* nodo = (Node*)malloc(sizeof(Node));
    if (nodo == NULL) {
        fprintf(stderr, "Error: Fallo de asignacion de memoria (malloc)\n");
        exit(1);
    }
    nodo->type = type;
    nodo->lineno = yylineno;
    return nodo;
}

Node* crearNodoPrograma(Node* funciones, Node* bloque) {
    Node* nodo = crearNodo(NODE_PROGRAMA);
    nodo->data.programa.lista_funciones = funciones;
    nodo->data.programa.bloque_principal = bloque;
    return nodo;
}

Node* crearNodoLista(Node* actual, Node* siguiente) {
    Node* nodo = crearNodo(NODE_LISTA);
    nodo->data.lista.nodo_actual = actual;
    nodo->data.lista.nodo_siguiente = siguiente;
    return nodo;
}

Node* crearNodoNulo(void) {
    Node* nodo = crearNodo(NODE_NULO);
    return nodo;
}

Node* crearNodoDefinicionFuncion(char* nombre, Node* parametros, Node* bloque) {
    Node* nodo = crearNodo(NODE_DEFINICION_FUNCION);
    nodo->data.def_funcion.nombre = nombre;
    nodo->data.def_funcion.parametros = parametros;
    nodo->data.def_funcion.bloque = bloque;
    return nodo;
}

Node* crearNodoParametro(Node* tipo, char* nombre) {
    Node* nodo = crearNodo(NODE_PARAMETRO);
    nodo->data.parametro.tipo = tipo;
    nodo->data.parametro.nombre = nombre;
    return nodo;
}

Node* crearNodoDeclaracion(int es_const, Node* tipo, char* nombre, Node* init) {
    Node* nodo = crearNodo(NODE_DECLARACION);
    nodo->data.declaracion.es_constante = es_const;
    nodo->data.declaracion.tipo = tipo;
    nodo->data.declaracion.nombre = nombre;
    nodo->data.declaracion.inicializador = init;
    return nodo;
}

Node* crearNodoSi(Node* cond, Node* bloque_si, Node* bloque_sino) {
    Node* nodo = crearNodo(NODE_SI);
    nodo->data.si.condicion = cond;
    nodo->data.si.bloque_si = bloque_si;
    nodo->data.si.bloque_sino = bloque_sino;
    return nodo;
}

Node* crearNodoMientras(Node* cond, Node* bloque) {
    Node* nodo = crearNodo(NODE_MIENTRAS);
    nodo->data.mientras.condicion = cond;
    nodo->data.mientras.bloque = bloque;
    return nodo;
}

Node* crearNodoPara(Node* init, Node* cond, Node* inc, Node* bloque) {
    Node* nodo = crearNodo(NODE_PARA);
    nodo->data.para.inicializacion = init;
    nodo->data.para.condicion = cond;
    nodo->data.para.incremento = inc;
    nodo->data.para.bloque = bloque;
    return nodo;
}

Node* crearNodoAsignacion(char* nombre, Node* expr) {
    Node* nodo = crearNodo(NODE_ASIGNACION);
    nodo->data.asignacion.nombre = nombre;
    nodo->data.asignacion.expresion = expr;
    return nodo;
}

Node* crearNodoLlamadaFuncion(char* nombre, Node* args) {
    Node* nodo = crearNodo(NODE_LLAMADA_FUNCION);
    nodo->data.llamada_funcion.nombre = nombre;
    nodo->data.llamada_funcion.argumentos = args;
    return nodo;
}

Node* crearNodoBinaria(OpBinario op, Node* izq, Node* der) {
    Node* nodo = crearNodo(NODE_BINARIA);
    nodo->data.binaria.op = op;
    nodo->data.binaria.izquierda = izq;
    nodo->data.binaria.derecha = der;
    return nodo;
}

Node* crearNodoUnaria(OpUnario op, Node* opdo) {
    Node* nodo = crearNodo(NODE_UNARIA);
    nodo->data.unaria.op = op;
    nodo->data.unaria.operando = opdo;
    return nodo;
}

Node* crearNodoIdent(char* nombre) {
    Node* nodo = crearNodo(NODE_IDENT);
    nodo->data.ident = nombre;
    return nodo;
}

Node* crearNodoNumero(char* valor) {
    Node* nodo = crearNodo(NODE_NUMERO);
    /* CAMBIO: numero -> val_num */
    nodo->data.val_num = valor;
    return nodo;
}

Node* crearNodoCaracter(char* valor) {
    Node* nodo = crearNodo(NODE_CARACTER);
    /* CAMBIO: caracter -> val_char */
    nodo->data.val_char = valor;
    return nodo;
}

Node* crearNodoBooleano(int valor) {
    Node* nodo = crearNodo(NODE_BOOLEANO);
    nodo->data.booleano = valor;
    return nodo;
}

Node* crearNodoTipo(TipoDato tipo) {
    Node* nodo = crearNodo(NODE_TIPO);
    nodo->data.tipo_dato.tipo = tipo;
    return nodo;
}

#ifndef strdup
char* strdup(const char* s) {
    if (s == NULL) return NULL;
    size_t len = strlen(s) + 1;
    char* nuevo = (char*)malloc(len);
    if (nuevo == NULL) {
        fprintf(stderr, "Error: Fallo de malloc en strdup\n");
        exit(1);
    }
    memcpy(nuevo, s, len);
    return nuevo;
}
#endif