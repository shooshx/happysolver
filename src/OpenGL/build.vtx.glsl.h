//generated code from C:\projects\cubeGL\happysolver\src\OpenGL\build.vtx.glsl

const char *code_build_vtx_glsl = " \
  varying vec4 color; \n\
   \n\
  uniform mat4 trans; \n\
  attribute vec4 colorA; \n\
  attribute vec3 vtx; \n\
   \n\
  void main() \n\
  {	 \n\
  	color = colorA; \n\
  	gl_Position = trans * vec4(vtx, 1.0);  \n\
  } \n\
  ";
