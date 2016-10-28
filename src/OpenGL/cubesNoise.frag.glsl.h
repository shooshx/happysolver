//generated code from C:\projects\cubeGL\happysolver\src\OpenGL\cubesNoise.frag.glsl

const char *code_cubesNoise_frag_glsl = " \
  precision highp float; \n\
   \n\
  varying float LightIntensity; \n\
  varying vec3 MCposition; \n\
  varying vec3 vNormal; \n\
   \n\
  uniform vec3 colorA; \n\
  uniform vec3 colorB; \n\
  uniform sampler2D noisef; \n\
  uniform int drawtype; \n\
   \n\
  uniform int flag; \n\
  uniform float fadeFactor; \n\
   \n\
  float offset= 4.72; \n\
  float offset2 = 0.5; \n\
  uniform vec3 texOffset; // z non-zero means we need to invert x \n\
  uniform vec2 texScale; // height is equal, already multiplied by 5 for MCposition [0,4] \n\
   \n\
  varying float sideLight; \n\
   \n\
  uniform mat2 texTrans; \n\
   \n\
  float mod(int x, float y){ \n\
      return float(x) - y * floor(float(x) / y); \n\
  } \n\
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
   \n\
     	//float LightIntensity  = dot(normalize(lightPos - ECposition), vNormal); // gl_LightSource[0].position.xyz \n\
  	//LightIntensity *= 1.2; \n\
   \n\
   \n\
      vec3 color = vec3(0.5, 0.5, 0.5); \n\
   \n\
      if (drawtype == 0) { // DRAW_COLOR \n\
          //color = colorA * LightIntensity; \n\
          color = colorA  * LightIntensity; \n\
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
          color = mix(colorA, colorB, sineval); \n\
          color *= LightIntensity; \n\
   \n\
      } \n\
      if (drawtype == 0x14)  \n\
      {  // DRAW_TEXTURE  \n\
         //  DRAW_TEXTURE_INDIVIDUAL_WHOLE, no need for smoothstep since its the same color but, meh \n\
   \n\
   \n\
          float tx = MCposition.x; \n\
          if (texOffset.z != 0.0) \n\
              tx = 1.0 - tx; \n\
   \n\
          vec2 tcoord = ((MCposition.zy - vec2(2.5, 2.5)) * texTrans) + vec2(2.5,2.5); \n\
          vec2 t = texOffset.xy + tcoord * texScale; // =40 MCPosition is [0,4] \n\
   \n\
          vec4 tc = texture2D(noisef, t); \n\
   \n\
          //color = (1.0 - tc.a) * colorB + tc.a * mix(tc.rgb, colorA, smoothstep(0.475, 0.525, tx));  \n\
          vec3 mtc = (1.0 - tc.a) * colorA + tc.a * tc.rgb; \n\
          color =  mix(mtc, colorB, smoothstep(0.475, 0.525, tx)); \n\
   \n\
          color *= LightIntensity; \n\
      } \n\
   \n\
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
          color = mix(colorA, colorB, sineval); \n\
          color *= LightIntensity; \n\
      } \n\
      if (drawtype == 0x100) { // flat \n\
          color = colorA; \n\
      } \n\
   \n\
      color += vec3(1,1,1) * sideLight; \n\
   \n\
      if (flag != 0) { \n\
          gl_FragColor = vec4(color.g + (0.8 - color.g)*fadeFactor, color.g - (0.3 * fadeFactor), color.b - (0.3 * fadeFactor), 1.0); \n\
          return; \n\
      } \n\
   \n\
      gl_FragColor = vec4(color, 1.0); \n\
  } \n\
  ";
