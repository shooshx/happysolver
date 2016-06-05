precision highp float;

varying float LightIntensity;
varying vec3  MCposition;
//varying vec3  vNormal;
//varying vec3 ECposition;

varying float sideLight;

uniform vec3 lightPos;

uniform mat4 trans;
uniform mat4 modelMat;
uniform mat3 normalMat;
uniform float isBlackSign;
attribute vec3 vtx;
attribute vec3 normal;

#define SIDE_DELTA 0.07

void main()
{
	vec3 ECposition = vec3(modelMat * vec4(vtx, 1.0));
	MCposition      = vtx;
	vec3 tnorm      = normalize(normalMat * normal); 
    //vNormal = tnorm;
	LightIntensity  = dot(normalize(lightPos - ECposition), tnorm); // gl_LightSource[0].position.xyz
	LightIntensity *= 1.2;

    sideLight = float(isBlackSign != 0.0 && vtx.y > SIDE_DELTA && vtx.y < 5.0-SIDE_DELTA && 
                vtx.z > SIDE_DELTA && vtx.z < 5.0-SIDE_DELTA && 
                vtx.x > isBlackSign * SIDE_DELTA && vtx.x < 1.0 + isBlackSign * SIDE_DELTA );  

    // x goes to either [-0.07 to 0.93] or to [0.07 to 1.07]

	gl_Position = trans * vec4(vtx, 1.0);
}
