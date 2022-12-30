#ifndef PHYSIC_H
#define PHYSIC_H

#include <btBulletDynamicsCommon.h>
#include "object.h"

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
        createGround(obj, 100., 100.);
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
        dynamics_world->setGravity(btVector3(0, GRAVITY, 0));
    }
    
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

    void addObject(Object *obj){
        btCollisionShape* col_shape = new btSphereShape(btScalar(obj->scale.x));
        collision_array.push_back(col_shape);
        object_array.push_back(obj);

        btTransform transform;
        transform.setIdentity();

        btScalar mass(1.f);

        btVector3 localInertia(0, 0, 0);
        if (mass != 0) col_shape->calculateLocalInertia(mass, localInertia);

        transform.setOrigin(btVector3(obj->initial_position.x, obj->initial_position.y, obj->initial_position.z));
        
        // TODO : add a rotation into the objet class
        float roll = 0; 
        float pitch = obj->0; 
        float yaw = obj->0; 
        float qx = sin(roll/2) * cos(pitch/2) * cos(yaw/2) - cos(roll/2) * sin(pitch/2) * sin(yaw/2);
        float qy = cos(roll/2) * sin(pitch/2) * cos(yaw/2) + sin(roll/2) * cos(pitch/2) * sin(yaw/2);
        float qz = cos(roll/2) * cos(pitch/2) * sin(yaw/2) - sin(roll/2) * sin(pitch/2) * cos(yaw/2);
        float qw = cos(roll/2) * cos(pitch/2) * cos(yaw/2) + sin(roll/2) * sin(pitch/2) * sin(yaw/2);
        transform.setRotation(btQuaternion(qx,qy,qz,qw)); 

        btDefaultMotionState* motion_state = new btDefaultMotionState(transform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motion_state, col_shape, localInertia);
        btRigidBody* rigid = new btRigidBody(rbInfo);

        dynamics_world->addRigidBody(rigid);
    }

    void animate(){
        dynamics_world->stepSimulation(1.f / 60.f, 1);

        for (int j = dynamics_world->getNumCollisionObjects() - 1; j >= 0; j--){
            btCollisionObject* obj = dynamics_world->getCollisionObjectArray()[j];
            btRigidBody* rigid = btRigidBody::upcast(obj);
            btTransform trans;
            if (rigid && rigid->getMotionState()) rigid->getMotionState()->getWorldTransform(trans);
            else trans = obj->getWorldTransform();
            

            Object* glObj = object_array[j];
            if (glObj != NULL) {
                btScalar roll, pitch, yaw;
                trans.getRotation().getEulerZYX(yaw,pitch,roll);
                glm::vec3 translation = glm::vec3(float(trans.getOrigin().getX()), float(trans.getOrigin().getY()), float(trans.getOrigin().getZ()));
                glObj->setPosRot(translation, glm::vec3(roll, pitch, yaw));
            }
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