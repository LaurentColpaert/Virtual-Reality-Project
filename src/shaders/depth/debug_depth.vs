#version 330 core

in vec3 position; 
in vec2 tex_coords; 
in vec3 normal; 

out vec2 TexCoords;


uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform mat4 itM;

void main()
{
    TexCoords = tex_coords;
    vec4 frag_coord = M*vec4(position, 1.0); 
    gl_Position = P*V*frag_coord; 
}