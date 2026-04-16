precision mediump float;

uniform float u_time;
uniform vec2 u_res;
uniform float u_rmsL;
uniform float u_rmsR;
uniform float u_freqs[32];

void main() {
    vec2 uv = gl_FragCoord.xy / u_res.xy;
    vec2 p = uv * 3.0;

    float energy = (u_rmsL + u_rmsR) * 0.5;
    float time = u_time * 0.2 ;

    float c = 0.0;
    for (int n = 1; n < 4; n++) {
        float i = float(n);
        p += vec2(sin(i * p.y + time), cos(i * p.x + time));
        c += length(vec2(1.0 / sin(p.x + i), 1.0 / cos(p.y + i)));
    }

     c /= 12.0;
    c = (energy * 2.0 ) - pow(c, 0.1);

    vec3 deepWater = vec3(0.01, 0.1, 0.2)  ;
    vec3 lightWater = vec3(0.1, 0.5, 0.6) ;

    float highlights = smoothstep(0.6, 0.9, c);

    vec3 finalColor = mix(deepWater, lightWater, c);
    finalColor += highlights * (0.3 + energy ) ;

    float vignette = 1.0 - length(uv - 0.5) * 1.2;
    finalColor *= clamp(vignette, 0.0, 1.0);

    gl_FragColor = vec4(finalColor, 1.0);
}
