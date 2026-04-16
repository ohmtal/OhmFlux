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
    vec2 uv = gl_FragCoord.xy / u_res.xy;
    uv = uv * 2.0 - 1.0;
    uv.x *= u_res.x / u_res.y;

    float rmsAvg = (u_rmsL + u_rmsR) * 0.5;
    float time = u_time * 0.3 + (rmsAvg * 0.1);


    float color = 0.0;

    for(float i = 1.0; i < 4.0; i++) {
        uv.x += 0.3 / i * sin(i * 3.0 * uv.y + time);
        uv.y += 0.4 / i * cos(i * 2.5 * uv.x + time);

        // was: 0.015
        float edgeSoftness = 0.3;
        color += (0.04 / (abs(uv.x) + edgeSoftness)) * i;
    }

    vec3 baseColor = vec3(0.1, 0.2, 0.5); // deep blue

     float hue = fract(u_time * 0.05);
//     vec3 glowColor = hsv2rgb(vec3(hue, 0.7, 0.9));
    vec3 glowColor = hsv2rgb(vec3(hue, 0.4, 0.7));


    vec3 finalColor = baseColor * color + (glowColor * color * rmsAvg * 2.0);

    // Subtle grain noise to prevent color banding
    // ... fast random generator  ...
//     float noise = fract(sin(dot(uv, vec2(12.9898, 78.233))) * 43758.5453);
//      finalColor += noise * 0.1;


    FragColor = vec4(finalColor * 0.5, 1.0);
}
