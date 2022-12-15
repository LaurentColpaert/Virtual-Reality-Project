#version 330 core

out vec4 FragColor;
precision mediump float; 
//Get the cube map
uniform samplerCube cubemapSampler; 
in vec3 texCoord_v; 

void main() { 
    //Use the coordinate and the cube map
    FragColor = texture(cubemapSampler,texCoord_v); 
}