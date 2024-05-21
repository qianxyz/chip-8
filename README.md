# CHIP-8

A CHIP-8 emulator in C, built with SDL2 and ported to the web with Emscripten.

## Build the binary

Run `make` to build the `chip8` binary. Run `./chip8 path/to/rom` to load a
ROM, or run with the `-h` flag to see the help message.

## Build for the web

Run `make web`. The output files will be in the `www` directory.

## References

- [Compiling an SDL2 Game to WASM](https://dev.to/mattconn/compiling-an-sdl2-game-to-wasm-42fj)
- [SDL Wiki on Emscripten](https://wiki.libsdl.org/SDL2/README/emscripten)
- [Emscripten Tutorial](https://emscripten.org/docs/getting_started/Tutorial.html)
- [Calling compiled C functions from JavaScript](https://emscripten.org/docs/porting/connecting_cpp_and_javascript/Interacting-with-code.html#interacting-with-code-ccall-cwrap)
