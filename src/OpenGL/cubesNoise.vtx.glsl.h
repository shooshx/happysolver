//generated code from C:\projects\cubeGL\happysolver\src\OpenGL\cubesNoise.vtx.glsl

const char *code_cubesNoise_vtx_glsl = " \
  varying float LightIntensity; \n\
  varying vec3  MCposition; \n\
   \n\
  uniform mat4 trans; \n\
  uniform mat4 modelMat; \n\
  uniform mat3 normalMat; \n\
  attribute vec3 vtx; \n\
  attribute vec3 normal; \n\
   \n\
  void main() \n\
  { \n\
  	vec3 ECposition = vec3(modelMat * vec4(vtx, 1.0)); \n\
  	MCposition      = vtx; \n\
  	vec3 tnorm      = normalize(normalMat * normal); // gl_NormalMatrix \n\
  	LightIntensity  = dot(normalize(gl_LightSource[0].position.xyz - ECposition), tnorm); \n\
  	LightIntensity *= 1.5; \n\
  	gl_Position     = trans * vec4(vtx, 1.0); \n\
  } \n\
  ";
