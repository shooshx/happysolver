varying vec3 normal;
varying vec3 lightVec;
varying vec3 eyeVec;

void main()
{	
	normal = gl_NormalMatrix * gl_Normal;

	vec3 vVertex = vec3(gl_ModelViewMatrix * gl_Vertex);

	// towards light in camera space
	lightVec = vec3(gl_LightSource[0].position.xyz - vVertex);
	// towards the eye in camera space
	eyeVec = -vVertex;

	gl_FrontColor = gl_Color;
	gl_Position = ftransform();		
}
