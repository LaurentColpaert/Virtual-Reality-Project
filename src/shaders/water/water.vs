#version 330 core

in vec3 position; 
in vec2 tex_coords; 
in vec3 normal; 

out vec3 v_frag_coord; 
out vec3 v_normal; 

uniform mat4 M; 
uniform mat4 itM; 
uniform mat4 V; 
uniform mat4 P; 
uniform float time;

struct Wave{
    vec2 dir;
    float steepness;
    float wavelength;
};

vec3 gerstner_wave(in Wave wave,vec3 position, inout vec3 tangent, inout vec3 binormal){
    float steepness = wave.steepness;
    float wavelength = wave.wavelength;
    float k = 2 * 3.14 / wavelength;
    float speed = sqrt(9.8/k);
    vec2 d = normalize(wave.dir);
    float f = k * (dot(d, position.xz) - speed * time);
    float a = 0.25 * steepness / k;
    
    tangent += vec3(1 - d.x * d.x * steepness * sin(f),d.x * steepness * cos(f), - d.x * d.y * steepness * sin(f));
    binormal += vec3(-d.x * d.y * steepness * sin(f),d.y * steepness * cos(f), 1 - d.y * d.y * steepness * sin(f));

    return vec3(d.x* a * cos(f), a * sin(f), d.y * a *cos(f));
} 

void main(){ 
    Wave wave;
    vec3 p = position;
    vec3 tangent = vec3(0.0);
    vec3 binormal = vec3(0.0);

    //---Wave 1-----
    wave.dir = vec2(1.0,0.0);
    wave.steepness = 0.5;
    wave.wavelength = 30;
    p += gerstner_wave(wave,position,tangent,binormal);

    //---Wave 2-----
    wave.dir = vec2(1.0,0.6);
    wave.steepness = 0.5;
    wave.wavelength = 15;
    p += gerstner_wave(wave,position,tangent,binormal);

    //---Wave 3-----
    wave.dir = vec2(1.0,1.3);
    wave.steepness = 0.5;
    wave.wavelength = 9;
    p += gerstner_wave(wave,position,tangent,binormal);

    vec4 frag_coord = M*vec4(p, 1.0); 
    gl_Position = P*V*frag_coord; 
    v_frag_coord = frag_coord.xyz;

    v_normal = normalize(cross(binormal,tangent));
};