rem --profiling -s DEMANGLE_SUPPORT=1
%EMSCRIPTEN%\em++ --bind -O3 -s TOTAL_MEMORY=20000000 -std=c++11 --memory-init-file 0 js_main.cpp unity.cpp -I../src -Wno-switch -o js_main.html 