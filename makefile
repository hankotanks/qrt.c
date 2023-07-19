CC = gcc
CFLAGS = -fopenmp -Wall -Wextra
LIBS = -lm

SRC_DIR := src
OBJ_DIR := obj
BIN_DIR := bin
DEP_DIR := include

EXE := $(BIN_DIR)/rt
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

.PHONY: all

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $^ $(LIBS) -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -I$(DEP_DIR) -c $< -o $@