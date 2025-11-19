# ==================================================================
# =                      Makefile                                  =
# =    Construye el compilador completo del robot.                 =
# =    Uso: make                                                   =
# ==================================================================

# --- Variables ---
TARGET = mi_compilador
CC = gcc
CFLAGS = -Wall -g # -Wall (todos los warnings), -g (debug)

# Herramientas
FLEX = flex
BISON = bison
BISON_FLAGS = -d # -d genera parser.tab.h

# --- Archivos ---

# Archivos .c que TÚ escribiste (o te di)
C_SOURCES = \
    main.c \
    ast.c \
    bytecode.c \
    compiler.c \
    semantic_analyzer.c \
    symbol_table.c \
    vm.c

# Archivos .c que se GENERARÁN
GEN_SOURCES = \
    lex.yy.c \
    parser.tab.c

# Todos los archivos objeto .o
OBJECTS = \
    $(C_SOURCES:.c=.o) \
    $(GEN_SOURCES:.c=.o)

# Librerías (lfl para flex, lm para math.h si lo usaras)
LIBS = -lm

# --- Reglas ---

# Regla por defecto: construir el ejecutable
all: $(TARGET)

# Regla para enlazar (linking) todo
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)
	@echo "--- Compilador [$(TARGET)] construido exitosamente. ---"

# Regla genérica para compilar .c -> .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# --- Reglas Especiales para Flex y Bison ---

# 1. Bison: parser.y -> parser.tab.c y parser.tab.h
parser.tab.c parser.tab.h: parser.y
	$(BISON) $(BISON_FLAGS) parser.y

# 2. Flex: lexer.l -> lex.yy.c
#    (Depende del parser.tab.h que generó Bison)
lex.yy.c: lexer.l parser.tab.h
	$(FLEX) -o lex.yy.c lexer.l

# Regla de limpieza
clean:
	rm -f $(TARGET) $(OBJECTS) lex.yy.c parser.tab.c parser.tab.h