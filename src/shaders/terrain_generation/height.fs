#version 330 core

out vec4 FragColor;

in float Height;
in vec3 v_normal;
in vec3 v_frag_coord;

struct DirLight{
    vec3 direction;
    float ambient;
    float diffuse;
    float specular;
};

uniform DirLight dir_light;
uniform vec3 u_view_pos;

void main()
{
    float nb_pixel = 255.0;
    //float h = Height;
    float h = (Height)/64.0f;	// shift and scale the height in to a grayscale value
    vec3 color = vec3(h);


    vec3 green = vec3(58, 116, 39)/nb_pixel ;
    vec3 brown = vec3(152, 115, 33)/nb_pixel;
    vec3 white = vec3(277,277,277)/nb_pixel;
    //Water
    if( Height > 10)
        color = vec3(47,165,201)/nb_pixel;
    //Grass
    if(Height>20){
        float gradient = smoothstep(30,45,Height);
        color = vec3(gradient*green + (1.0 - gradient) *green);
    }
    if(Height>30){
        color = brown;
    }
    if(Height >37){
        float gradient = smoothstep(37,41,Height);
        color = vec3(gradient*white + (1.0 - gradient) * brown);
    }
    if(Height>41){
        float gradient = smoothstep(47,50,Height);
        color = vec3(gradient*white + (1.0 - gradient) * white);
    }

    //Compute normals
    vec3 x = dFdx(v_frag_coord); // "FragPos = vec3(model * p);" from the tess eval
    vec3 y = dFdy(v_frag_coord);
    vec3 normal = inverse(mat3(mat4(1.0))) * normalize(cross(x, y));
    
    //Directional light
    vec3 norm = normalize(normal);
    vec3 lightDir = normalize(dir_light.direction);  
    float diff = max(dot(norm, lightDir), 0.0);
    float dir_diffuse = dir_light.diffuse * diff;  
    vec3 viewDir = normalize(u_view_pos - v_frag_coord);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 30);
    float dir_specular = dir_light.specular * spec ;  
    float dir_light = dir_light.ambient +  (dir_diffuse + dir_specular);

    FragColor = vec4(color, 1.0);
}