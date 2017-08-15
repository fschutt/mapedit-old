R"(

#version 130
in vec4 delta;

#ifdef GL_ES
	precision mediump float;
#endif

uniform float time;
uniform vec2 mouse;
uniform vec2 resolution;

float sdBox( vec3 p, vec3 b )
{
  vec3 d = abs(p) - b;
  return min(max(d.x,max(d.y,d.z)),0.0) + length(max(d,0.0));
}

float sdBox( vec2 p, vec2 b )
{
  vec2 d = abs(p) - b;
  return min(max(d.x,d.y),0.0) + length(max(d,0.0));
}


vec3 genNorgenNormalmal(vec3 p)
{
    const float d = 0.01;
    return normalize( vec3(
        map(p+vec3(  d,0.0,0.0))-map(p+vec3( -d,0.0,0.0)),
        map(p+vec3(0.0,  d,0.0))-map(p+vec3(0.0, -d,0.0)),
        map(p+vec3(0.0,0.0,  d))-map(p+vec3(0.0,0.0, -d)) ));
}

void main()
{
    vec2 pos = (gl_FragCoord.xy*2.0 - resolution.xy) / resolution.y;
    vec3 camPos = vec3(-0.5,0.0,3.0);
    vec3 camDir = normalize(vec3(0.3, 0.0, -1.0));
    camPos -=  vec3(0.0,0.0,time*3.0);
    vec3 camUp  = normalize(vec3(0.5, 1.0, 0.0));
    vec3 camSide = cross(camDir, camUp);
    float focus = 1.8;

    vec3 rayDir = normalize(camSide*pos.x + camUp*pos.y + camDir*focus);
    vec3 ray = camPos;
    int march = 0;
    float d = 0.0;

    float total_d = 0.0;
    const int MAX_MARCH = 64;
    const float MAX_DIST = 100.0;
    for(int mi=0; mi<MAX_MARCH; ++mi) {
        d = map(ray);
        march=mi;
        total_d += d;
        ray += rayDir * d;
        if(d<0.001) {break; }
        if(total_d>MAX_DIST) {
            total_d = MAX_DIST;
            march = MAX_MARCH-1;
            break;
        }
    }

    float glow = 0.0;

    float fog = min(1.0, (1.0 / float(MAX_MARCH)) * float(march))*1.0;
    vec3  fog2 = 0.01 * vec3(3, 4, 5) * total_d;
    gl_FragColor = vec4(vec3(0.15, 0.15, 3)*fog + fog2, 1.0);
}

uniform vec4 lineColor;

out vec4 color;

void main()
{
	color = vec4(0.0f, 0.0f, 0.0f, 1.0f);

}

)"
