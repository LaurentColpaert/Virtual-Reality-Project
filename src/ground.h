#ifndef GROUND_H
#define GROUND_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "./simple_shader.h"
#include "./object.h"

class Ground{
public:
    Object* ground;
	Shader shader = Shader(PATH_TO_SHADER "/bump/bump.vs", PATH_TO_SHADER "/bump/bump.fs");
    unsigned int diffuseMap;
    unsigned int normalMap;
    
    btRigidBody* rigid_body;

    Ground(){
        ground = new Object();
        ground->numVertices = 6;    
        ground->makeGround(shader);
        ground->transform.setTranslation(glm::vec3(0.0,40.0,0.0));
        ground->transform.setScale(glm::vec3(100.0,1.0,100.0));
        ground->transform.updateModelMatrix();

        diffuseMap = loadTexture(PATH_TO_TEXTURE "/brickwall.jpg",2);
        normalMap  = loadTexture(PATH_TO_TEXTURE "/brickwall_normal.jpg",3);
        
        ground->setName("ground");
                
        set_rigid_body();
    }

    void setup_ground_shader(glm::vec3 light_pos){
        shader.use();
        shader.setInteger("shadowMap",6);
        shader.setInteger("diffuseMap", 2);
        shader.setInteger("normalMap", 3);
        // shader.setFloat("shininess", 40.0f);
        // shader.setFloat("light.ambient_strength", 1.0);
        // shader.setFloat("light.diffuse_strength", 0.1);
        // shader.setFloat("light.specular_strength", 0.1);
        // shader.setFloat("light.constant", 1.4);
        // shader.setFloat("light.linear", 0.74);
        // shader.setFloat("light.quadratic", 0.27);
        // shader.setVector3f("light.light_pos",light_pos);
        shader.setVector3f("lightPos",light_pos);
    }

    void set_lightspace(glm::mat4 lightspace){
        shader.use();
        shader.setMatrix4("lightspace",lightspace);
    }



    void draw(Camera* camera){
        shader.use();
        // glActiveTexture(GL_TEXTURE0+2);
        // glBindTexture(GL_TEXTURE_2D, diffuseMap);
        // shader.setInteger("diffuseMap", 2);
        // glActiveTexture(GL_TEXTURE0+3);
        // glBindTexture(GL_TEXTURE_2D, normalMap);
        // shader.setInteger("normalMap", 3);
		shader.setVector3f("viewPos", camera->Position);
		shader.setMatrix4("M", ground->transform.model);
		shader.setMatrix4("V", camera->GetViewMatrix());
		shader.setMatrix4("P", camera->GetProjectionMatrix());
		ground->draw();
    }  

    void draw_depth(Camera* camera,Shader shader){
        shader.use();
		shader.setMatrix4("M", ground->transform.model);
		ground->draw();
    }  

    Object* getObject(){
        return ground;
    }

    btRigidBody* getRigidBody(){
        return this->rigid_body;
    }

    Shader getShader(){
        return shader;
    }

private:
    unsigned int loadTexture(char const * path, int texture_nb){
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glActiveTexture(GL_TEXTURE0+texture_nb);

        int width, height, nrComponents;
        unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glActiveTexture(GL_TEXTURE0+texture_nb);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            std::cout << "Texture failed to load at path: " << path << std::endl;
            stbi_image_free(data);
        }
        return textureID;
    }

    void set_rigid_body(){
        btCollisionShape* groundShape = new btBoxShape(btVector3(ground->transform.scale.x, 1.0, ground->transform.scale.z));

        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(btVector3(0, ground->transform.translation.y, 0));

        btScalar mass(0);

        btDefaultMotionState* motion_state = new btDefaultMotionState(transform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motion_state, groundShape, btVector3(0,0,0));
        rigid_body = new btRigidBody(rbInfo);
        rigid_body->setCollisionFlags(rigid_body->getCollisionFlags() | btCollisionObject::CF_STATIC_OBJECT);
        rigid_body->setFriction(0.0f);
    }
};
#endif