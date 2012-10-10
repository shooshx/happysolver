//generated code from C:\projects\cubeGL\happysolver\src\OpenGL\cubesNoise.frag.glsl

const char *code_cubesNoise_frag_glsl = " \
  varying float LightIntensity; \n\
  varying vec3 MCposition; \n\
   \n\
  uniform vec3 colorA; \n\
  uniform vec3 colorB; \n\
  uniform sampler2D noisef; \n\
  uniform int drawtype; \n\
   \n\
  const float offset= 4.72; \n\
  const float offset2 = 0.5; \n\
   \n\
   \n\
  vec4 flat_texture3D(vec3 p) \n\
  { \n\
  	vec2 inimg = p.xy; \n\
  	inimg.y = max(inimg.y, 0.00390625); \n\
  	inimg.y = min(inimg.y, 0.99609375); \n\
   \n\
  	float fpz = p.z*128.0; \n\
  	int d1 = int(fpz); \n\
  	float ix1 = mod(d1, 8.0);  \n\
  	float iy1 = float(d1 / 8); \n\
  	vec2 oc1 = inimg + vec2(ix1, iy1); \n\
  	oc1 *= vec2(0.125, 0.25); \n\
   \n\
  	int d2 = d1 + 1; \n\
  	float ix2 = mod(d2, 8.0);  \n\
  	float iy2 = float(d2 / 8); \n\
  	vec2 oc2 = inimg + vec2(ix2, iy2); \n\
  	oc2 *= vec2(0.125, 0.25); \n\
   \n\
  	vec4 t1 = texture2D(noisef, oc1); \n\
  	vec4 t2 = texture2D(noisef, oc2); \n\
   \n\
  	vec4 t = mix(t1, t2, fract(fpz)); \n\
   \n\
  	return t; \n\
  } \n\
   \n\
  void main (void) \n\
  { \n\
  	if (drawtype == 0) { // DRAW_COLOR \n\
  		vec3 color = colorA * LightIntensity; \n\
  		gl_FragColor = vec4(color, 1.0); \n\
  		return; \n\
  	} \n\
  	if (drawtype == 2) { // blend blakc \n\
  		vec3 p = MCposition.yzx * 0.2; \n\
   \n\
  		vec4 noisevec = flat_texture3D(p); \n\
       \n\
  		float intensity = (noisevec[0] - offset2); \n\
   \n\
  		float sineval = sin(intensity * offset) * 2.0; \n\
  		sineval = clamp(sineval, 0.0, 1.0); \n\
   \n\
  		vec3 color   = mix(colorA, colorB, sineval); \n\
  		color       *= LightIntensity; \n\
   \n\
  		gl_FragColor = vec4(color, 1.0); \n\
  	} \n\
   \n\
  } \n\
  ";
