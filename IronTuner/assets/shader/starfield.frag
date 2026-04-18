precision highp float;

out vec4 FragColor;

uniform float u_time;
uniform vec2 u_res;
uniform float u_rmsL;
uniform float u_rmsR;


//---------------------------------------------------------
// Starfield
#ifdef GL_ES
const float starLoops = 4.0; //4.0 ; 4
const float starLoopInc = 1.0; //0.4
#else
const float starLoops = 4.0; //4.0 ; 4
const float starLoopInc = 0.4; //0.4
#endif

const float speed = 0.25;
//---------------------------------------------------------


void main() {
    vec2 uv = gl_FragCoord.xy / u_res.xy;
    vec2 baseUV = uv;
    vec2 p = uv * 3.0;

    float energy = (u_rmsL + u_rmsR) * 0.5 + 0.01;
    float time = u_time * speed ;

    for(float i = 1.0; i < 2.0; i++) {
        uv.x += 0.1 / i * sin(i * 3.5 * uv.y + time);
        uv.y += 0.1 / i * cos(i * 2.5 * uv.x + time);
    }

    vec3 finalColor = vec3(0.1, 0.0, 0.1);;

//     for(float i = 0.0; i < 4.0; i+=0.44) {

    for(float i = 0.0; i < starLoops; i+=starLoopInc) {
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
            vec3 color = vec3(0.01, 0.01, 0.01) + energy * 4.0 * sin(vec3(0.0, 1.0, 2.0) + rand * 4.0);
            finalColor += color * (star + glow) * fade * mask;
        }
    }



    //darken
    float darken = 1.0 - length(uv - 0.5) * 1.5;
    finalColor *= clamp(darken, 0.0, 1.0);



    FragColor = vec4(finalColor, 1);
}


// precision mediump float;
//
// out vec4 FragColor;
//
// uniform float u_time;
// uniform vec2 u_res;
// uniform float u_rmsL;
// uniform float u_rmsR;
//
//
// void main() {
//     vec2 uv = gl_FragCoord.xy / u_res.xy;
//     vec2 baseUV = uv;
//     vec2 p = uv * 3.0;
//
//     float energy = (u_rmsL + u_rmsR) * 0.5 + 0.01;
//     float time = u_time * 0.2 ;
//
//     float c = 0.0;
//
// //      float timeVal = 0.1 + mod(u_time * 0.1, 14.0);
//
// //     timeVal = 8;
//     for(float i = 1.0; i < 2.0; i++) {
// //         if (i > timeVal) break;
//         uv.x += 0.1 / i * sin(i * 3.5 * uv.y + time);
//         uv.y += 0.1 / i * cos(i * 2.5 * uv.x + time);
//
//         c += (0.08 / (abs(uv.x) + 0.15)) * i ;
//     }
//
//         c /= 16;
//         c *= (energy + 0.5);
//
//
// //     c /= 256.0 * (energy + 0.1);
// //     c =  2.0 - pow(c, 2.0);
//
//     vec3 deepWater = vec3(0.2, 0.1, 0.2)  ;
//     vec3 lightWater = vec3(0.1, 0.1, 0.1) ;
//
//
//
//     float highlights = smoothstep(0.6, 0.9, c);
//
//     vec3 finalColor = mix(deepWater, lightWater, c);
//     finalColor += highlights * ( energy * 0.1 ) ;
//
//
//     // from starfield :D
//     for(float i = 0.0; i < 8.0; i+=1.5) {
//         float depth = fract(i * 0.25 + time * 0.1);
//         float scale = mix(20.0, 0.5, depth);
//         float fade = depth * smoothstep(1.0, 0.8, depth);
//
//         vec2 gridPtr = uv * scale + (i * 45.32);
//         vec2 id = floor(gridPtr);
//         vec2 gv = fract(gridPtr) - 0.5;
//
//         float rand = fract(sin(dot(id, vec2(12.9898, 78.233) + i)) * 43758.5453);
//
//         if(rand < energy )
//         {
//             float dist = length(gv);
//             float star = 0.02 / dist;
//             float glow = (0.05 * energy) / dist;
//             float mask = smoothstep(0.5, 0.2, dist);
//             vec3 color = vec3(0.7, 0.8, 1.0) + 0.3 * sin(vec3(0, 1, 2) + i);
//             finalColor += color * (star + glow) * fade * mask;
//         }
//     }
//
//
//     float vignette = 1.0 - length(baseUV - 0.5) * 1.2;
//     finalColor *= clamp(vignette, 0.0, 1.0);
//
//     FragColor = vec4(finalColor, 1.0);
// }



// precision mediump float;
//
// out vec4 FragColor;
//
// uniform float u_time;
// uniform vec2 u_res;
// uniform float u_rmsL;
// uniform float u_rmsR;
// uniform float u_freqCount;
// uniform float u_freqs[32];
//
//
// void main() {
//     vec2 uv = (gl_FragCoord.xy - 0.5 * u_res.xy) / min(u_res.y, u_res.x);
//
//     float energy = (u_rmsL + u_rmsR) * 0.5;
//
//     float time = u_time * 0.3 ;
//
//     vec3 finalColor = vec3(0.0);
//
//     for(float i = 0.0; i < 4.0; i++) {
//         float depth = fract(i * 0.25 + time * 0.1);
//         float scale = mix(20.0, 0.5, depth);
//         float fade = depth * smoothstep(1.0, 0.8, depth);
//
//         vec2 gridPtr = uv * scale + (i * 45.32);
//         vec2 id = floor(gridPtr);
//         vec2 gv = fract(gridPtr) - 0.5;
//
//         float rand = fract(sin(dot(id, vec2(12.9898, 78.233) + i)) * 43758.5453);
//
//
//         if(rand > 0.9) {
//             float dist = length(gv);
//             float star = 0.002 / dist;
//             float glow = 0.05 * energy / dist;
//             float mask = smoothstep(0.5, 0.2, dist);
//             vec3 color = vec3(0.7, 0.8, 1.0) + 0.3 * sin(vec3(0, 1, 2) + i);
//             finalColor += color * (star + glow) * fade * mask;
//         }
//     }
//
// //      finalColor += vec3(0.02, 0.03, 0.05) * energy;
//      finalColor += vec3(0.04, 0.06, 0.1) * energy;
//
//     FragColor = vec4(finalColor, 1.0);
// }
