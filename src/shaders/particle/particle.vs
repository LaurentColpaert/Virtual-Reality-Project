#version 330 core

in vec2 tex_coord;
in vec3 position;



out vec2 TexCoords;
out vec4 ParticleColor;

uniform mat4 projection;
uniform mat4 view;
uniform vec3 offset;
uniform vec4 color;

void main()
{
    vec4 frag_pos = vec4(position+ offset,1.0);
    TexCoords = tex_coord;
    ParticleColor = color;
    gl_Position = projection * view * frag_pos;
}