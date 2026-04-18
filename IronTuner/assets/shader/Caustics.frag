precision highp float;

out vec4 FragColor;

//---------------------------------------------------------
#ifdef GL_ES
const float CausicsLoops = 8.0;
#else
const float CausicsLoops = 8.0; //8.0
#endif


const float SparklePow  = 8.0;
const float speed = 0.2;
//---------------------------------------------------------


uniform float u_time;
uniform vec2 u_res;
uniform float u_rmsL;
uniform float u_rmsR;


vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - vec3(K.w));
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}


void main() {
    vec2 static_uv = gl_FragCoord.xy / u_res.xy;
    vec2 p = static_uv * 4.0 - 20.0;


    float energy = (u_rmsL + u_rmsR) * 0.5;
    float time = u_time * speed;

    vec2 i = vec2(p);
    float c = 1.0;
    float intensity = 0.005;

    for (float n = 0.0; n < CausicsLoops; n++) {
        float t =  time * (1.0 - (3.0 / (n + 1.0)));
        i = p + vec2(cos(t - i.x) + sin(t + i.y), sin(t - i.y) + cos(t + i.x));
        c += 0.1 / length(vec2(p.x / (sin(i.x + t) / intensity), p.y / (cos(i.y + t) / intensity)));
    }

    c /= CausicsLoops;
    c = 1.5 - pow(c, 0.5);

    float hue = fract(u_time * 0.05);
    float dynamicSaturation = 0.1 + energy ;
    vec3 color = hsv2rgb(vec3(hue, dynamicSaturation, 0.5)) ;

    float sparkle = pow(abs(c), SparklePow );
    vec3 causticColor = vec3(0.2, 0.6, 0.7) * sparkle;

    vec3 finalColor = color + causticColor + (causticColor /** highs*/ * 2.0);

    // Vignette
    float vignette = 1.0 - length(static_uv - 0.5) * 0.9;
    finalColor *= clamp(vignette, 0.0, 1.0);

    FragColor = vec4(finalColor, 1.0);
}
