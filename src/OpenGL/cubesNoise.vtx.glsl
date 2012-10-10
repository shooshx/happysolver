varying float LightIntensity;
varying vec3  MCposition;

void main()
{
	vec3 ECposition = vec3(gl_ModelViewMatrix * gl_Vertex);
	MCposition      = vec3(gl_Vertex);
	vec3 tnorm      = normalize(vec3(gl_NormalMatrix * gl_Normal));
	LightIntensity  = dot(normalize(gl_LightSource[0].position.xyz - ECposition), tnorm);
	LightIntensity *= 1.5;
	gl_Position     = ftransform();
}
