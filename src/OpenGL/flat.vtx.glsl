precision highp float;

uniform mat4 trans;
uniform vec3 colorA;
attribute vec3 vtx;

varying vec3 color;

// used for piece selection, not build selection
void main()
{	
	color = colorA;
	//gl_Position = trans * vec4(vtx, 1.0);
    gl_Position = trans * vec4(vtx, 1.0);
}
