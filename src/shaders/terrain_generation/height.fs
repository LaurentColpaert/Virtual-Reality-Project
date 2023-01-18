#version 330 core

out vec4 FragColor;

in float Height;
in vec3 v_normal;

struct DirLight{
    vec3 direction;
    float ambient;
    float diffuse;
    float specular;
};

uniform DirLight dir_light;
uniform vec3 u_view_pos;
uniform vec3 v_frag_coord;

void main()
{
    float nb_pixel = 255.0;
    //float h = Height;
    float h = (Height)/64.0f;	// shift and scale the height in to a grayscale value
    vec3 color = vec3(h);

    //Water
    if( Height < 10)
        color = vec3(47,165,201)/nb_pixel;
    //Grass
    else if(Height<30)
        color = vec3(33, 152, 56)/nb_pixel;
    else if(Height <40)
        color = vec3(152, 115, 33)/nb_pixel;
    else
        color = vec3(255, 255, 255)/nb_pixel;

    //Directional light
    vec3 norm = normalize(v_normal);
    vec3 lightDir = normalize(dir_light.direction);  
    float diff = max(dot(norm, lightDir), 0.0);
    float dir_diffuse = dir_light.diffuse * diff;  
    vec3 viewDir = normalize(u_view_pos - v_frag_coord);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 30);
    float dir_specular = dir_light.specular * spec ;  
    float dir_light = dir_light.ambient +  (dir_diffuse + dir_specular);


    FragColor = vec4(color*1.5*dir_light, 1.0);
}