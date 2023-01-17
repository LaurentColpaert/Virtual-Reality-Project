/**
* @brief This main file defines how to launch the project
*
* @author Adela Surca & Laurent Colpaert
*
* @project OpenGL project
*
**/
#include<iostream>
#include <map>
#include<algorithm>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/string_cast.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "./camera.h"
#include "./simple_shader.h"
#include "./tess_shader.h"
#include "./object.h"
#include "./terrain_generation.h"
#include "./skybox.h"
#include "./water.h"
#include "./ground.h"
#include "./spirit.h"
#include "./physic.h"
#include "./particles.h"
#include "./utils/debug.h"
#include "./utils/callbacks.h"
#include "./utils/fps.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window, Shader shader,Physic physic, Spirit spirit, ParticleGenerator* particle);
Object* create_launch_sphere(Shader shader, Physic physic, Spirit spirit);
void render_scene(Shader shader, Terrain terrain, Skybox skybox, Water water, Spirit spirit, Ground ground, glm::vec3 light_pos, Object sphere, double now,glm::mat4 lightspace,ParticleGenerator particle);
void render_depth_scene(Shader shader, Terrain terrain, Skybox skybox, Water water, Spirit spirit, Ground ground, glm::vec3 light_pos, Object sphere, double now);

int speed = 1;
float degree_rotation = 2.0;

int src_width = 700;
const int src_height = 700;
float lastX = src_width / 2.0f;
float lastY = src_height / 2.0f;
bool firstMouse = true;

std::vector<Object*> cubes;
std::vector<Object*> launched_spheres;
double now;
bool sphere_launched = false;

Camera* camera = new Camera(glm::vec3(0, 50.0, -25));

int main(int argc, char* argv[])
{
	//Create the OpenGL context 
	if (!glfwInit()) {
		throw std::runtime_error("Failed to initialise GLFW \n");
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifndef NDEBUG
	//create a debug context to help with Debugging
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
#endif

	GLFWwindow* window = glfwCreateWindow(src_width, src_height, "Project", nullptr, nullptr);
	if (window == NULL)
	{
		glfwTerminate();
		throw std::runtime_error("Failed to create GLFW window\n");
	}
	glfwMakeContextCurrent(window);
	
	//load openGL function
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) throw std::runtime_error("Failed to initialize GLAD");
	
	//glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetFramebufferSizeCallback(window,framebuffer_size_callback);
	glViewport(0,0,src_width,src_height);
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);  
	glPatchParameteri(GL_PATCH_VERTICES,4);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_READ_COLOR_ARB);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//comment to use texture
	// glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	//comment to stop using the mouse to move
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	FPS fps = FPS(window);
#ifndef NDEBUG
	call_debug();
#endif
	Physic physic = Physic();

	Shader simple_shader(PATH_TO_SHADER "/simple.vs", PATH_TO_SHADER "/simple.fs");
	Shader depth_shader(PATH_TO_SHADER "/depth/depth.vs", PATH_TO_SHADER "/depth/depth.fs");
	Shader debugDepthQuad(PATH_TO_SHADER "/depth/debug_depth.vs", PATH_TO_SHADER "/depth/debug_depth.fs");
	
	Terrain terrain = Terrain();
	Skybox skybox = Skybox();
	Water water = Water(1000,1.0, 37.0);	
	Ground ground = Ground();
	Spirit spirit = Spirit(glm::vec3(1,50,1));
	physic.setSpirit(&spirit);
	ParticleGenerator* particle = new ParticleGenerator(200,&spirit,camera);

	physic.addGround(&ground);
	physic.addSpirit(&spirit);

	Object sphere = Object(PATH_TO_OBJECTS "/sphere_smooth.obj");
	sphere.makeObject(simple_shader);
	sphere.transform.setTranslation(glm::vec3(0,50,0));
	sphere.transform.updateModelMatrix(sphere.transform.model);
	physic.addSphere(&sphere);

	Object plane_test = Object(PATH_TO_OBJECTS "/plane.obj");
	plane_test.makeObject(debugDepthQuad,true);
	plane_test.transform.setTranslation(glm::vec3(0.0,45.0,15.0));
	plane_test.transform.setScale(glm::vec3(5.0,0.0,5.0));
	plane_test.transform.setRotation(glm::vec3(glm::radians(90.0),0.0,0.0));
	plane_test.transform.updateModelMatrix(plane_test.transform.model);

	for(int i = 0; i < 5; i++){
		for(int j =0; j <5; j++ ){
			Object* cube = new Object(PATH_TO_OBJECTS "/sphere_smooth.obj");
			cube->makeObject(simple_shader);
			cube->transform.setTranslation(glm::vec3(i * 3 + j*1.5,45.0,i*2+j *3));
			cube->transform.updateModelMatrix(cube->transform.model);
			physic.addSphere(cube);
			cubes.push_back(cube);
		}
	}

	//Shadow  depth map
	const unsigned int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
    unsigned int depthMapFBO;
    glGenFramebuffers(1, &depthMapFBO);
    // create depth texture
    unsigned int depthMap;
    glGenTextures(1, &depthMap);
    glBindTexture(GL_TEXTURE_2D, depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    // attach depth texture as FBO's depth buffer
    glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glm::vec3 light_dir = glm::vec3(-30.0, 60.0, 30.0);
	glm::vec3 light_pos = glm::vec3(0.0, 43.0, 0.0);
	
	debugDepthQuad.use();
	debugDepthQuad.setInteger("depthMap", 6);
	
	simple_shader.use();
	simple_shader.setFloat("shininess", 40.0f);
	simple_shader.setFloat("light.ambient_strength", 0.2);
	simple_shader.setFloat("light.diffuse_strength", 0.7);
	simple_shader.setFloat("light.specular_strength", 0.9);
	simple_shader.setFloat("light.constant", 0.9);
	simple_shader.setFloat("light.linear", 0.7);
	simple_shader.setFloat("light.quadratic", 0.0);
	simple_shader.setVector3f("light.light_pos",light_pos);
	simple_shader.setVector3f("dir_light.direction",light_dir);
	simple_shader.setFloat("dir_light.ambient", 0.0f);
	simple_shader.setFloat("dir_light.diffuse", 0.6f);
	simple_shader.setFloat("dir_light.specular", 0.3f);
	simple_shader.setInteger("shadowMap", 6);

	float ambient = 0.2;
	float diffuse = 0.6;
	float specular = 1.0;

	glm::vec3 materialColour = glm::vec3(0.17,0.68,0.89);	

	water.setup_water_shader(ambient,diffuse,specular);
	spirit.setup_spirit_shader(ambient,diffuse,specular,light_pos);
	ground.setup_ground_shader(light_pos);
	
	glfwSwapInterval(1);
	while (!glfwWindowShouldClose(window)) {
		//Setup
		processInput(window, simple_shader, physic, spirit,particle);
		glfwPollEvents();
		double now = glfwGetTime();
		double deltaTime = fps.display(now);
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		auto delta = light_pos + glm::vec3(std::cos(now),0.0,2 * std::sin(now));

		//Update
		physic.update();
		particle->Update((float)deltaTime,0,spirit.getObject());

		//Depth pass
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float near_plane = -5.0f, far_plane = 50.0f;
        glm::mat4 P = glm::ortho(-50.0f, 50.0f, -50.0f, 50.0f, near_plane, far_plane);
        glm::mat4 V = glm::lookAt(light_dir, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 lightspace = P*V;

        // render scene from light's point of view
        depth_shader.use();
        depth_shader.setMatrix4("lightSpaceMatrix", lightspace);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		render_depth_scene(depth_shader, terrain, skybox, water, spirit, ground, delta, sphere,now);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Color pass
        glViewport(0, 0, src_width, src_width);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		simple_shader.use();
		simple_shader.setVector3f("light.light_pos",delta);
		simple_shader.setMatrix4("lightspace",lightspace);
		glActiveTexture(GL_TEXTURE0+6);
        glBindTexture(GL_TEXTURE_2D, depthMap);

		render_scene(simple_shader,terrain, skybox, water, spirit, ground, delta, sphere,now,lightspace,*particle);


		// render Depth map to quad for visual debugging
        debugDepthQuad.use();
        debugDepthQuad.setFloat("near_plane", near_plane);
        debugDepthQuad.setFloat("far_plane", far_plane);
		debugDepthQuad.setMatrix4("M", plane_test.transform.model);
		debugDepthQuad.setMatrix4("V", camera->GetViewMatrix());
		debugDepthQuad.setMatrix4("P", camera->GetProjectionMatrix());
		debugDepthQuad.setInteger("depthMap",6);
		plane_test.draw();

		//Draw the particle after the rest to be able to blend the color
		particle->draw();
		
		glfwSwapBuffers(window);

		//Delete the objects that falls bellow the water to avoid lagging
		cubes.erase(std::remove_if(cubes.begin(), cubes.end(), [](Object* obj){return obj->transform.is_below_level(37);}),cubes.end());
		launched_spheres.erase(std::remove_if(launched_spheres.begin(), launched_spheres.end(), [](Object* obj){return obj->transform.is_below_level(37);}),launched_spheres.end());
	}

	terrain.destroy();
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	src_width = width;
	glViewport(0,0,width,width);
	camera->setRatio(width,width);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos){
	if (firstMouse){
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera->ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset){
	camera->ProcessMouseScroll(static_cast<float>(yoffset));
}

Object* create_launch_sphere(Shader shader, Physic physic, Spirit spirit){
	Object* sphere = new  Object(PATH_TO_OBJECTS "/sphere_smooth.obj");
	sphere->makeObject(shader);
	glm::vec3 dir = spirit.getObject()->transform.get_forward();
	glm::vec3 position = spirit.getObject()->transform.getWorldTranslation() + dir * glm::vec3(1,0,1);
	sphere->transform.setTranslation(position);
	sphere->transform.updateModelMatrix(sphere->transform.model);
	physic.launch_sphere(sphere, 2000, spirit);
	launched_spheres.push_back(sphere);
	return sphere;
}

void render_scene(Shader shader,Terrain terrain, Skybox skybox, Water water, Spirit spirit, Ground ground, glm::vec3 light_pos, Object sphere, double now,glm::mat4 lightspace,ParticleGenerator particle){
	glm::vec3 materialColour = glm::vec3(0.17,0.68,0.89);	

	terrain.draw(*camera,src_width,src_height);
	skybox.draw(*camera);
	
	water.draw(*camera,materialColour,light_pos,now, skybox.getSkyTexture());

	ground.set_lightspace(lightspace);
	ground.draw(camera);
	spirit.draw(camera);

	shader.use();
	shader.setVector3f("u_view_pos",camera->Position);
	shader.setMatrix4("M", sphere.transform.model);
	shader.setMatrix4("itM", glm::inverseTranspose(sphere.transform.model));
	shader.setVector3f("materialColour", materialColour);
	shader.setMatrix4("V", camera->GetViewMatrix());
	shader.setMatrix4("P", camera->GetProjectionMatrix());
	sphere.draw();

	for(int i = 0; i < cubes.size(); i++){
		Object * obj = cubes[i];
		shader.setMatrix4("M", obj->transform.model);
		shader.setMatrix4("itM", glm::inverseTranspose(obj->transform.model));
		shader.setVector3f("materialColour", materialColour);
		shader.setMatrix4("V", camera->GetViewMatrix());
		shader.setMatrix4("P", camera->GetProjectionMatrix());
		obj->draw();
	}

	for(int i = 0; i < launched_spheres.size(); i++){
		Object * obj = launched_spheres[i];
		shader.setMatrix4("M", obj->transform.model);
		shader.setMatrix4("itM", glm::inverseTranspose(obj->transform.model));
		shader.setVector3f("materialColour", materialColour);
		shader.setMatrix4("V", camera->GetViewMatrix());
		shader.setMatrix4("P", camera->GetProjectionMatrix());
		obj->draw();
	}
}

void render_depth_scene(Shader shader, Terrain terrain, Skybox skybox, Water water, Spirit spirit, Ground ground, glm::vec3 light_pos, Object sphere, double now){
	glm::vec3 materialColour = glm::vec3(0.17,0.68,0.89);

	spirit.draw_depth(camera, shader);
	ground.draw_depth(camera, shader);

	shader.use();
	shader.setMatrix4("M", sphere.transform.model);
	sphere.draw();

	for(int i = 0; i < cubes.size(); i++){
		Object * obj = cubes[i];
		shader.setMatrix4("M", obj->transform.model);
		obj->draw();
	}

	for(int i = 0; i < launched_spheres.size(); i++){
		Object * obj = launched_spheres[i];
		shader.setMatrix4("M", obj->transform.model);
		obj->draw();
	}
}

/**Handle the input of the keyboard and launch the corresponding function**/
void processInput(GLFWwindow* window, Shader shader,Physic physic, Spirit spirit, ParticleGenerator* particle){
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)			camera->ProcessKeyboardMovement(LEFT, 0.1);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)		    camera->ProcessKeyboardMovement(RIGHT, 0.1);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)		    camera->ProcessKeyboardMovement(FORWARD, 0.1);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)		    camera->ProcessKeyboardMovement(BACKWARD, 0.1);
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)		camera->ProcessKeyboardRotation(1, 0.0, 1);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)		camera->ProcessKeyboardRotation(-1, 0.0, 1);
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)		    camera->ProcessKeyboardRotation(0.0, 1.0, 1);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)		camera->ProcessKeyboardRotation(0.0, -1.0, 1);
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS){ //forward
		glm::vec3 dir = spirit.getObject()->transform.get_forward();
		btVector3 bt_dir = btVector3(dir.x,dir.y,dir.z);
		btTransform xform = spirit.getRigidBody()->getWorldTransform();
		btVector3 cur = spirit.getRigidBody()->getLinearVelocity();
		btVector3 basis = xform.getBasis()[2];
		btVector3 vel = speed * 2 * bt_dir;
		spirit.getRigidBody()->activate(true);
		spirit.getRigidBody()->setLinearVelocity(btVector3(vel[0], cur[1], vel[2]));
	}
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS){ //Turn left
		btTransform transform = spirit.getRigidBody()->getWorldTransform();
		btQuaternion rotation = transform.getRotation();
		btQuaternion q; // Quaternion to rotate
		btScalar angle = glm::radians(degree_rotation); // Euler angles in radians
		q.setEuler(0, 0, angle); // Set the quaternion using Euler angles

		transform.setRotation(q * rotation);
		spirit.getRigidBody()->setWorldTransform(transform);
		spirit.getRigidBody()->getMotionState()->getWorldTransform(transform);
		btQuaternion orientation = transform.getRotation();
		glm::quat gl_orientation(orientation.w(), orientation.x(), orientation.y(), orientation.z());
		spirit.getObject()->transform.setRotation(glm::eulerAngles(gl_orientation));
		spirit.getObject()->transform.updateModelMatrix();
	}
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS){//backward
		glm::vec3 dir = spirit.getObject()->transform.get_forward();
		btVector3 bt_dir = btVector3(dir.x,dir.y,dir.z);
		btTransform xform = spirit.getRigidBody()->getWorldTransform();
		btVector3 cur = spirit.getRigidBody()->getLinearVelocity();
		btVector3 basis = xform.getBasis()[2];
		btVector3 vel = speed * 2 * bt_dir;
		spirit.getRigidBody()->activate(true);
		spirit.getRigidBody()->setLinearVelocity(btVector3(-vel[0], cur[1], -vel[2]));
	}
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS){ //turn right
		btTransform transform = spirit.getRigidBody()->getWorldTransform();
		btQuaternion rotation = transform.getRotation();
		btQuaternion q; // Quaternion to rotate
		btScalar angle = -glm::radians(degree_rotation); // Euler angles in radians
		q.setEuler(0, 0, angle); // Set the quaternion using Euler angles

		transform.setRotation(q * rotation);
		spirit.getRigidBody()->setWorldTransform(transform);
		spirit.getRigidBody()->getMotionState()->getWorldTransform(transform);
		btQuaternion orientation = transform.getRotation();
		glm::quat gl_orientation(orientation.w(), orientation.x(), orientation.y(), orientation.z());
		spirit.getObject()->transform.setRotation(glm::eulerAngles(gl_orientation));
		spirit.getObject()->transform.updateModelMatrix();
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){ 
		spirit.getRigidBody()->setLinearVelocity(btVector3(0.0,0.0,0.0));
		spirit.getRigidBody()->setAngularVelocity(btVector3(0.0,0.0,0.0));
	}
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS){
		if (!sphere_launched){
			Object* sphere = create_launch_sphere(shader,physic,spirit);
			particle->Update(0,50,sphere);
			now = glfwGetTime();
			sphere_launched = true;
		}else{
			if(glfwGetTime()- now > 1){
				Object* sphere = create_launch_sphere(shader,physic,spirit);
				particle->Update(0,50,spirit.getObject());
				now = glfwGetTime();
			}
		}
	}
}