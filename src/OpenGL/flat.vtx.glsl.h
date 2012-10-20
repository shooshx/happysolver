//generated code from C:\projects\cubeGL\happysolver\src\OpenGL\flat.vtx.glsl

const char *code_flat_vtx_glsl = " \
  uniform mat4 trans; \n\
  uniform vec3 colorA; \n\
  varying vec3 color; \n\
  attribute vec3 vtx; \n\
   \n\
  void main() \n\
  {	 \n\
  	color = colorA; \n\
  	gl_Position = trans * vec4(vtx, 1.0); \n\
  } \n\
  ";
