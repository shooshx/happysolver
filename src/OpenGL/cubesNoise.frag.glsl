precision highp float;

varying float LightIntensity;
varying vec3 MCposition;

uniform vec3 colorA;
uniform vec3 colorB;
uniform sampler2D noisef;
uniform int drawtype;

uniform int flag;
uniform float fadeFactor;

float offset= 4.72;
float offset2 = 0.5;
uniform vec3 texOffset; // z non-zero means we need to invert x
uniform vec2 texScale; // height is equal, already multiplied by 5 for MCposition [0,4]

float mod(int x, float y){
    return float(x) - y * floor(float(x) / y);
}

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
   // gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
   // return;

    vec3 color = vec3(0.5, 0.5, 0.5);

    if (drawtype == 0) { // DRAW_COLOR
        color = colorA * LightIntensity;
    }
    if (drawtype == 2) { // blend black
        vec3 p = MCposition.yzx * 0.2;

        vec4 noisevec = flat_texture3D(p);
    
        float intensity = (noisevec[0] - offset2);

        float sineval = sin(intensity * offset) * 2.0;
        sineval = clamp(sineval, 0.0, 1.0);

        color = mix(colorA, colorB, sineval);
        color *= LightIntensity;

    }
    if (drawtype == 0x14 || drawtype == 0x18) 
    {  // DRAW_TEXTURE_INDIVIDUAL_HALF 
       //  DRAW_TEXTURE_INDIVIDUAL_WHOLE, no need for smoothstep since its the same color but, meh


        float tx = MCposition.x;
        if (texOffset.z != 0.0)
            tx = 1.0 - tx;

        vec2 t = texOffset.xy + MCposition.yz * texScale; // =40 MCPosition is [0,4]

        vec4 tc = texture2D(noisef, t);

        color = (1.0 - tc.a) * colorA + tc.a * mix(tc.rgb, colorA, smoothstep(0.475, 0.525, tx));

        color *= LightIntensity;
    }

    if (drawtype == 4) { // DRAW_TEXTURE_MARBLE
        vec3 p = MCposition.yzx * 0.2;

        vec4 noisevec = flat_texture3D(p);
    
        float x = noisevec[0] - 0.54; 
        float y = noisevec[1] - 0.54;

        float intensity = abs(x)*0.5 + abs(y)*0.5;

        float sineval = sin(intensity) * 2.98;
        sineval = clamp(sineval, 0.0, 1.0);

        color = mix(colorA, colorB, sineval);
        color *= LightIntensity;
    }
    if (drawtype == 0x100) { // flat
        color = colorA;
    }

    if (flag != 0) {
        gl_FragColor = vec4(color.g + (0.8 - color.g)*fadeFactor, color.g - (0.3 * fadeFactor), color.b - (0.3 * fadeFactor), 1.0);
        return;
    }

    gl_FragColor = vec4(color, 1.0);
}