//generated code from C:\projects\cubeGL\happysolver\src\OpenGL\build.vtx.glsl

const char *code_build_vtx_glsl = " \
  precision highp float; \n\
  varying vec4 color; \n\
   \n\
  uniform mat4 trans; \n\
  attribute vec4 colorA; \n\
  attribute vec3 vtx; \n\
  attribute float tag; \n\
  uniform float fadeFactor; \n\
   \n\
  void main() \n\
  {	 \n\
  	color = colorA; \n\
  	if (tag == 1.0)  \n\
  		color *= fadeFactor; \n\
  	else if (tag == 2.0)  \n\
  		color = vec4(1.0, 1.0, 1.0, 1.0) - color*fadeFactor; \n\
  	else if (tag != 0.0) \n\
  		color = vec4(0.0, 1.0, 0.0, 1.0); \n\
   \n\
  	gl_Position = trans * vec4(vtx, 1.0);  \n\
  } \n\
  ";
