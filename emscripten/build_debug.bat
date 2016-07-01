rem %EMSCRIPTEN%\emcc --clear-cache
rem --tracing
rem -D_LIBCPP_DEBUG=0

%EMSCRIPTEN%\em++ -g3 -O0 -std=c++11 -s ASSERTIONS=1 -s SAFE_HEAP=1 -s DEMANGLE_SUPPORT=1 -s ALLOW_MEMORY_GROWTH=0 -D_DEBUG -D_LIBCPP_DEBUG=0 --memory-init-file 0 js_main.cpp unity.cpp -I../src -Wno-switch -o js_main.html -s EXPORTED_FUNCTIONS="['_cpp_start',  '_cpp_draw', '_initCubeEngine', '_resizeGl', '_mouseDown', '_mouseUp', '_mouseMove', '_mouseDblClick', '_mouseWheel', '_cpp_slvrun', '_getTms', '_solveGo', '_setGrpCount', '_setEditAction', '_runningRestart', '_conf', '_serializeCurrent', '_deserializeAndLoad', '_readCubeFromEditor', '_newRestart', '_stackState', '_textureParamCube', '_getCubeTextureHandle', '_readCubeTexCoord', '_loadSlvSimple', '_freeMeshAllocator', '_readCubeToEditor', '_textureParamToEditor', '_getToothPossibilities', '_readCubeFromSig', '_bucketAddFam', '_setPicCount', '_postReadAllPics']"