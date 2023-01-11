#version 330 core
precision mediump float;

out vec4 FragColor;

in vec3 v_frag_coord;
in vec3 v_normal;
in vec4 frag_pos_lightspace;

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
uniform vec3 u_view_pos;
uniform float shininess;
uniform vec3 materialColour;
uniform sampler2D shadowMap;

float specularCalculation(vec3 N, vec3 L, vec3 V ){
    vec3 R = reflect (-L,N);
    float cosTheta = dot(R , V);
    float spec = pow(max(cosTheta,0.0), 32.0);
    return light.specular_strength * spec;
}

float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(v_normal);
    vec3 lightDir = normalize(light.light_pos - v_frag_coord);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}
void main() {
    vec3 N = normalize(v_normal);
    vec3 L = normalize(light.light_pos - v_frag_coord);
    vec3 V = normalize(u_view_pos - v_frag_coord);
    float specular = specularCalculation( N, L, V);
    float diffuse = light.diffuse_strength * max(dot(N,L),0.0);
    float distance = length(light.light_pos - v_frag_coord);
    float attenuation = pow((light.constant + light.linear * distance + light.quadratic * distance * distance),-1);
    float shadow = ShadowCalculation(frag_pos_lightspace);
    float light = light.ambient_strength + (1.0 - shadow) * attenuation * (diffuse + specular);
    FragColor = vec4(materialColour * vec3(light), 1.0);
}