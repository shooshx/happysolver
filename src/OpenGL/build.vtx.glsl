precision highp float;
varying vec4 color;

uniform mat4 trans;
attribute vec4 colorAtt;
attribute vec3 vtx;
attribute float tag;
uniform float fadeFactor;

void main()
{	
	color = colorAtt;
	if (tag == 1.0) 
		color.a *= fadeFactor;
	else if (tag == 2.0) 
		color = vec4(1.0, 1.0, 1.0, 1.0) - color*fadeFactor;
	else if (tag != 0.0)
		color = vec4(0.0, 1.0, 0.0, 1.0);

	gl_Position = trans * vec4(vtx, 1.0); 
}
