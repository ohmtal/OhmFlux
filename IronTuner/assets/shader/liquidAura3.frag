//---------------------------------------------------------
// Liquid Aura III
// - but with more random flow
// - and energy(rms) saturation only
// - less loops with opengl ES
//---------------------------------------------------------

precision mediump float;

out vec4 FragColor;

uniform float u_time;
uniform vec2 u_res;
uniform float u_rmsL;
uniform float u_rmsR;
uniform float u_freqs[32];  // frequency FFT data

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - vec3(K.w));
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}


//---------------------------------------------------------
// Liquid
// - but with more random flow
// - and energy(rms) saturation only

// 4/2 low gpu usage
// 16/4 was my default
#ifdef GL_ES
    const float texLoops = 4.0; //8.0 ;min 4,  6 is ok
    const float starLoops = 4.0; //4.0 ; 4
#else
    const float texLoops = 8.0; //8.0 ;min 4,  6 is ok
    const float starLoops = 4.0; //4.0 ; 4
#endif

const float speed = 0.3;
//---------------------------------------------------------

vec3 finalColor = vec3(0.0,0.0,0.0);

void main() {



    vec2 uv = gl_FragCoord.xy / u_res.xy;
    float energy = (u_rmsL + u_rmsR) * 0.5;
    float bass = u_freqs[0];

    float c = 0.0;
    float time = u_time * 0.2 ;
 //.......

// using different times...
float t1 = u_time * 0.173 * speed;
float t2 = u_time * 0.241 * speed;


const float edgeSoftness = 0.15;

for(float i = 1.0; i < texLoops; i++) {
    // near Golden ratio
    uv.x += 0.3 / i * sin(i * 3.11 * uv.y + t1);
    uv.y += 0.4 / i * cos(i * 2.69 * uv.x + t2);

    // lightness
     c += (0.08 / (abs(uv.x) + edgeSoftness));

    // move time
    t1 += 0.05;
    t2 += 0.03;
}


if (true) {
    float hue = fract(time * speed  );
    float dynamicSaturation = 0.1 + (energy * 8.0);
    vec3 color = hsv2rgb(vec3(hue, dynamicSaturation, 0.5));
//     if ( energy < 0.1 )  color *= 0.0;

     c /= texLoops; //4.0;
    float sparkle = pow(abs(c), 16.0 );
    vec3 causticColor = vec3(0.2, 0.6, 0.7) * sparkle;
    finalColor += color + causticColor + (causticColor /** highs*/ * 2.0);
} else {
    //blueish
    finalColor +=  vec3(c * 0.1, c * 0.2, c * energy  );

}



// ------

    //  starfield :D
    for(float i = 0.0; i < starLoops; i++) {
        float depth = fract(i * 0.25 + u_time * speed * 0.08);
        float scale = mix(20.0, 0.5, depth);
        float fade = depth * smoothstep(1.0, 0.8, depth);

        vec2 gridPtr = uv * scale + (i * 45.32);
        vec2 id = floor(gridPtr);
        vec2 gv = fract(gridPtr) - 0.5;

        float rand = fract(sin(dot(id, vec2(12.9898, 78.233) + i)) * 43758.5453);

//          if(rand < energy)
        {
            float dist = length(gv);
            float star = 0.002 / dist;
            float glow = 0.1 * bass / dist;
            float mask = smoothstep(0.5, 0.2, dist);
            vec3 color = vec3(1.0, 1.0, 1.0); //vec3(0.7, 0.8, 1.0) + 0.3 * sin(vec3(0, 1, 2) + i);
            finalColor += color * (star + glow) * fade * mask;
        }
    }


    float darken = 1.0 - length(uv - 0.5) * 0.8;
    finalColor *= clamp(darken, 0.0, 1.0);

    FragColor = vec4(finalColor, 1.0);
}
