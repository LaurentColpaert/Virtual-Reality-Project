#ifndef SKYBOX_H
#define SKYBOX_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "./simple_shader.h"
#include "./object.h"

class Skybox{
public:
    GLuint sky_texture;
    Shader skybox_shader = Shader(PATH_TO_SHADER "/sky_box/sky.vs", PATH_TO_SHADER "/sky_box/sky.fs");
    Object skybox_cube = Object(PATH_TO_OBJECTS "/cube.obj");

    Skybox(){
	    skybox_cube.makeObject(skybox_shader);

        glGenTextures(1, &sky_texture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, sky_texture);
        // texture parameters
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	

        //This is the image you will use as your skybox
        std::string pathToCubeMap = PATH_TO_TEXTURE "/sky_box/";
        std::map<std::string, GLenum> facesToLoad = { 
            {pathToCubeMap + "right.png",GL_TEXTURE_CUBE_MAP_POSITIVE_X},
            {pathToCubeMap + "top.png",GL_TEXTURE_CUBE_MAP_POSITIVE_Y},
            {pathToCubeMap + "front.png",GL_TEXTURE_CUBE_MAP_POSITIVE_Z},
            {pathToCubeMap + "left.png",GL_TEXTURE_CUBE_MAP_NEGATIVE_X},
            {pathToCubeMap + "bottom.png",GL_TEXTURE_CUBE_MAP_NEGATIVE_Y},
            {pathToCubeMap + "back.png",GL_TEXTURE_CUBE_MAP_NEGATIVE_Z},
        };
        //load the six faces, you need to complete the function
        for (std::pair<std::string, GLenum> pair : facesToLoad) {
            loadCubemapFace(pair.first.c_str(), pair.second);
        }  
    }

    void set(){
        glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP,sky_texture);
    }
    
    void draw(Camera camera){
        /**
         * The GL-LEQUAL and GL_LESS is use to make sure that it render the skybox eventhough it's far away from the scene  
        */
        glDepthFunc(GL_LEQUAL);
		skybox_shader.use();
		skybox_shader.setMatrix4("V", camera.GetViewMatrix());
		skybox_shader.setMatrix4("P", camera.GetProjectionMatrix());
		
		//Activate and bind the texture for the cubemap
		skybox_shader.setInteger("cubemapTexture", 0);
		
		skybox_cube.draw();
		glDepthFunc(GL_LESS);
    }

private:
    void loadCubemapFace(const char * path, const GLenum& targetFace){
        int imWidth, imHeight, imNrChannels;
        //Load the image using stbi_load
        unsigned char* data = stbi_load(path, &imWidth, &imHeight, &imNrChannels, 0);
        if (data)
        {
            //Send the image to the the buffer
            glTexImage2D(targetFace, 0, GL_RGB, imWidth, imHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
        else {
            std::cout << "Failed to Load texture" << std::endl;
            const char* reason = stbi_failure_reason();
            std::cout << reason << std::endl;
        }
        //Don't forget to free the memory
        stbi_image_free(data);
    }
};
#endif