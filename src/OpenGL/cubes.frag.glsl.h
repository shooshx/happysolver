//generated code from C:\projects\cubeGL\happysolver\src\OpenGL\cubes.frag.glsl

const char *code_cubes_frag_glsl = " \
  varying vec3 normal; \n\
  varying vec3 lightVec; \n\
  varying vec3 eyeVec; \n\
   \n\
  void main (void) \n\
  {					 \n\
  	vec3 N = normalize(normal); \n\
  	vec3 L = normalize(lightVec); \n\
  	 \n\
  	float lambertTerm = max(dot(N,L), 0.0); \n\
  	 \n\
   	// ** phong \n\
  	// vec3 E = normalize(eyeVec); \n\
  	// float prod = dot(reflect(L, N), E); \n\
  	// ** blinn \n\
  	float prod = dot(gl_LightSource[0].halfVector.xyz, N);  \n\
   \n\
  	float specularTerm = pow( max(prod, 0.0), gl_FrontMaterial.shininess ); \n\
   \n\
  	vec4 ambient = gl_LightSource[0].ambient; \n\
  	vec4 diffuse = gl_LightSource[0].diffuse * lambertTerm;	 \n\
  	vec4 specular = gl_LightSource[0].specular *  \n\
  	                gl_FrontMaterial.specular * specularTerm;	 \n\
   \n\
  	// mimicing GL_COLOR_MATERIAL \n\
  	gl_FragColor = (ambient + diffuse) * gl_Color + specular; \n\
   \n\
  } \n\
   \n\
  ";
