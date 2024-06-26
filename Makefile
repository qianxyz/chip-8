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

web:
	mkdir -p www
	emcc $(SRC) -o www/index.js \
		-s USE_SDL=2 \
		-s EXPORTED_FUNCTIONS=_main,_reload_rom \
		-s EXPORTED_RUNTIME_METHODS=ccall,cwrap \
		--preload-file rom/

-include $(OBJ:.o=.d)
