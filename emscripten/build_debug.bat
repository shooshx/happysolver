rem %EMSCRIPTEN%\emcc --clear-cache
rem --tracing
rem -D_LIBCPP_DEBUG=0

%EMSCRIPTEN%\em++ --bind -g3 -O0 -std=c++11 -s ASSERTIONS=1 -s SAFE_HEAP=1 -s DEMANGLE_SUPPORT=1 -s ALLOW_MEMORY_GROWTH=0 -D_DEBUG -D_LIBCPP_DEBUG=0 --memory-init-file 0 js_main.cpp unity.cpp -I../src -Wno-switch -o js_main.html