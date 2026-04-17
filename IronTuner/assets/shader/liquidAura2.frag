precision mediump float;

out vec4 FragColor;

uniform float u_time;
uniform vec2 u_res;
uniform float u_rmsL;
uniform float u_rmsR;

// Converts Hue, Saturation, Value to RGB
vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - vec3(K.w));
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}


void main() {
    vec2 static_uv = gl_FragCoord.xy / u_res.xy; // Für Vignette und Noise behalten
    vec2 uv = static_uv;


    float rmsAvg = (u_rmsL + u_rmsR) * 0.5;
    float time = u_time * 0.2;

    float color = 0.0;

//     float timeVal = 2.0 + mod(u_time * 0.001, 5.0);

    for(float i = 1.0; i < 5.9; i++) {
        uv.x += 0.3 / i * sin(i * 3.0 * uv.y + time);
        uv.y += 0.4 / i * cos(i * 2.5 * uv.x + time);

        float edgeSoftness = 0.09;
        color +=  (0.04 / (abs(uv.x) + edgeSoftness)) * i;
    }



    vec3 deepBlue = vec3(0.02, 0.04, 0.1);
    float hue = fract(u_time * 0.04);
    float dynamicSaturation = 0.1 + (/*rmsAvg **/ 4.0);
    vec3 accentColor = hsv2rgb(vec3(hue, dynamicSaturation, 0.8));

    vec3 finalColor = mix(deepBlue, accentColor, clamp(color * 0.4, 0.0, 5.0));


    finalColor += accentColor * rmsAvg * 0.2;

         // STARS
    for(float i = 0.0; i < 4.0; i++) {
        float depth = fract(i * 0.25 + time * 0.1);
        float scale = mix(20.0, 0.5, depth);
        float fade = depth * smoothstep(1.0, 0.8, depth);

        vec2 gridPtr = uv * scale + (i * 45.32);
        vec2 id = floor(gridPtr);
        vec2 gv = fract(gridPtr) - 0.5;

        float rand = fract(sin(dot(id, vec2(12.9898, 78.233) + i)) * 43758.5453 );


        if(rand < 0.5)
        {
            float dist = length(gv);
            float star = 1.0 / (dist * 4.0);
            float glow = 0.05 * rmsAvg / dist;
            float mask = smoothstep(0.5, 0.2, dist);
            vec3 color = vec3(0.7, 0.6, 0.8) + 0.3 * sin(vec3(0.0, 1.0, 2.0) + i);
            finalColor += color * (star + glow) * fade * mask;
        }
    }


    //vignette
    float vignette = 1.0 - length(static_uv - 0.5) * 1.2;
    finalColor *= clamp(vignette, 0.0, 1.0);



    FragColor = vec4(finalColor, 1.0);
}

