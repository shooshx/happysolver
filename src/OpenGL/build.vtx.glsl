varying vec4 color;

uniform mat4 trans;
attribute vec4 colorA;
attribute vec3 vtx;

void main()
{	
	color = colorA;
	gl_Position = trans * vec4(vtx, 1.0); 
}
