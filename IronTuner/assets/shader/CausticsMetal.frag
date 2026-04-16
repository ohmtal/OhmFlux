// # Liquid Metal with bars
precision mediump float;

out vec4 FragColor;

uniform float u_time;
uniform vec2 u_res;
uniform float u_rmsL;
uniform float u_rmsR;

uniform float u_freqCount;  // frequencies (bands) count MAX 32!
uniform float u_freqs[32];  // frequency FFT data

// Converts Hue, Saturation, Value to RGB
vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}



void main() {

    vec3 finalColor;
    vec2 uv = gl_FragCoord.xy / u_res.xy;
    vec2 p = uv * 3.0;

    float energy = (u_rmsL + u_rmsR) * 0.5;
    float time = u_time * 0.2 ;


    // ~~~~~~~~ LIQUID ~~~~~~~~~~
    float c = 0.0;

    for(float i = 1.0; i < 16.0; i++) {
        uv.x += 0.3 / i * sin(i * 3.0 * uv.y + time);
        uv.y += 0.4 / i * cos(i * 2.5 * uv.x + time);

        // was: 0.015
        float edgeSoftness = 0.015;
        c += (0.08 / (abs(uv.x) + edgeSoftness)) * i;
    }



    c /= 256.0 * (energy + 0.1);
    c =  2.0 - pow(c, 2.0);




    vec3 deepWater = vec3(0.01, 0.01, 0.01)  ;
    vec3 lightWater = vec3(0.1, 0.1, 0.1) ;



     float highlights = smoothstep(0.6, 0.9, c);

     finalColor += mix(deepWater, lightWater, c);
    finalColor += highlights * ( energy * 0.2 ) ;




//     // --- Bars (Wide Version) ---
//     vec2 uv_center = (gl_FragCoord.xy * 2.0 - u_res.xy) / min(u_res.y, u_res.x);
//
//     float aspect = u_res.x / u_res.y;
//     float barAreaWidth = 1.8; // Total width of the spectrum display
//     float xPos = uv_center.x + (barAreaWidth * 0.5);
//
//     if (xPos >= 0.0 && xPos <= barAreaWidth) {
//         // Current bar index (0 to 31 (default)
//
//         float barCount = u_freqCount;
//         int idx = int((xPos / barAreaWidth) * (barCount - 0.01));
//         float val = u_freqs[idx];
//         if (idx == 0) val*=0.85;
//
//         // --- Horizontal Bar Width Logic ---
//         // Get local X coordinate within a single bar (0.0 to 1.0)
//         float localX = fract((xPos / barAreaWidth) * barCount);
//
//         // Gap size: 0.1 means 10% gap, 90% bar width.
//         // Increase 0.1 to 0.3 if you want more space between bars.
//         float gap = 0.15;
//         float horizontalMask = smoothstep(0.0, 0.05, localX - gap) *
//         smoothstep(1.0, 0.95, localX);
//
//         // --- Vertical Height Logic ---
//         float h = val * 0.8; // Scale height
//         float verticalMask = smoothstep(h, h - 0.02, uv_center.y + 0.85);
//         verticalMask *= step(-0.92, uv_center.y); // Bottom crop
//
//         // Combine masks
//         float finalBarMask = horizontalMask * verticalMask;
//
//         // --- Dynamic Color ---
//         // Full rainbow shift from left to right + time animation
//         float hue = fract(float(idx) / barCount + u_time * 0.05);
//         vec3 barColor = hsv2rgb(vec3(hue, 0.9, 0.9));
//
//         // Add to final color with a bit of "bloom/glow"
//         finalColor += barColor * finalBarMask * 0.7;
//
//     }



    // STARS
    for(float i = 0.0; i < 4.0; i++) {
        float depth = fract(i * 0.25 + time * 0.1);
        float scale = mix(20.0, 0.5, depth);
        float fade = depth * smoothstep(1.0, 0.8, depth);

        vec2 gridPtr = uv * scale + (i * 45.32);
        vec2 id = floor(gridPtr);
        vec2 gv = fract(gridPtr) - 0.5;

        float rand = fract(sin(dot(id, vec2(12.9898, 78.233) + i)) * 43758.5453);


        //         if(rand < energy)
        {
            float dist = length(gv);
            float star = 0.002 / dist;
            float glow = 0.05 * energy / dist;
            float mask = smoothstep(0.5, 0.2, dist);
            vec3 color = vec3(0.7, 0.8, 1.0) + 0.3 * sin(vec3(0, 1, 2) + i);
             finalColor += color * (star + glow) * fade * mask;
        }
    }


    // -------------

    float vignette = 1.0 - length(uv - 0.5) * 1.2;
    finalColor *= clamp(vignette, 0.0, 1.0);

    FragColor = vec4(finalColor, 1.0);
}
