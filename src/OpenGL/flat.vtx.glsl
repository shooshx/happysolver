uniform mat4 trans;
uniform vec3 colorA;
varying vec3 color;
attribute vec3 vtx;

void main()
{	
	color = colorA;
	gl_Position = trans * vec4(vtx, 1.0);
}
