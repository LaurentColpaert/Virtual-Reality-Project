#version 330 core

in vec3 FragPos;
in vec2 TexCoords;
in vec3 TangentLightPos;
in vec3 TangentViewPos;
in vec3 TangentFragPos;
in vec4 frag_pos_lightspace;

out vec4 FragColor;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;
uniform sampler2D shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

float ShadowCalculation(vec4 fragPosLightSpace){
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
    float shadow = currentDepth > closestDepth  ? 1.0 : 0.0;

    return shadow;
}


void main()
{           
    // // obtain normal from normal map in range [0,1]
    // vec3 normal = texture(normalMap, TexCoords).rgb;
    // // transform normal vector to range [-1,1]
    // normal = normalize(normal * 2.0 - 1.0);  // this normal is in tangent space
   
    // // get diffuse color
    // vec3 color = texture(diffuseMap, TexCoords).rgb;
    // // ambient
    // vec3 ambient = 0.1 * color;
    // // diffuse
    // vec3 lightDir = normalize(TangentLightPos - TangentFragPos);
    // float diff = max(dot(lightDir, normal), 0.0);
    // vec3 diffuse = diff * color;
    // // specular
    // vec3 viewDir = normalize(TangentViewPos - TangentFragPos);
    // vec3 reflectDir = reflect(-lightDir, normal);
    // vec3 halfwayDir = normalize(lightDir + viewDir);  
    // float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);

    // vec3 specular = vec3(0.2) * spec;
    // float shadow = ShadowCalculation(frag_pos_lightspace);
    // FragColor = vec4(ambient + (1.0-shadow) * (diffuse + specular), 1.0);

    vec3 color = vec3(0.2,0.5,0.9);
    vec3 normal = vec3(0.0,1.0,0.0);
    vec3 lightColor = vec3(1.0);
    // ambient
    vec3 ambient = 0.15 * lightColor;
    // diffuse
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(lightDir, normal), 0.0);
    vec3 diffuse = diff * lightColor;
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    float spec = 0.0;
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    spec = pow(max(dot(normal, halfwayDir), 0.0), 64.0);
    vec3 specular = spec * lightColor;    
    // calculate shadow
    float shadow = ShadowCalculation(frag_pos_lightspace);       
    vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * color;    
    
    FragColor = vec4(lighting, 1.0);
}

