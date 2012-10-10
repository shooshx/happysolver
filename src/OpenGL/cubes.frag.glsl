varying vec3 normal;
varying vec3 lightVec;
varying vec3 eyeVec;

void main (void)
{					
	vec3 N = normalize(normal);
	vec3 L = normalize(lightVec);
	
	float lambertTerm = max(dot(N,L), 0.0);
	
 	// ** phong
	// vec3 E = normalize(eyeVec);
	// float prod = dot(reflect(L, N), E);
	// ** blinn
	float prod = dot(gl_LightSource[0].halfVector.xyz, N); 

	float specularTerm = pow( max(prod, 0.0), gl_FrontMaterial.shininess );

	vec4 ambient = gl_LightSource[0].ambient;
	vec4 diffuse = gl_LightSource[0].diffuse * lambertTerm;	
	vec4 specular = gl_LightSource[0].specular * 
	                gl_FrontMaterial.specular * specularTerm;	

	// mimicing GL_COLOR_MATERIAL
	gl_FragColor = (ambient + diffuse) * gl_Color + specular;

}

