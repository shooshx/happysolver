varying float LightIntensity;
varying vec3 MCposition;

uniform vec3 colorA;
uniform vec3 colorB;
uniform sampler2D noisef;
uniform int drawtype;

const float offset= 4.72;
const float offset2 = 0.5;


vec4 flat_texture3D(vec3 p)
{
	vec2 inimg = p.xy;
	inimg.y = max(inimg.y, 0.00390625);
	inimg.y = min(inimg.y, 0.99609375);

	float fpz = p.z*128.0;
	int d1 = int(fpz);
	float ix1 = mod(d1, 8.0); 
	float iy1 = float(d1 / 8);
	vec2 oc1 = inimg + vec2(ix1, iy1);
	oc1 *= vec2(0.125, 0.25);

	int d2 = d1 + 1;
	float ix2 = mod(d2, 8.0); 
	float iy2 = float(d2 / 8);
	vec2 oc2 = inimg + vec2(ix2, iy2);
	oc2 *= vec2(0.125, 0.25);

	vec4 t1 = texture2D(noisef, oc1);
	vec4 t2 = texture2D(noisef, oc2);

	vec4 t = mix(t1, t2, fract(fpz));

	return t;
}

void main (void)
{
	if (drawtype == 0) { // DRAW_COLOR
		vec3 color = colorA * LightIntensity;
		gl_FragColor = vec4(color, 1.0);
		return;
	}
	if (drawtype == 2) { // blend black
		vec3 p = MCposition.yzx * 0.2;

		vec4 noisevec = flat_texture3D(p);
    
		float intensity = (noisevec[0] - offset2);

		float sineval = sin(intensity * offset) * 2.0;
		sineval = clamp(sineval, 0.0, 1.0);

		vec3 color   = mix(colorA, colorB, sineval);
		color       *= LightIntensity;

		gl_FragColor = vec4(color, 1.0);
		return;
	}
	if (drawtype == 0x100) { // flat
		gl_FragColor = vec4(colorA, 1.0);
	}

}