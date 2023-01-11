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
    std::vector<short int> temp;
    unsigned int texture;

    Terrain(){
        terrain_obj = new Object();
        // Load the image of the mipmaps
        glGenTextures(1, &texture);
        glActiveTexture(GL_TEXTURE0+4);
        glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
        
         // load image, create texture and generate mipmaps
        int width, height, nrChannels;
        // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
        unsigned char *data = stbi_load(PATH_TO_TEXTURE "/flat_island.png", &width, &height, &nrChannels, 0);
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
                    temp.push_back(data[(y * width + x)]);
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

        shape = new btHeightfieldTerrainShape(terrain_height,
            terrain_width,
            heightData,
            64.0/255.0,
            0,
            64,
            1,
            PHY_SHORT,
            false
        );
        btVector3 aabbMin, aabbMax;
        shape->getAabb(btTransform::getIdentity(), aabbMin, aabbMax);
        std::cout << "Min AABB: " << "(" << aabbMin[0] << ", " << aabbMin[1] << ", " << aabbMin[2] << ")" << std::endl;
        std::cout << "Max AABB: " << "(" << aabbMax[0] << ", " << aabbMax[1] << ", " << aabbMax[2] << ")" << std::endl;

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

    void draw(Camera camera,float width,float height){
        tessHeightMapShader.use();
        glActiveTexture(GL_TEXTURE0+4);
        glBindTexture(GL_TEXTURE_2D, texture);
        tessHeightMapShader.setInteger("heightMap", 4);
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