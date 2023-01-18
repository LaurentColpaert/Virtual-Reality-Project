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
        
        spirit_texture = loadTexture(PATH_TO_TEXTURE "/spirit_uv.jpg",1);
        // load_texture();
        spirit->setName("spirit");
        set_rigid_body();
    }

    void setup_spirit_shader(float ambient, float diffuse, float specular, glm::vec3 light_pos, glm::vec3 light_dir){
        shader.use();
        glActiveTexture(GL_TEXTURE0+1);
        glBindTexture(GL_TEXTURE_2D, spirit_texture);
        shader.setInteger("my_texture",1);
        shader.setFloat("light.ambient_strength", 0.9);
        shader.setFloat("light.diffuse_strength", 0.7);
        shader.setFloat("light.specular_strength", 0.9);
        shader.setFloat("light.constant", 0.9);
        shader.setFloat("light.linear", 0.7);
        shader.setFloat("light.quadratic", 0.0);
        shader.setVector3f("light.light_pos",light_pos);
        shader.setVector3f("dir_light.direction",light_dir);
        shader.setFloat("dir_light.ambient", 0.0f);
        shader.setFloat("dir_light.diffuse", 0.6f);
        shader.setFloat("dir_light.specular", 0.3f);
    }

    void draw(Camera* camera, glm::vec3 light_pos){
        shader.use();
        glActiveTexture(GL_TEXTURE0+1);
        glBindTexture(GL_TEXTURE_2D, spirit_texture);
        shader.setInteger("my_texture",1);
        shader.setVector3f("light.light_pos",light_pos);
		shader.setVector3f("u_view_pos", camera->Position);
		shader.setMatrix4("M", spirit->transform.model);
		shader.setMatrix4("itM", glm::inverseTranspose(spirit->transform.model));
		shader.setMatrix4("V", camera->GetViewMatrix());
		shader.setMatrix4("P", camera->GetProjectionMatrix());
		spirit->draw();
    }  

    void draw_depth(Camera* camera, Shader shader){
        shader.use();
		shader.setMatrix4("M", spirit->transform.model);
		spirit->draw();
    }  
    Object* getObject(){
        return spirit;
    }

    btRigidBody* getRigidBody(){
        return this->rigid_body;
    }

    unsigned int loadTexture(char const * path, int texture_nb){
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glActiveTexture(GL_TEXTURE0+texture_nb);
        glBindTexture(GL_TEXTURE_2D, textureID);
        stbi_set_flip_vertically_on_load(true);
        int imWidth, imHeight, imNrChannels;
        unsigned char* data = stbi_load(path, &imWidth, &imHeight, &imNrChannels, 0);
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
        glActiveTexture(GL_TEXTURE0+texture_nb);
        glBindTexture(GL_TEXTURE_2D, textureID);
        return textureID;
    }
private:

    void set_rigid_body(){
        btCollisionShape* shape = new btSphereShape(1);
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
        // rigid_body->setFriction(0.0f);
        rigid_body->setRollingFriction(0.0);
        rigid_body->setSpinningFriction(0.0);
    }
};
#endif