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

void main(){ 
    vec2 dir = normalize(vec2(1,0));
    float k = 2;
    float speed = sqrt(9.8/k);
    float steepness = 0.5;
    float a = steepness / k;
    float f = k * (dot(dir, position.xz) - speed * time);
    // sin wave
    //position.y = a*sin(f);
    // Gerstner
    vec3 new_pos = vec3(position.x +dir.x* a * cos(f), a * sin(f), position.z + dir.y * a *cos(f));
    vec4 frag_coord = M*vec4(new_pos, 1.0); 
    gl_Position = P*V*frag_coord; 

    //normal
    vec3 tangent = vec3(1 - dir.x * dir.x * steepness * sin(f),dir.x * steepness * cos(f), - dir.x * dir.y * steepness * sin(f));
    vec3 binormal = vec3(-dir.x * dir.y * steepness * sin(f),dir.y * steepness * cos(f), 1 - dir.y * dir.y * steepness * sin(f));
    v_normal = vec3(itM * vec4(cross(binormal,tangent), 1.0)); 
    v_frag_coord = frag_coord.xyz; 
};