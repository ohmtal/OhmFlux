precision mediump float;

out vec4 FragColor;

uniform float u_time;
uniform vec2 u_res;
uniform float u_rmsL;
uniform float u_rmsR;


void main() {
    vec2 uv = gl_FragCoord.xy / u_res.xy;
    vec2 baseUV = uv;
    vec2 p = uv * 3.0;

    float energy = (u_rmsL + u_rmsR) * 0.5 + 0.01;
    float time = u_time * 0.25 ;

    for(float i = 1.0; i < 2.0; i++) {
        uv.x += 0.1 / i * sin(i * 3.5 * uv.y + time);
        uv.y += 0.1 / i * cos(i * 2.5 * uv.x + time);
    }


    vec3 finalColor = vec3(0.1, 0.0, 0.1);;





    for(float i = 0.0; i < 4.0; i+=0.33) {
        float depth = fract(i * 0.25 + time * 0.1);
        float scale = mix(20.0, 0.5, depth);
        float fade = depth * smoothstep(1.0, 0.8, depth);

        vec2 gridPtr = uv * scale + (i * 45.32);
        vec2 id = floor(gridPtr);
        vec2 gv = fract(gridPtr) - 0.5;

        float rand = fract(sin(dot(id, vec2(12.9898, 78.233) + i)) * 43758.5453);

        if(rand < energy )
        {
            float dist = length(gv);
            float star = 0.02 / dist;
            float glow = 0.1; //(0.01 * energy) / dist;
            float mask = smoothstep(0.5, 0.0, dist);
            vec3 color = vec3(0.01, 0.01, 0.01) + energy  * sin(vec3(0.0, 1.0, 2.0) + rand * 4.0);
            finalColor += color * (star + glow) * fade * mask;
        }
    }

    float grain = fract(sin(dot(uv, vec2(12.9898, 78.233))) * time );
    grain = grain  / energy  * 2.0; // range -0.025 to 0.025
    finalColor /= grain;

//     float darken = 1.0 - length(uv - 0.5) * 1.5;
//     finalColor *= clamp(darken, 0.0, 1.0);

    FragColor = vec4(finalColor, 1);
}
