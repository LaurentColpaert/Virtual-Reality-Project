#version 330 core
precision mediump float;

out vec4 FragColor;

in vec3 v_frag_coord;
in vec3 v_normal;

uniform vec3 u_view_pos;
struct Light{
    vec3 light_pos;
    float ambient_strength;
    float diffuse_strength;
    float specular_strength;
    float constant;
    float linear;
    float quadratic;
};

uniform Light light;
uniform float shininess;
uniform vec3 materialColour;

uniform samplerCube cubemapSampler;


float far = 1000;
float near = 0.01; 
float specularCalculation(vec3 N, vec3 L, vec3 V ){
    vec3 R = reflect (-L,N);
    float cosTheta = dot(R , V);
    float spec = pow(max(cosTheta,0.0), 32.0);
    return light.specular_strength * spec;
}


void main() {
    vec3 N = normalize(v_normal);
    vec3 L = normalize(light.light_pos - v_frag_coord);
    vec3 V = normalize(u_view_pos - v_frag_coord);
    float specular = specularCalculation( N, L, V);
    float diffuse = light.diffuse_strength * max(dot(N,L),0.0);
    float distance = length(light.light_pos - v_frag_coord);
    float attenuation = pow((light.constant + light.linear * distance + light.quadratic * distance * distance),-1);
    float light = light.ambient_strength + attenuation * (diffuse + specular);
    vec3 R = reflect(-V,N);

    FragColor = vec4(materialColour * vec3(light) * texture(cubemapSampler,R).xyz, 1.0);
}