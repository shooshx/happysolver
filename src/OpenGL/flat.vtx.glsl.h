//generated code from C:\projects\cubeGL\happysolver\src\OpenGL\flat.vtx.glsl

const char *code_flat_vtx_glsl = " \
  precision highp float; \n\
   \n\
  uniform mat4 trans; \n\
  uniform vec3 colorA; \n\
  attribute vec3 vtx; \n\
   \n\
  varying vec3 color; \n\
   \n\
  // used for piece selection, not build selection \n\
  void main() \n\
  {	 \n\
  	color = colorA; \n\
  	//gl_Position = trans * vec4(vtx, 1.0); \n\
      gl_Position = trans * vec4(vtx, 1.0); \n\
  } \n\
  ";
