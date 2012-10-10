//generated code from C:\projects\cubeGL\happysolver\src\OpenGL\cubes.vtx.glsl

const char *code_cubes_vtx_glsl = " \
  varying vec3 normal; \n\
  varying vec3 lightVec; \n\
  varying vec3 eyeVec; \n\
   \n\
  void main() \n\
  {	 \n\
  	normal = gl_NormalMatrix * gl_Normal; \n\
   \n\
  	vec3 vVertex = vec3(gl_ModelViewMatrix * gl_Vertex); \n\
   \n\
  	// towards light in camera space \n\
  	lightVec = vec3(gl_LightSource[0].position.xyz - vVertex); \n\
  	// towards the eye in camera space \n\
  	eyeVec = -vVertex; \n\
   \n\
  	gl_FrontColor = gl_Color; \n\
  	gl_Position = ftransform();		 \n\
  } \n\
  ";
