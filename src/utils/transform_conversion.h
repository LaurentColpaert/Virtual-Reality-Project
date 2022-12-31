#ifndef TRANSFORM_CONV_H
#define TRANSFORM_CONV_H

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <btBulletDynamicsCommon.h>


class TransformConversion{
public:
    static glm::mat4 btScalar2glmMat4(btScalar* matrix) {
        return glm::mat4(
                matrix[0], matrix[1], matrix[2], matrix[3],
                matrix[4], matrix[5], matrix[6], matrix[7],
                matrix[8], matrix[9], matrix[10], matrix[11],
                matrix[12], matrix[13], matrix[14], matrix[15]);
    }

    static glm::vec3 btVector32glmVec3(btVector3 vec){
        return glm::vec3(vec[0], vec[1], vec[2]);
    }

    static btVector3 glmVec32btVector3(glm::vec3 vec){
        return btVector3(vec[0], vec[1], vec[2]);
    }

    static glm::quat btQuaternion2glmQuat(btQuaternion quaternion) {
        return glm::quat(quaternion.w(), quaternion.x(), quaternion.y(), quaternion.z());
    }
    
    static btQuaternion glmQuat2btQuaternion(glm::quat quaternion){
        return btQuaternion(quaternion.x, quaternion.y, quaternion.z, quaternion.w);
    }
};

#endif