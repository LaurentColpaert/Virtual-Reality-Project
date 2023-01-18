/**
* @brief This header file defines the Terrain class.
*
* @author Adela Surca & Laurent Colpaert
*
* @project OpenGL project
*
**/
#ifndef TERRAIN_H
#define TERRAIN_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"
#include <iostream>

#include "./tess_shader.h"

const unsigned int NUM_PATCH_PTS = 4;

const float SIZE_X = 128.0f;
const float SIZE_Y = 10.0f;
const float SIZE_Z = 128.0f;

/**
* @brief Class that handle a 3D terrain object with a tesselation shader
**/
class Terrain{
public:
    TShader tessHeightMapShader = TShader(PATH_TO_SHADER "/terrain_generation/height.vs",PATH_TO_SHADER "/terrain_generation/height.fs",nullptr,PATH_TO_SHADER "/terrain_generation/height.tcs", PATH_TO_SHADER "/terrain_generation/height.tes");
    unsigned int terrainVAO, terrainVBO;
    std::vector<float> vertices;  
    int terrain_width = 0;
    int terrain_height = 0; 
    unsigned rez = 20;  
    btRigidBody* rigid;
    Object* terrain_obj;
    btCollisionShape* shape;
    short int *heightData;
    unsigned int texture;

    /** Constructor**/
    Terrain(){
        //Setup the 3D object
        terrain_obj = new Object();

        // Load the texture and the height values
        glGenTextures(1, &texture);
        glActiveTexture(GL_TEXTURE0+4);
        glBindTexture(GL_TEXTURE_2D, texture); 
        int width, height, nrChannels;
        unsigned char *data = stbi_load(PATH_TO_TEXTURE "/new_island.png", &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            std::cout << "Loaded heightmap of size " << height << " x " << width << std::endl;
            // Iterate through the image data and populate the heightfieldData array
            heightData = (short int *) malloc(height*width*2);
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    // Extract the height value from the red channel of the pixel at position (x, y)
                    heightData[y * width + x] = (short int) data[(y * width + x)];
                }
            }
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
                vertices.push_back(SIZE_Y/2); // v.y
                vertices.push_back(-height/2.0f + height*j/(float)rez); // v.z
                vertices.push_back(i / (float)rez); // u
                vertices.push_back(j / (float)rez); // v

                vertices.push_back(-width/2.0f + width*(i+1)/(float)rez); // v.x
                vertices.push_back(SIZE_Y/2); // v.y
                vertices.push_back(-height/2.0f + height*j/(float)rez); // v.z
                vertices.push_back((i+1) / (float)rez); // u
                vertices.push_back(j / (float)rez); // v

                vertices.push_back(-width/2.0f + width*i/(float)rez); // v.x
                vertices.push_back(SIZE_Y/2); // v.y
                vertices.push_back(-height/2.0f + height*(j+1)/(float)rez); // v.z
                vertices.push_back(i / (float)rez); // u
                vertices.push_back((j+1) / (float)rez); // v

                vertices.push_back(-width/2.0f + width*(i+1)/(float)rez); // v.x
                vertices.push_back(SIZE_Y/2); // v.y
                vertices.push_back(-height/2.0f + height*(j+1)/(float)rez); // v.z
                vertices.push_back((i+1) / (float)rez); // u
                vertices.push_back((j+1) / (float)rez); // v
            }
        }
        std::cout << "Loaded " << rez*rez << " patches of 4 control points each" << std::endl;
        std::cout << "Processing " << rez*rez*4 << " vertices in vertex shader" << std::endl;
        Load_buffer(); 
    }


    /** Bind your vertex arrays and call glDrawArrays and setup the MVP matrix **/
    void draw(Camera camera, glm::vec3 light_dir){
        tessHeightMapShader.use();
        glActiveTexture(GL_TEXTURE0+4);
        glBindTexture(GL_TEXTURE_2D, texture);
        tessHeightMapShader.setInteger("heightMap", 4);
        tessHeightMapShader.setMatrix4("projection",camera.GetProjectionMatrix());
        tessHeightMapShader.setMatrix4("view", camera.GetViewMatrix());
        tessHeightMapShader.setMatrix4("model", glm::mat4(1.0f));
        tessHeightMapShader.setFloat("dir_light.ambient", 0.2f);
        tessHeightMapShader.setFloat("dir_light.diffuse", 0.6f);
        tessHeightMapShader.setFloat("dir_light.specular", 0.3f);
        tessHeightMapShader.setVector3("dir_light.direction", light_dir);
        tessHeightMapShader.setVector3("u_view_pos", camera.Position);

        glBindVertexArray(terrainVAO);
        glDrawArrays(GL_PATCHES, 0, NUM_PATCH_PTS*rez*rez);
    }

    /** Destroy the buffers of the terrain **/
    void destroy(){
        glDeleteVertexArrays(1, &terrainVAO);
        glDeleteBuffers(1, &terrainVBO);
    }

private:
    /** Load the vertex information and link them to the buffer **/
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