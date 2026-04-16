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

//     for(float i = 1.0; i < 4.0; i++) {
//         uv.x += 0.3 / i * sin(i * 3.0 * uv.y + time);
//         uv.y += 0.4 / i * cos(i * 2.5 * uv.x + time);
//         color += (0.05 / (abs(uv.x) + 0.01)) * i;
//     }

    for(float i = 1.0; i < 4.0; i++) {
        uv.x += 0.3 / i * sin(i * 3.0 * uv.y + time);
        uv.y += 0.4 / i * cos(i * 2.5 * uv.x + time);

        float edgeSoftness = 0.03;
        color += (0.04 / (abs(uv.x) + edgeSoftness)) * i;
    }



    vec3 deepBlue = vec3(0.02, 0.04, 0.1);
    float hue = fract(u_time * 0.05);
    float dynamicSaturation = /*0.5 +*/ (rmsAvg * 2.0);
    vec3 accentColor = hsv2rgb(vec3(hue, dynamicSaturation, 0.8));

    vec3 finalColor = mix(deepBlue, accentColor, clamp(color * 0.4, 0.0, 5.0));


    finalColor += accentColor /** rmsAvg*/ * 0.3;

    //vignette
    float vignette = 1.0 - length(static_uv - 0.5) * 1.2;
    finalColor *= clamp(vignette, 0.0, 1.0);



    FragColor = vec4(finalColor, 1.0);
}

