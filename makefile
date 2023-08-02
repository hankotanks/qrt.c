CC = gcc
CFLAGS = -fopenmp -Wall -Wextra
LIBS = -lm -lcsfml-graphics -lcsfml-system -lcsfml-window

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
DEP_DIR := include

CSFML_DEP_DIR := SFML/include
CSFML_LIB_DIR := SFML/lib
CSFML := -L $(CSFML_LIB_DIR) -I $(CSFML_DEP_DIR)

EXE := $(BIN_DIR)/rt
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

.PHONY: all

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $(CFLAGS) $^ $(LIBS) $(CSFML) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -I $(DEP_DIR) $(CSFML) -c $< -o $@

