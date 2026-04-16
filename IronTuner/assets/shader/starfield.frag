precision mediump float;

out vec4 FragColor;

uniform float u_time;
uniform vec2 u_res;
uniform float u_rmsL;
uniform float u_rmsR;
uniform float u_freqCount;
uniform float u_freqs[32];


void main() {
    vec2 uv = (gl_FragCoord.xy - 0.5 * u_res.xy) / min(u_res.y, u_res.x);

    float energy = (u_rmsL + u_rmsR) * 0.5;

    float time = u_time * 0.3 ;

    vec3 finalColor = vec3(0.0);

    for(float i = 0.0; i < 4.0; i++) {
        float depth = fract(i * 0.25 + time * 0.1);
        float scale = mix(20.0, 0.5, depth);
        float fade = depth * smoothstep(1.0, 0.8, depth);

        vec2 gridPtr = uv * scale + (i * 45.32);
        vec2 id = floor(gridPtr);
        vec2 gv = fract(gridPtr) - 0.5;

        float rand = fract(sin(dot(id, vec2(12.9898, 78.233) + i)) * 43758.5453);


        if(rand > 0.9) {
            float dist = length(gv);
            float star = 0.002 / dist;
            float glow = 0.05 * energy / dist;
            float mask = smoothstep(0.5, 0.2, dist);
            vec3 color = vec3(0.7, 0.8, 1.0) + 0.3 * sin(vec3(0, 1, 2) + i);
            finalColor += color * (star + glow) * fade * mask;
        }
    }

//      finalColor += vec3(0.02, 0.03, 0.05) * energy;
     finalColor += vec3(0.04, 0.06, 0.1) * energy;

    FragColor = vec4(finalColor, 1.0);
}
