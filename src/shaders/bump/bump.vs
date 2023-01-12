#version 330 core

in vec3 aPos;
in vec3 aNormal;
in vec2 aTexCoords;
in vec3 aTangent;
in vec3 aBitangent;

out vec3 v_frag_coord; 
out vec3 v_normal; 
out vec3 FragPos;
out vec2 TexCoords;
out vec3 TangentLightPos;
out vec3 TangentViewPos;
out vec3 TangentFragPos;
out vec4 frag_pos_lightspace;


uniform mat4 M; 
uniform mat4 V; 
uniform mat4 P; 

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform mat4 lightspace; 

void main()
{
    vec4 frag_coord = M*vec4(aPos, 1.0); 
    FragPos = vec3(M * vec4(aPos, 1.0));   
    TexCoords = aTexCoords;
    
    mat3 normalMatrix = transpose(inverse(mat3(M)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    
    mat3 TBN = transpose(mat3(T, B, N));    
    TangentLightPos = TBN * lightPos;
    TangentViewPos  = TBN * viewPos;
    TangentFragPos  = TBN * FragPos;
        
    gl_Position = P*V*M*vec4(aPos, 1.0);
    
    frag_pos_lightspace = lightspace * frag_coord;
}