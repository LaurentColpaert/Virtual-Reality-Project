/**
* @brief This header file defines the Transform class. Based on the `jeffeverett/opengl-driving-scene` class
*
* @author Adela Surca & Laurent Colpaert
*
* @project OpenGL project
*
**/
#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>

//We enable experimental because it allows us to use the quaternions
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>

#include <iostream>

class Transform{
public:
    glm::vec3 scale;
    glm::vec3 translation;
    glm::quat rotation;

    glm::mat4 model;

    Transform(){
        scale = glm::vec3(1);
        translation = glm::vec3(0);
        rotation = glm::vec3(0);
        model = glm::mat4(1);
    }

    void setScale(const glm::vec3 &scale_vector){
        scale = scale_vector;
    }
    
    void setTranslation(const glm::vec3 &translation_vector){
        translation = translation_vector;
    }

    void setRotation(const glm::quat &quaternion){
        rotation = quaternion;
    }

    void setRotation(const glm::vec3 &euler_angles){
        rotation = glm::toQuat(glm::orientate3(euler_angles));
    }

    glm::vec3 getWorldTranslation(){
        return glm::vec3(model[3][0], model[3][1], model[3][2]);
    }

    void updateModelMatrix(){
        glm::mat4 mtx(1);
        mtx = glm::scale(mtx, scale);
        glm::mat4 rotation_matrix = glm::toMat4(rotation);
        mtx = rotation_matrix * mtx;
        mtx[3][0] += translation.x;
        mtx[3][1] += translation.y;
        mtx[3][2] += translation.z;
        model = mtx;
    }
    void updateModelMatrix(glm::mat4 starting_matrix){
        glm::mat4 mtx(1);
        mtx = glm::scale(mtx, scale);
        glm::mat4 rotation_matrix = glm::toMat4(rotation);
        mtx = rotation_matrix * mtx;
        mtx[3][0] += translation.x;
        mtx[3][1] += translation.y;
        mtx[3][2] += translation.z;
        model = starting_matrix * mtx;
    }

    glm::vec3 get_forward(){
        glm::mat3 rotMat(model);;
        glm::vec3 forward = rotMat * glm::vec3(0, 0, 1);
        return glm::normalize(forward);
    }

    bool is_below_level(float level){
        if (model[3][1] < level) return true;
        return false;
    }
};
#endif