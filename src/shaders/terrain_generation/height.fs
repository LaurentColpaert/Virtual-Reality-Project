#version 330 core

out vec4 FragColor;

in float Height;

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
    else if(Height<40)
        color = vec3(33, 152, 56)/nb_pixel;
    else if(Height <55)
        color = vec3(152, 115, 33)/nb_pixel;
    else
        color = vec3(0, 0, 0)/nb_pixel;
    FragColor = vec4(color*h, 1.0);
}