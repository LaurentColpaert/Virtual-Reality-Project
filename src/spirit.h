#ifndef SPIRIT_H
#define SPIRIT_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "./simple_shader.h"
#include "./object.h"

class Spirit{
public:
    Object* spirit;
    Shader shader = Shader(PATH_TO_SHADER "/texture/simple_texture.vs", PATH_TO_SHADER "/texture/simple_texture.fs");
    btRigidBody* rigid_body;
    unsigned int spirit_texture;

    Spirit(glm::vec3 translation){
        spirit = new Object(PATH_TO_OBJECTS "/spirit.obj",true);
        spirit->makeObject(shader,true);
        spirit->transform.translation = translation;
        spirit->transform.updateModelMatrix();
        
        spirit_texture = loadTexture(PATH_TO_TEXTURE "/spirit_uv.jpg",5);
        // load_texture();
        set_rigid_body();
    }

    void setup_spirit_shader(float ambient, float diffuse, float specular, glm::vec3 light_pos){
        shader.use();
        shader.setInteger("my_texture",5);
        shader.setFloat("shininess", 40.0f);
        shader.setFloat("light.ambient_strength", 1.0);
        shader.setFloat("light.diffuse_strength", 0.1);
        shader.setFloat("light.specular_strength", 0.1);
        shader.setFloat("light.constant", 1.4);
        shader.setFloat("light.linear", 0.74);
        shader.setFloat("light.quadratic", 0.27);
        shader.setVector3f("light.light_pos",light_pos);
    }

    void draw(Camera* camera){
        glActiveTexture(GL_TEXTURE0+5);
        glBindTexture(GL_TEXTURE_2D, spirit_texture);
        shader.use();
        shader.setInteger("my_texture",5);
		shader.setVector3f("u_view_pos", camera->Position);
		shader.setMatrix4("M", spirit->transform.model);
		shader.setMatrix4("itM", glm::inverseTranspose(spirit->transform.model));
		shader.setMatrix4("V", camera->GetViewMatrix());
		shader.setMatrix4("P", camera->GetProjectionMatrix());
		spirit->draw();
    }  

    Object* getObject(){
        return spirit;
    }

    btRigidBody* getRigidBody(){
        return this->rigid_body;
    }

private:
    unsigned int loadTexture(char const * path, int texture_nb){
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glActiveTexture(GL_TEXTURE0+texture_nb);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);	// set texture wrapping to GL_REPEAT (default wrapping method)
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

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

            // glActiveTexture(GL_TEXTURE0+texture_nb);
            // glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Texture failed to load at path: " << path << std::endl;
            stbi_image_free(data);
        }
        glActiveTexture(GL_TEXTURE0+texture_nb);
        glBindTexture(GL_TEXTURE_2D, textureID);
        return textureID;
    }

    void set_rigid_body(){
        btCollisionShape* shape = new btBoxShape(btVector3(1,0.5,1));
        // Create a motion state for the cube
        btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(spirit->transform.translation.x,spirit->transform.translation.y, spirit->transform.translation.z))); 

        // Set the mass and inertia of the cube
        btScalar mass = 50.0f;
        btVector3 inertia(0, 0, 0);
        shape->calculateLocalInertia(mass, inertia);

        // Create a rigid body for the cube
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, inertia);
        rigid_body = new btRigidBody(rbInfo);
        rigid_body->setAngularFactor(btVector3(0,0,1));
        rigid_body->setFriction(0.0f);
    }

};
#endif