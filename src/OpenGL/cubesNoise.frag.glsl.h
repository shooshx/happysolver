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
  uniform vec3 texOffset; // z non-zero means we need to invert x \n\
   \n\
  float mod(int x, float y){ \n\
      return float(x) - y * floor(float(x) / y); \n\
  } \n\
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
  	if (drawtype == 2) { // blend black \n\
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
  		return; \n\
  	} \n\
  	if (drawtype == 0x14) {  // DRAW_TEXTURE_INDIVIDUAL_HALF  \n\
  		const float T_HIGH = 0.525; \n\
  		const float T_LOW = 0.475; \n\
   \n\
  		float tx = MCposition.x; \n\
  		if (texOffset.z != 0.0) \n\
  			tx = 1.0 - tx; \n\
   \n\
  		if (tx > T_HIGH) { \n\
  			gl_FragColor = vec4(colorA * LightIntensity, 1.0); \n\
  			return; \n\
  		} \n\
   \n\
  		vec2 t = texOffset.xy + MCposition.yz / (8.0*5.0); \n\
   \n\
  		vec3 color = texture2D(noisef, t).rgb; \n\
  		if (tx > T_LOW && tx <= T_HIGH) { \n\
  			color = mix(color, colorA, smoothstep(T_LOW, T_HIGH, tx)); \n\
  		} \n\
   \n\
  		color *= LightIntensity; \n\
  		gl_FragColor = vec4(color, 1.0); \n\
  		return; \n\
  	} \n\
  	if (drawtype == 0x18) { //  DRAW_TEXTURE_INDIVIDUAL_WHOLE, no need for smoothstep since its the same color \n\
  		float tx = MCposition.x; \n\
  		if (texOffset.z != 0.0) \n\
  			tx = 1.0 - tx; \n\
   \n\
  		if (tx > 0.2) { \n\
  			gl_FragColor = vec4(colorA * LightIntensity, 1.0); \n\
  			return; \n\
  		} \n\
  		vec2 t = texOffset.xy + MCposition.yz / (8.0*5.0); \n\
  		vec3 color = texture2D(noisef, t).rgb * LightIntensity; \n\
  		gl_FragColor = vec4(color, 1.0); \n\
  		return; \n\
  	} \n\
  	if (drawtype == 4) { // DRAW_TEXTURE_MARBLE \n\
  		vec3 p = MCposition.yzx * 0.2; \n\
   \n\
  		vec4 noisevec = flat_texture3D(p); \n\
       \n\
  		float x = noisevec[0] - 0.54;  \n\
  		float y = noisevec[1] - 0.54; \n\
   \n\
  		float intensity = abs(x)*0.5 + abs(y)*0.5; \n\
   \n\
  		float sineval = sin(intensity) * 2.98; \n\
  		sineval = clamp(sineval, 0.0, 1.0); \n\
   \n\
  		vec3 color   = mix(colorA, colorB, sineval); \n\
  		color       *= LightIntensity; \n\
   \n\
  		gl_FragColor = vec4(color, 1.0); \n\
  		return; \n\
  	} \n\
  	if (drawtype == 0x100) { // flat \n\
  		gl_FragColor = vec4(colorA, 1.0); \n\
  	} \n\
   \n\
  } \n\
  ";
