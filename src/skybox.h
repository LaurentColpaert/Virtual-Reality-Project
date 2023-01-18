/**
* @brief This header file defines the Skybox class
*
* @author Adela Surca & Laurent Colpaert
*
* @project OpenGL project
*
**/
#ifndef SKYBOX_H
#define SKYBOX_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "./simple_shader.h"
#include "./object.h"

/**
* @brief Class that handle the skybox and the texture linked to it
**/
class Skybox{
public:
    GLuint sky_texture;
    Shader skybox_shader = Shader(PATH_TO_SHADER "/sky_box/sky.vs", PATH_TO_SHADER "/sky_box/sky.fs");
    Object skybox_cube = Object(PATH_TO_OBJECTS "/cube.obj");

    /** Constructor **/
    Skybox(){
        //create the cube
	    skybox_cube.makeObject(skybox_shader);

        //load the textures in the right buffer
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
        //Load the six faces, you need to complete the function
        for (std::pair<std::string, GLenum> pair : facesToLoad) {
            loadCubemapFace(pair.first.c_str(), pair.second);
        }  
        glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP,sky_texture);
    }

    GLuint getSkyTexture(){
        return sky_texture; 
    }
    
    /** Bind your vertex arrays and call glDrawArrays and setup the VP matrix **/
    void draw(Camera camera){
        //The GL-LEQUAL and GL_LESS is use to make sure that it render the skybox eventhough it's far away from the scene  
        glDepthFunc(GL_LEQUAL);
		skybox_shader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, sky_texture);
		skybox_shader.setInteger("cubemapTexture", 0);
		skybox_shader.setMatrix4("V", camera.GetViewMatrix());
		skybox_shader.setMatrix4("P", camera.GetProjectionMatrix());
		
		//Activate and bind the texture for the cubemap
		skybox_cube.draw();
		glDepthFunc(GL_LESS);
    }

private:
    /** Load the texture with the appropriate parameters and link it the right face of the cube**/
    void loadCubemapFace(const char * path, const GLenum& targetFace){
        int imWidth, imHeight, imNrChannels;
        //Load the image using stbi_load
        unsigned char* data = stbi_load(path, &imWidth, &imHeight, &imNrChannels, 0);
        if (data)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, sky_texture);
            //Send the image to the the buffer
            glTexImage2D(targetFace, 0, GL_RGB, imWidth, imHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        }
        else {
            std::cout << "Failed to Load texture" << std::endl;
            const char* reason = stbi_failure_reason();
            std::cout << reason << std::endl;
        }
        stbi_image_free(data);
    }
};
#endif