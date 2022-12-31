#ifndef PHYSIC_H
#define PHYSIC_H

#include <btBulletDynamicsCommon.h>
#include "object.h"
#include "utils/transform_conversion.h"

class Physic{
public:
    btDiscreteDynamicsWorld* dynamics_world;
    btDefaultCollisionConfiguration* collision_configuration;
    btCollisionDispatcher* dispatcher;
    btBroadphaseInterface* broadphase_interface;
    btSequentialImpulseConstraintSolver* solver;

    btAlignedObjectArray<btCollisionShape*> collision_array;
    std::vector<Object*> object_array;

    float size_x = 0.175;
    float size_y = 1.0;
    float size_z = 0.5;

    Physic(Object *obj){
        initializeEngine();
    }

    void initializeEngine(){
        ///collision configuration contains default setup for memory, collision setup. Advanced users can create their own configuration.
        collision_configuration = new btDefaultCollisionConfiguration();
        ///use the default collision dispatcher. For parallel processing you can use a diffent dispatcher (see Extras/BulletMultiThreaded)
        dispatcher = new btCollisionDispatcher(collision_configuration);
        ///btDbvtBroadphase is a good general purpose broadphase. You can also try out btAxis3Sweep.
        broadphase_interface = new btDbvtBroadphase();
        ///the default constraint solver. For parallel processing you can use a different solver (see Extras/BulletMultiThreaded)
        solver = new btSequentialImpulseConstraintSolver;
        dynamics_world = new btDiscreteDynamicsWorld(dispatcher, broadphase_interface, solver, collision_configuration);
        
        float gravity = -25.0f;
        dynamics_world->setGravity(btVector3(0, gravity, 0));
    }
    
    /*
    void createGround(Object *obj, float width=50., float depth=50.){
        btCollisionShape* groundShape = new btBoxShape(btVector3(btScalar(width), btScalar(0.), btScalar(depth)));

        collision_array.push_back(groundShape);
        object_array.push_back(obj);

        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(btVector3(0, 0, 0));

        btScalar mass(0.);

        btDefaultMotionState* motion_state = new btDefaultMotionState(transform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motion_state, groundShape, localInertia);
        btRigidBody* rigid = new btRigidBody(rbInfo);

        dynamics_world -> addRigidBody(rigid);
    }
    */

    void addObject(Object *obj){
        btCollisionShape* col_shape = new btSphereShape(btScalar(0.0));
        collision_array.push_back(col_shape);
        object_array.push_back(obj);

        btTransform transform;
        transform.setIdentity();

        btScalar mass(1.f);

        btVector3 localInertia(0, 0, 0);
        if (mass != 0) col_shape->calculateLocalInertia(mass, localInertia);

        transform.setOrigin(btVector3(obj->transform.translation.x, obj->transform.translation.y, obj->transform.translation.z));


        btDefaultMotionState* motion_state = new btDefaultMotionState(transform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motion_state, col_shape, btVector3(0,0,0));
        btRigidBody* rigid = new btRigidBody(rbInfo);

        dynamics_world->addRigidBody(rigid);
        obj->rigid = rigid;
    }

    void update(){
        dynamics_world->stepSimulation(1.f / 60.f, 1);

        // for (int j = dynamics_world->getNumCollisionObjects() - 1; j >= 0; j--){
        for( int i = 0; i < object_array.size();i++){
            Object* object = object_array[i];
            auto physicsBodyTransform = object->rigid->getWorldTransform();
            auto gameObjectTransform = object->transform;

            gameObjectTransform.translation = TransformConversion::btVector32glmVec3(physicsBodyTransform.getOrigin());
            std::cout << glm::to_string(gameObjectTransform.translation) << std::endl;
            gameObjectTransform.rotation = TransformConversion::btQuaternion2glmQuat(physicsBodyTransform.getRotation());


            btScalar transform[16];
            physicsBodyTransform.getOpenGLMatrix(transform);
            glm::mat4 translateRotateMtx = TransformConversion::btScalar2glmMat4(transform);
            glm::mat4 scaleMtx = glm::scale(glm::mat4(1), gameObjectTransform.scale);
            gameObjectTransform.model = translateRotateMtx * scaleMtx;
            
            //Update the objects
            object->transform.updateModelMatrix(object->transform.model);
            object->rigid->getWorldTransform().setRotation(TransformConversion::glmQuat2btQuaternion(object->transform.rotation));
            object->rigid->getWorldTransform().setOrigin(TransformConversion::glmVec32btVector3(object->transform.getWorldTranslation()));
            

            // btCollisionObject* obj = dynamics_world->getCollisionObjectArray()[j];
            // btRigidBody* rigid = btRigidBody::upcast(obj);
            // btTransform trans;
            // if (rigid && rigid->getMotionState()) rigid->getMotionState()->getWorldTransform(trans);
            // else trans = obj->getWorldTransform();
            

            // Object* glObj = object_array[j];
            // if (glObj != NULL) {
            //     btScalar roll, pitch, yaw;
            //     trans.getRotation().getEulerZYX(yaw,pitch,roll);
            //     glm::vec3 translation = glm::vec3(float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));
            //     glObj->setPosRot(translation, glm::vec3(roll, pitch, yaw));
            // }
        }
    }
    void destroy(){
        for (int i = dynamics_world->getNumCollisionObjects() - 1; i >= 0; i--)
        {
            btCollisionObject* obj = dynamics_world->getCollisionObjectArray()[i];
            btRigidBody* rigid = btRigidBody::upcast(obj);

            if (rigid && rigid->getMotionState())  delete rigid->getMotionState();
            
            dynamics_world->removeCollisionObject(obj);
            delete obj;
        }

        //delete collision shapes
        for (int j = 0; j < collision_array.size(); j++)
        {
            btCollisionShape* shape = collision_array[j];
            collision_array[j] = 0;
            delete shape;
        }

        delete this->dynamics_world;
        delete solver;
        delete broadphase_interface;
        delete dispatcher;
        delete collision_configuration;

        collision_array.clear();
        object_array.clear();
    }
};
#endif