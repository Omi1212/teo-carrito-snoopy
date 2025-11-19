/* ================================================================== */
/* =                         parser.y                               = */
/* =    (CORREGIDO - Arreglada la lógica del AST para AND/OR)       = */
/* ================================================================== */

%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h" 

extern int yylex(void);
extern char* yytext;
extern int yylineno;
void yyerror(const char *s);
Node *ast_root = NULL;
%}

%union {
    struct Node* nodo;
    char* sval;
}

%token <sval> IDENT
%token <sval> numero
%token <sval> caracter
%token SI SINO MIENTRAS PARA
%token VARIABLE CONSTANTE T_FUNCION
%token VERDADERO FALSO T_NUMERO T_CARACTER T_BOOLEANO
%token T_NULL
%token OP_POTENCIA
%token IGUAL_IGUAL DIFERENTE
%token MAYOR MENOR MAYOR_IGUAL MENOR_IGUAL
%token AND OR OP_NEGACION

%type <nodo> Programa lista_funciones funcion_definicion
%type <nodo> Bloque contenido_bloque contenido_prima
%type <nodo> sentencia
%type <nodo> declaracion inicializacion lista_expresiones lista_exp_prima
%type <nodo> si parte_sino
%type <nodo> mientras
%type <nodo> para asignacion_simple
%type <nodo> lista_params_opcional lista_params params_prima parametro
%type <nodo> instruccion_con_ident inst_despues_de_ident
%type <nodo> args_opcionales lista_argumentos lista_argumentos_prima
%type <nodo> expresion expresion_booleana expresion_bool_prima
%type <nodo> logica_AND logica_AND_prima
%type <nodo> logica_NOT
%type <nodo> igualdad igualdad_prima op_igualdad
%type <nodo> comparacion comparacion_prima op_comparacion
%type <nodo> expresion_aritmetica expresion_arit_prima
%type <nodo> expresion_aritmetica_termino expresion_arit_termino_prima op_termino
%type <nodo> unario
%type <nodo> potencia potencia_prima
%type <nodo> factor factor_despues_de_ident
%type <nodo> literal booleano
%type <nodo> nombre_tipo

%start Programa

%%
Programa: lista_funciones Bloque
    { 
        $$ = crearNodoPrograma($1, $2); 
        ast_root = $$; 
    }
;

lista_funciones: funcion_definicion lista_funciones
    { $$ = crearNodoLista($1, $2); }
    | { $$ = crearNodoNulo(); }
;

Bloque: '{' contenido_bloque '}'
    { $$ = $2; }
;

contenido_bloque: sentencia contenido_prima
    { $$ = crearNodoLista($1, $2); }
    | { $$ = crearNodoNulo(); }
;

contenido_prima: ';' sentencia contenido_prima
    { $$ = crearNodoLista($2, $3); }
    | { $$ = crearNodoNulo(); }
;

sentencia: declaracion           { $$ = $1; }
    | si                       { $$ = $1; }
    | mientras                 { $$ = $1; }
    | para                     { $$ = $1; }
    | Bloque                   { $$ = $1; }
    | instruccion_con_ident    { $$ = $1; }
;

declaracion: VARIABLE IDENT ':' nombre_tipo inicializacion
    { $$ = crearNodoDeclaracion(0, $4, strdup($2), $5); }
    | CONSTANTE IDENT ':' nombre_tipo inicializacion
    { $$ = crearNodoDeclaracion(1, $4, strdup($2), $5); }
;

inicializacion: '=' expresion
    { $$ = $2; }
    | '[' ']' '=' '[' lista_expresiones ']'
    { $$ = $5; }
    | { $$ = crearNodoNulo(); }
;

lista_expresiones: expresion lista_exp_prima
    { $$ = crearNodoLista($1, $2); }
;

lista_exp_prima: ',' expresion lista_exp_prima
    { $$ = crearNodoLista($2, $3); }
    | { $$ = crearNodoNulo(); }
;

si: SI '(' expresion_booleana ')' Bloque parte_sino
    { $$ = crearNodoSi($3, $5, $6); }
;

parte_sino: SINO Bloque
    { $$ = $2; }
    | { $$ = crearNodoNulo(); }
;

mientras: MIENTRAS '(' expresion_booleana ')' Bloque
    { $$ = crearNodoMientras($3, $5); }
;

para: PARA '(' asignacion_simple ';' expresion_booleana ';' asignacion_simple ')' Bloque
    { $$ = crearNodoPara($3, $5, $7, $9); }
;

asignacion_simple: IDENT '=' expresion 
    { $$ = crearNodoAsignacion(strdup($1), $3); }
;

funcion_definicion: T_FUNCION IDENT '(' lista_params_opcional ')' Bloque
    { $$ = crearNodoDefinicionFuncion(strdup($2), $4, $6); }
;

lista_params_opcional: lista_params
    { $$ = $1; }
    | { $$ = crearNodoNulo(); }
;
        
lista_params: parametro params_prima
    { $$ = crearNodoLista($1, $2); }
;

params_prima: ',' parametro params_prima
    { $$ = crearNodoLista($2, $3); }
    | { $$ = crearNodoNulo(); }
;

parametro: nombre_tipo IDENT
    { $$ = crearNodoParametro($1, strdup($2)); }
;

instruccion_con_ident: IDENT inst_despues_de_ident
    {
        if ($2->type == NODE_ASIGNACION) {
            $2->data.asignacion.nombre = strdup($1);
        } else if ($2->type == NODE_LLAMADA_FUNCION) {
            $2->data.llamada_funcion.nombre = strdup($1);
        }
        $$ = $2;
    }
;

inst_despues_de_ident: '=' expresion            
    { $$ = crearNodoAsignacion(NULL, $2); }
    | '(' args_opcionales ')'
    { $$ = crearNodoLlamadaFuncion(NULL, $2); }
;

args_opcionales: lista_argumentos
    { $$ = $1; }
    | { $$ = crearNodoNulo(); }
;

lista_argumentos: expresion lista_argumentos_prima
    { $$ = crearNodoLista($1, $2); }
;

lista_argumentos_prima: ',' expresion lista_argumentos_prima
    { $$ = crearNodoLista($2, $3); }
    | { $$ = crearNodoNulo(); }
;

expresion: expresion_booleana { $$ = $1; }
;

expresion_booleana: logica_AND expresion_bool_prima
    { 
        if ($2->type == NODE_NULO) { 
            $$ = $1;
        } else { 
            $$ = crearNodoBinaria(AST_OP_OR, $1, $2);
        }
    }
;

expresion_bool_prima: OR logica_AND expresion_bool_prima
    { 
        if ($3->type == NODE_NULO) {
            $$ = $2;
        } else {
            $$ = crearNodoBinaria(AST_OP_OR, $2, $3); 
        }
    }
    | { $$ = crearNodoNulo(); }
;

logica_AND: logica_NOT logica_AND_prima
    { 
        if ($2->type == NODE_NULO) { 
            $$ = $1;
        } else { 
            $$ = crearNodoBinaria(AST_OP_AND, $1, $2);
        }
    }
;

logica_AND_prima: AND logica_NOT logica_AND_prima
    { 
        if ($3->type == NODE_NULO) {
            $$ = $2;
        } else {
            $$ = crearNodoBinaria(AST_OP_AND, $2, $3); 
        }
    }
    | { $$ = crearNodoNulo(); }
;

logica_NOT: OP_NEGACION logica_NOT
    { $$ = crearNodoUnaria(AST_OP_NOT, $2); }
    | igualdad
    { $$ = $1; }
;

igualdad: comparacion igualdad_prima
    {
        if ($2->type == NODE_NULO) { $$ = $1; }
        else {
            $2->data.binaria.izquierda = $1;
            $$ = $2;
        }
    }
;

igualdad_prima: op_igualdad comparacion igualdad_prima
    {
        if ($3->type == NODE_NULO) {
            $1->data.binaria.derecha = $2;
        } else {
            $3->data.binaria.izquierda = $2;
            $1->data.binaria.derecha = $3;
        }
        $$ = $1;
    }
    | { $$ = crearNodoNulo(); }
;

op_igualdad: IGUAL_IGUAL { $$ = crearNodoBinaria(AST_OP_EQUAL, NULL, NULL); }
    | DIFERENTE          { $$ = crearNodoBinaria(AST_OP_NOT_EQUAL, NULL, NULL); }
    | T_NULL             { $$ = crearNodoBinaria(AST_OP_IS_NULL, NULL, NULL); }
;

comparacion: expresion_aritmetica comparacion_prima
    { 
        if ($2->type == NODE_NULO) { $$ = $1; }
        else { $2->data.binaria.izquierda = $1; $$ = $2; }
    }
;

comparacion_prima: op_comparacion expresion_aritmetica comparacion_prima
    {
        if ($3->type == NODE_NULO) {
            $1->data.binaria.derecha = $2;
        } else {
            $3->data.binaria.izquierda = $2;
            $1->data.binaria.derecha = $3;
        }
        $$ = $1;
    }
    | { $$ = crearNodoNulo(); }
;

op_comparacion: MAYOR     { $$ = crearNodoBinaria(AST_OP_GREATER, NULL, NULL); }
    | MENOR                { $$ = crearNodoBinaria(AST_OP_LESS, NULL, NULL); }
    | MAYOR_IGUAL          { $$ = crearNodoBinaria(AST_OP_GREATER_EQUAL, NULL, NULL); }
    | MENOR_IGUAL          { $$ = crearNodoBinaria(AST_OP_LESS_EQUAL, NULL, NULL); }
;

expresion_aritmetica: expresion_aritmetica_termino expresion_arit_prima
    { 
        if ($2->type == NODE_NULO) { $$ = $1; }
        else { $2->data.binaria.izquierda = $1; $$ = $2; }
    }
;

expresion_arit_prima: '+' expresion_aritmetica_termino expresion_arit_prima
    { 
        if ($3->type == NODE_NULO) { $$ = crearNodoBinaria(AST_OP_ADD, NULL, $2); }
        else { $3->data.binaria.izquierda = $2; $$ = crearNodoBinaria(AST_OP_ADD, NULL, $3); }
    }
    | '-' expresion_aritmetica_termino expresion_arit_prima
    { 
        if ($3->type == NODE_NULO) { $$ = crearNodoBinaria(AST_OP_SUB, NULL, $2); }
        else { $3->data.binaria.izquierda = $2; $$ = crearNodoBinaria(AST_OP_SUB, NULL, $3); }
    }
    | { $$ = crearNodoNulo(); }
;

expresion_aritmetica_termino: unario expresion_arit_termino_prima
    { 
        if ($2->type == NODE_NULO) { $$ = $1; }
        else { $2->data.binaria.izquierda = $1; $$ = $2; }
    }
;

expresion_arit_termino_prima: op_termino unario expresion_arit_termino_prima
    {
        if ($3->type == NODE_NULO) {
            $1->data.binaria.derecha = $2;
        } else {
            $3->data.binaria.izquierda = $2;
            $1->data.binaria.derecha = $3;
        }
        $$ = $1;
    }
    | { $$ = crearNodoNulo(); }
;

op_termino: '*' { $$ = crearNodoBinaria(AST_OP_MUL, NULL, NULL); }
    | '/'        { $$ = crearNodoBinaria(AST_OP_DIV, NULL, NULL); }
    | '%'        { $$ = crearNodoBinaria(AST_OP_MOD, NULL, NULL); }
;

unario: '+' unario
    { $$ = crearNodoUnaria(AST_OP_UNARY_PLUS, $2); }
    | '-' unario
    { $$ = crearNodoUnaria(AST_OP_NEGATE, $2); }
    | potencia
    { $$ = $1; }
;

potencia: factor potencia_prima
    { 
        if ($2->type == NODE_NULO) { $$ = $1; }
        else { $2->data.binaria.izquierda = $1; $$ = $2; }
    }
;

potencia_prima: OP_POTENCIA potencia
    { $$ = crearNodoBinaria(AST_OP_POW, NULL, $2); }
    | { $$ = crearNodoNulo(); }
;

factor: IDENT factor_despues_de_ident
    { 
        if ($2->type == NODE_NULO) {
            $$ = crearNodoIdent(strdup($1));
        } else {
            $2->data.llamada_funcion.nombre = strdup($1);
            $$ = $2;
        }
    }
    | literal
    { $$ = $1; }
    | '(' expresion ')'
    { $$ = $2; }
;

factor_despues_de_ident: '(' args_opcionales ')'
    { $$ = crearNodoLlamadaFuncion(NULL, $2); }
    | { $$ = crearNodoNulo(); }
;

literal: numero
    { $$ = crearNodoNumero(strdup($1)); }
    | caracter
    { $$ = crearNodoCaracter(strdup($1)); }
    | booleano
    { $$ = $1; }
;

booleano: VERDADERO
    { $$ = crearNodoBooleano(1); }
    | FALSO
    { $$ = crearNodoBooleano(0); }
;

nombre_tipo: T_NUMERO
    { $$ = crearNodoTipo(TIPO_NUMERO); }
    | T_CARACTER
    { $$ = crearNodoTipo(TIPO_CARACTER); }
    | T_BOOLEANO
    { $$ = crearNodoTipo(TIPO_BOOLEANO); }
    | T_FUNCION
    { $$ = crearNodoTipo(TIPO_FUNCION); }
;

%%