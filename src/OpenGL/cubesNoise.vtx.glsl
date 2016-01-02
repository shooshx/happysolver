precision highp float;

varying float LightIntensity;
varying vec3  MCposition;

uniform mat4 trans;
uniform mat4 modelMat;
uniform mat3 normalMat;
attribute vec3 vtx;
attribute vec3 normal;
uniform vec3 lightPos;

void main()
{
	vec3 ECposition = vec3(modelMat * vec4(vtx, 1.0));
	MCposition      = vtx;
	vec3 tnorm      = normalize(normalMat * normal); 
	LightIntensity  = dot(normalize(lightPos - ECposition), tnorm); // gl_LightSource[0].position.xyz
	LightIntensity *= 1.2;
	gl_Position     = trans * vec4(vtx, 1.0);
}
