SRC_DIR := src
OBJ_DIR := obj

EXE := chip8
SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

CFLAGS  := -Wall -Wextra -pedantic
LDFLAGS := $(shell sdl2-config --cflags --libs) -lm

.PHONY: all clean

all: $(EXE)

$(EXE): $(OBJ)
	$(CC) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR):
	mkdir -p $@

clean :
	rm -rv $(EXE) $(OBJ_DIR)

-include $(OBJ:.o=.d)
