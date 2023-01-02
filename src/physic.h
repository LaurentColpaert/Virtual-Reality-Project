#ifndef PHYSIC_H
#define PHYSIC_H

#include <btBulletDynamicsCommon.h>
#include "BulletCollision/CollisionShapes/btCollisionShape.h"
#include "BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h"

#include "object.h"
#include "utils/transform_conversion.h"

class Physic{
public:
    btDiscreteDynamicsWorld* dynamics_world;
    btDefaultCollisionConfiguration* collision_configuration;
    btCollisionDispatcher* dispatcher;
    btBroadphaseInterface* broadphase_interface;
    btSequentialImpulseConstraintSolver* solver;

    std::vector<Object*> objects;

    float size_x = 0.175;
    float size_y = 1.0;
    float size_z = 0.5;

    Physic(){
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
        
        dynamics_world->setGravity(btVector3(0, -1, 0));
    }
    
    
    
    void addSphere(Object *obj){
        btCollisionShape* shape = new btSphereShape(1.0f); // radius of 1.0
        printf("Translation is %f\n", obj->transform.translation.y);
        // Create a motion state for the sphere
        btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(obj->transform.translation.x,obj->transform.translation.y, obj->transform.translation.z))); 

        // Set the mass and inertia of the sphere
        btScalar mass = 1.0f;
        btVector3 inertia(0, 0, 0);
        shape->calculateLocalInertia(mass, inertia);

        // Create a rigid body for the sphere
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motionState, shape, inertia);
        btRigidBody* body = new btRigidBody(rbInfo);

        body->setLinearVelocity(btVector3(0,0,0));

        body->setUserPointer(obj);
        dynamics_world->addRigidBody(body);
        obj->rigid = body;
        objects.push_back(obj);
    }


    void addObject(Object *obj){
        btCollisionShape* shape = new btSphereShape(btScalar(0.0));

        btTransform transform;
        transform.setIdentity();

        btScalar mass(1.f);

        btVector3 localInertia(0, 0, 0);
        if (mass != 0) shape->calculateLocalInertia(mass, localInertia);

        transform.setOrigin(btVector3(obj->transform.translation.x, obj->transform.translation.y, obj->transform.translation.z));

        btDefaultMotionState* motion_state = new btDefaultMotionState(transform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motion_state, shape, btVector3(0,0,0));
        btRigidBody* body = new btRigidBody(rbInfo);

        body->setUserPointer(obj);
        dynamics_world->addRigidBody(body);
        obj->rigid = body;
        objects.push_back(obj);
    }

    void addTerrainToWorld(Terrain& terrain){
        // Create a triangle mesh shape for the terrain
        btCollisionShape* shape = terrain.shape;

        // Create a motion state for the terrain
        btDefaultMotionState* motionState = new btDefaultMotionState(btTransform(btQuaternion(0, 0, 0, 1), btVector3(0, 0, 0)));

        // Create a rigid body for the terrain
        btRigidBody::btRigidBodyConstructionInfo constructionInfo(0, motionState, shape, btVector3(0, 0, 0));
        btRigidBody* body = new btRigidBody(constructionInfo);

        // Add the rigid body to the dynamics world
        dynamics_world->addRigidBody(body);

        Object* obj = terrain.terrain_obj;
        body->setUserPointer(obj);
        dynamics_world->addRigidBody(body);
        obj->rigid = body;
        objects.push_back(obj);
    }

    void createGround(Object *obj){
        btCollisionShape* groundShape = new btBoxShape(btVector3(obj->transform.scale.x, 0, obj->transform.scale.z));

        btTransform transform;
        transform.setIdentity();
        transform.setOrigin(btVector3(0, obj->transform.translation.y, 0));

        btScalar mass(0);

        btDefaultMotionState* motion_state = new btDefaultMotionState(transform);
        btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, motion_state, groundShape, btVector3(0,0,0));
        btRigidBody* body = new btRigidBody(rbInfo);

        body->setUserPointer(obj);
        dynamics_world->addRigidBody(body);
        obj->rigid = body;
        objects.push_back(obj);
    }
    
    void update(){
        dynamics_world->stepSimulation(1.f / 60.f, 1);
        for (int i = 0; i < dynamics_world->getNumCollisionObjects(); i++)
        {
            // Get the collision object
            btCollisionObject* obj = dynamics_world->getCollisionObjectArray()[i];

            // Check if the collision object is a rigid body
            if (obj->getInternalType() == btCollisionObject::CO_RIGID_BODY)
            {
                btRigidBody* body = static_cast<btRigidBody*>(obj);

                // Get the current transform of the rigid body
                btTransform transform;
                body->getMotionState()->getWorldTransform(transform);

                // Extract the position, orientation, and scale of the object
                btVector3 position = transform.getOrigin();
                btQuaternion orientation = transform.getRotation();

                // Convert the position, orientation, and scale to glm types
                glm::vec3 gl_position(position.x(), position.y(), position.z());
                glm::quat gl_orientation(orientation.w(), orientation.x(), orientation.y(), orientation.z());

                // Retrieve the object associated with the rigid body
                Object* object = static_cast<Object*>(body->getUserPointer());

                object->transform.setTranslation(gl_position);
                object->transform.setRotation(glm::eulerAngles(gl_orientation));

                object->transform.updateModelMatrix();
            }
        }
    }
};
#endif