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

    Spirit(glm::vec3 translation){
        spirit = new Object(PATH_TO_OBJECTS "/spirit.obj",true);
        spirit->makeObject(shader,true);
        spirit->transform.translation = translation;
        spirit->transform.updateModelMatrix();
        
        load_texture();
        set_rigid_body();
    }

    void setup_spirit_shader(float ambient, float diffuse, float specular, glm::vec3 light_pos){
        shader.use();
        shader.setInteger("my_texture",1);
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
        shader.use();
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
    void load_texture(){
        GLuint spirit_texture;
        glGenTextures(1, &spirit_texture);
        glActiveTexture(GL_TEXTURE0+1);
        glBindTexture(GL_TEXTURE_2D, spirit_texture);

        stbi_set_flip_vertically_on_load(true);
        int imWidth, imHeight, imNrChannels;
        unsigned char* data = stbi_load(PATH_TO_TEXTURE "/spirit_uv.jpg", &imWidth, &imHeight, &imNrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imWidth, imHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);
        }
        else {
            std::cout << "Failed to Load texture" << std::endl;
            const char* reason = stbi_failure_reason();
            std::cout << reason << std::endl;
        }

        stbi_image_free(data);
        glActiveTexture(GL_TEXTURE0+1);
        glBindTexture(GL_TEXTURE_2D, spirit_texture);
    }

    void set_rigid_body(){
        btCollisionShape* shape = new btBoxShape(btVector3(spirit->transform.scale.x,spirit->transform.scale.y,spirit->transform.scale.z));
        // Create a motion state for the cube
        btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(spirit->transform.translation.x,spirit->transform.translation.y+3, spirit->transform.translation.z))); 
        std::cout<<"The cube position is :" + glm::to_string(spirit->transform.translation)<<std::endl;

        // Set the mass and inertia of the cube
        btScalar mass = 50.0f;
        btVector3 inertia(0, 0, 0);
        shape->calculateLocalInertia(mass, inertia);

        // Create a rigid body for the cube
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, inertia);
        rigid_body = new btRigidBody(rbInfo);
        rigid_body->setAngularFactor(btVector3(0,0,1));
    }

};
#endif