%{
/* ================================================================== */
/* =                 SECCIÓN 1: DEFINICIONES EN C                   = */
/* ================================================================== */
#include <stdio.h>
#include <string.h>

#include "parser.tab.h"

/* Prototipo de la función de error (definida en main.c) */
void yyerror(const char *s);

/* strdup es necesario para yylval (no es C99 estándar) */
#ifndef _WIN32
extern char* strdup(const char*);
#endif

%}

/* ================================================================== */
/* =        SECCIÓN 2: DEFINICIONES Y REGLAS DE FLEX            = */
/* ================================================================== */

DIGITO      [0-9]
LETRA       [a-zA-Z_]
IDENT       {LETRA}({LETRA}|{DIGITO})*
numero      {DIGITO}+(\.{DIGITO}+)?
caracter    \'([^'\\]|\\.)*\'

%%

[ \t\r]+        { /* Ignorar */ }
\n              { yylineno++; }
"//".* { /* Ignorar comentario */ }

"SI"            { return SI; }
"SINO"          { return SINO; }
"MIENTRAS"      { return MIENTRAS; }
"PARA"          { return PARA; }
"VARIABLE"      { return VARIABLE; }
"CONSTANTE"     { return CONSTANTE; }
"T_FUNCION"     { return T_FUNCION; }
"VERDADERO"     { return VERDADERO; }
"FALSO"         { return FALSO; }
"T_NUMERO"      { return T_NUMERO; }
"T_CARACTER"    { return T_CARACTER; }
"T_BOOLEANO"    { return T_BOOLEANO; }
"NULL"          { return T_NULL; }

"=="            { return IGUAL_IGUAL; }
"!="            { return DIFERENTE; }
">="            { return MAYOR_IGUAL; }
"<="            { return MENOR_IGUAL; }
"&&"            { return AND; }
"||"            { return OR; }
"^"             { return OP_POTENCIA; } 
"!"             { return OP_NEGACION; } 

"+"             { return '+'; }
"-"             { return '-'; }
"*"             { return '*'; }
"/"             { return '/'; }
"%"             { return '%'; }
"="             { return '='; }
">"             { return MAYOR; }
"<"             { return MENOR; }
"("             { return '('; }
")"             { return ')'; }
"{"             { return '{'; }
"}"             { return '}'; }
"["             { return '['; }
"]"             { return ']'; }
","             { return ','; }
";"             { return ';'; }
":"             { return ':'; }

{numero}        { yylval.sval = strdup(yytext); return numero; }
{caracter}      { yylval.sval = strdup(yytext); return caracter; }
{IDENT}         { yylval.sval = strdup(yytext); return IDENT; }

.               { 
                    char msg[100];
                    sprintf(msg, "Error Lexicografico: Caracter inesperado '%s'", yytext);
                    yyerror(msg);
                }

%%
/* ================================================================== */
/* =              SECCIÓN 3: CÓDIGO DE USUARIO EN C                 = */
/* ================================================================== */

int yywrap(void) {
    return 1;
}