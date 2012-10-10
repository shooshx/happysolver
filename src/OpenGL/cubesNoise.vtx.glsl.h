//generated code from C:\projects\cubeGL\happysolver\src\OpenGL\cubesNoise.vtx.glsl

const char *code_cubesNoise_vtx_glsl = " \
  varying float LightIntensity; \n\
  varying vec3  MCposition; \n\
   \n\
  void main() \n\
  { \n\
  	vec3 ECposition = vec3(gl_ModelViewMatrix * gl_Vertex); \n\
  	MCposition      = vec3(gl_Vertex); \n\
  	vec3 tnorm      = normalize(vec3(gl_NormalMatrix * gl_Normal)); \n\
  	LightIntensity  = dot(normalize(gl_LightSource[0].position.xyz - ECposition), tnorm); \n\
  	LightIntensity *= 1.5; \n\
  	gl_Position     = ftransform(); \n\
  } \n\
  ";
