#ifndef TERRAIN_H
#define TERRAIN_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "./tess_shader.h"

const unsigned int NUM_PATCH_PTS = 4;

class Terrain{
public:
    TShader tessHeightMapShader = TShader(PATH_TO_SHADER "/terrain_generation/height.vs",PATH_TO_SHADER "/terrain_generation/height.fs",nullptr,PATH_TO_SHADER "/terrain_generation/height.tcs", PATH_TO_SHADER "/terrain_generation/height.tes");
    unsigned int terrainVAO, terrainVBO;
    std::vector<float> vertices;  
    int terrain_width = 0;
    int terrain_height = 0; 
    unsigned rez = 20;   

    Terrain(){
        // Load the image of the mipmaps
        unsigned int texture;
        glGenTextures(1, &texture);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
        
        // load image, create texture and generate mipmaps
        int width, height, nrChannels;
        // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
        unsigned char *data = stbi_load(PATH_TO_TEXTURE "/height_map_island.png", &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            tessHeightMapShader.setInteger("heightMap", 0);
            std::cout << "Loaded heightmap of size " << height << " x " << width << std::endl;
        }
        else
        {
            std::cout << "Failed to load texture" << std::endl;
        }
        stbi_image_free(data);
        terrain_width = width;
        terrain_height = height;

        // vertex generation
        for(unsigned i = 0; i <= rez-1; i++)
        {
            for(unsigned j = 0; j <= rez-1; j++)
            {
                vertices.push_back(-width/2.0f + width*i/(float)rez); // v.x
                vertices.push_back(0.0f); // v.y
                vertices.push_back(-height/2.0f + height*j/(float)rez); // v.z
                vertices.push_back(i / (float)rez); // u
                vertices.push_back(j / (float)rez); // v

                vertices.push_back(-width/2.0f + width*(i+1)/(float)rez); // v.x
                vertices.push_back(0.0f); // v.y
                vertices.push_back(-height/2.0f + height*j/(float)rez); // v.z
                vertices.push_back((i+1) / (float)rez); // u
                vertices.push_back(j / (float)rez); // v

                vertices.push_back(-width/2.0f + width*i/(float)rez); // v.x
                vertices.push_back(0.0f); // v.y
                vertices.push_back(-height/2.0f + height*(j+1)/(float)rez); // v.z
                vertices.push_back(i / (float)rez); // u
                vertices.push_back((j+1) / (float)rez); // v

                vertices.push_back(-width/2.0f + width*(i+1)/(float)rez); // v.x
                vertices.push_back(0.0f); // v.y
                vertices.push_back(-height/2.0f + height*(j+1)/(float)rez); // v.z
                vertices.push_back((i+1) / (float)rez); // u
                vertices.push_back((j+1) / (float)rez); // v
            }
        }
        std::cout << "Loaded " << rez*rez << " patches of 4 control points each" << std::endl;
        std::cout << "Processing " << rez*rez*4 << " vertices in vertex shader" << std::endl;
        Load_buffer();   
    }

    void draw(Camera camera,float width,float height){
        tessHeightMapShader.use();

        // view/projection transformations
        tessHeightMapShader.setMatrix4("projection",camera.GetProjectionMatrix());
        tessHeightMapShader.setMatrix4("view", camera.GetViewMatrix());
        tessHeightMapShader.setMatrix4("model", glm::mat4(1.0f));

        // render the terrain
        glBindVertexArray(terrainVAO);
        glDrawArrays(GL_PATCHES, 0, NUM_PATCH_PTS*rez*rez);
    }

    void destroy(){
        glDeleteVertexArrays(1, &terrainVAO);
        glDeleteBuffers(1, &terrainVBO);
    }
private:
    void Load_buffer(){
        glGenVertexArrays(1, &terrainVAO);
        glBindVertexArray(terrainVAO);

        glGenBuffers(1, &terrainVBO);
        glBindBuffer(GL_ARRAY_BUFFER, terrainVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

        // position attribute
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // texCoord attribute
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(sizeof(float) * 3));
        glEnableVertexAttribArray(1);
    }
};
#endif