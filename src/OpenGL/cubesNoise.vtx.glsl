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
uniform bool isBlack;
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

    /*if (normal == vec3(0.0, 1.0, 0.0))
        sideLight = 1.0;
    else
        sideLight = 0.0;*/

    sideLight = length(normal.yz) * float(isBlack && vtx.y > SIDE_DELTA && vtx.y < 5.0-SIDE_DELTA && 
                                          vtx.z > SIDE_DELTA && vtx.z < 5.0-SIDE_DELTA && 
                                          vtx.x > SIDE_DELTA && vtx.x < 1.0-SIDE_DELTA );

	gl_Position = trans * vec4(vtx, 1.0);
}
