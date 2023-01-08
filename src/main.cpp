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
#include "./utils/debug.h"
#include "./utils/callbacks.h"
#include "./utils/fps.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window, Shader shader,Physic physic, Spirit spirit);
void create_launch_sphere(Shader shader, Physic physic, Spirit spirit);
void renderScene(bool depth,Shader shader, Terrain terrain, Skybox skybox, Water water, Spirit spirit, Ground ground, glm::vec3 light_pos, Object sphere, double now);
void renderQuad();

int speed = 1;
float degree_rotation = 2.0;

int src_width = 700;
const int src_height = 700;
float lastX = src_width / 2.0f;
float lastY = src_height / 2.0f;
bool firstMouse = true;

std::vector<Object*> cubes;
int nb_cubes = 0;

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
	glPatchParameteri(GL_PATCH_VERTICES,4);

	//comment to use texture
	// glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	//comment to stop using the mouse to move
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	FPS fps = FPS(window);
#ifndef NDEBUG
	call_debug();
#endif

	Shader simple_shader(PATH_TO_SHADER "/simple.vs", PATH_TO_SHADER "/simple.fs");
	Shader depth_shader(PATH_TO_SHADER "/depth/depth.vs", PATH_TO_SHADER "/depth/depth.fs");
	Shader debugDepthQuad(PATH_TO_SHADER "/depth/debug_depth.vs", PATH_TO_SHADER "/depth/debug_depth.fs");

	Terrain terrain = Terrain();
	// physic.addTerrainToWorld(terrain);
	Skybox skybox = Skybox();
	Water water = Water(1000,1.0, 37.0);	
	Physic physic = Physic();

	Ground ground = Ground();
	physic.addGround(&ground);

	Spirit spirit = Spirit(glm::vec3(1,50,1));
	// spirit.getObject()->transform.setRotation(glm::vec3(cos(90),0,0));
	physic.addSpirit(&spirit);

	Object sphere = Object(PATH_TO_OBJECTS "/sphere_smooth.obj");
	sphere.makeObject(simple_shader);
	sphere.transform.setTranslation(glm::vec3(0,50,0));
	sphere.transform.updateModelMatrix(sphere.transform.model);
	physic.addSphere(&sphere);

	Object plane_test = Object(PATH_TO_OBJECTS "/plane.obj");
	plane_test.makeObject(debugDepthQuad);
	plane_test.transform.setTranslation(glm::vec3(-2.0,43.0,0.0));
	plane_test.transform.updateModelMatrix(plane_test.transform.model);

	for(int i = 0; i < 5; i++){
		for(int j =0; j <5; j++ ){
			Object* cube = new Object(PATH_TO_OBJECTS "/sphere_smooth.obj");
			cube->makeObject(simple_shader);
			cube->transform.setTranslation(glm::vec3(i * 3 + j*1.5,42.0+j *3,i*2));
			cube->transform.updateModelMatrix(cube->transform.model);
			physic.addSphere(cube);
			cubes.push_back(cube);
			nb_cubes++;
		}
	}

	//Shadow  depth map
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
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

	debugDepthQuad.use();
	debugDepthQuad.setInteger("depthMap", 6);

	glm::vec3 light_pos = glm::vec3(0.0, 46.0, -1.0);
	
	simple_shader.use();
	simple_shader.setFloat("shininess", 40.0f);
	simple_shader.setFloat("light.ambient_strength", 0.2);
	simple_shader.setFloat("light.diffuse_strength", 0.7);
	simple_shader.setFloat("light.specular_strength", 0.9);
	simple_shader.setFloat("light.constant", 0.9);
	simple_shader.setFloat("light.linear", 0.7);
	simple_shader.setFloat("light.quadratic", 0.0);
	simple_shader.setVector3f("light.light_pos",light_pos);
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
		processInput(window, simple_shader, physic, spirit);
		glfwPollEvents();
		double now = glfwGetTime();
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		// physic.update();

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		
        float near_plane = 1.0f, far_plane = 7.5f;
        glm::mat4 P = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
        glm::mat4 V = glm::lookAt(light_pos, glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
		glm::mat4 lightspace = P*V;
        // render scene from light's point of view
        depth_shader.use();
        depth_shader.setMatrix4("P", P);
        depth_shader.setMatrix4("V", V);

        glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
        glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
		glClear(GL_DEPTH_BUFFER_BIT);
		renderScene(true,depth_shader, terrain, skybox, water, spirit, ground, light_pos, sphere,now);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // reset viewport
        glViewport(0, 0, src_width, src_width);
		
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		

		simple_shader.use();
		simple_shader.setMatrix4("lightspace",lightspace);
		glActiveTexture(GL_TEXTURE0+6);
        glBindTexture(GL_TEXTURE_2D, depthMap);

		renderScene(false,simple_shader, terrain, skybox, water, spirit, ground, light_pos, sphere,now);


		// render Depth map to quad for visual debugging
        // ---------------------------------------------
        debugDepthQuad.use();
        debugDepthQuad.setFloat("near_plane", near_plane);
        debugDepthQuad.setFloat("far_plane", far_plane);
        // renderQuad();
		debugDepthQuad.setMatrix4("M", plane_test.transform.model);
		debugDepthQuad.setMatrix4("itM", glm::inverseTranspose(plane_test.transform.model));
		debugDepthQuad.setMatrix4("V", camera->GetViewMatrix());
		debugDepthQuad.setMatrix4("P", camera->GetProjectionMatrix());
		plane_test.draw();

		fps.display(now);
		glfwSwapBuffers(window);
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

/**Handle the input of the keyboard and launch the corresponding function**/
void processInput(GLFWwindow* window, Shader shader,Physic physic, Spirit spirit){
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

		spirit.getRigidBody()->setLinearVelocity(btVector3(vel[0], cur[1], vel[2]));

		// spirit.getRigidBody()->applyCentralForce(btVector3(0,0,1) * 500);
	}
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS){ //Turn left
		btTransform transform = spirit.getRigidBody()->getWorldTransform();
		btQuaternion rotation = transform.getRotation();
		btQuaternion q; // Quaternion to rotate
		btScalar angle = glm::radians(degree_rotation); // Euler angles in radians
		q.setEuler(0, 0, angle); // Set the quaternion using Euler angles

		transform.setRotation(q * rotation);
		spirit.getRigidBody()->setWorldTransform(transform);
	}
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS){//backward
		btTransform xform = spirit.getRigidBody()->getWorldTransform();
		btVector3 cur = spirit.getRigidBody()->getLinearVelocity();
		btVector3 basis = xform.getBasis()[2];
		btVector3 vel = -speed * 2 * basis;
		spirit.getRigidBody()->setLinearVelocity(btVector3(vel[0], cur[1], vel[2]));
	}
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS){ //turn right
		btTransform transform = spirit.getRigidBody()->getWorldTransform();
		btQuaternion rotation = transform.getRotation();
		btQuaternion q; // Quaternion to rotate
		btScalar angle = -glm::radians(degree_rotation); // Euler angles in radians
		q.setEuler(0, 0, angle); // Set the quaternion using Euler angles

		transform.setRotation(q * rotation);
		spirit.getRigidBody()->setWorldTransform(transform);
	}
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS){
		if (!sphere_launched){
			create_launch_sphere(shader,physic,spirit);
			now = glfwGetTime();
			sphere_launched = true;
		}else{
			if(glfwGetTime()- now > 1){
				create_launch_sphere(shader,physic,spirit);
				now = glfwGetTime();
			}
		}
	}
	else{
		// btVector3 temp_velocity = spirit.getRigidBody()->getLinearVelocity();
		// spirit.getRigidBody()->setLinearVelocity(btVector3(0,temp_velocity[1],0));
	}
}

void create_launch_sphere(Shader shader, Physic physic, Spirit spirit){
	Object* sphere = new  Object(PATH_TO_OBJECTS "/sphere_smooth.obj");
	sphere->makeObject(shader);
	glm::vec3 dir = spirit.getObject()->transform.get_forward();
	glm::vec3 position = spirit.getObject()->transform.getWorldTranslation() + dir * glm::vec3(1,0,1);
	sphere->transform.setTranslation(position);
	sphere->transform.updateModelMatrix(sphere->transform.model);
	physic.launch_sphere(sphere, 2000, spirit);
	launched_spheres.push_back(sphere);
}

void renderScene(bool depth,Shader shader, Terrain terrain, Skybox skybox, Water water, Spirit spirit, Ground ground, glm::vec3 light_pos, Object sphere, double now){
	glm::vec3 materialColour = glm::vec3(0.17,0.68,0.89);	

	terrain.draw(*camera,src_width,src_height);
	skybox.set();
	water.draw(*camera,materialColour,light_pos,now);
	skybox.draw(*camera);
	spirit.draw(camera);
	if (depth)	ground.draw(camera,shader);
	else ground.draw(camera);

	// simple_shader.use();
	// simple_shader.setVector3f("u_view_pos", camera->Position);
	// simple_shader.setMatrix4("M", plane_test.transform.model);
	// simple_shader.setMatrix4("itM", glm::inverseTranspose(plane_test.transform.model));
	// simple_shader.setVector3f("materialColour", glm::vec3(0.56,0.24,0.12));
	// simple_shader.setMatrix4("V", camera->GetViewMatrix());
	// simple_shader.setMatrix4("P", camera->GetProjectionMatrix());
	// plane_test.draw();

	shader.use();
	shader.setMatrix4("M", sphere.transform.model);
	shader.setMatrix4("itM", glm::inverseTranspose(sphere.transform.model));
	shader.setVector3f("materialColour", materialColour);
	shader.setMatrix4("V", camera->GetViewMatrix());
	shader.setMatrix4("P", camera->GetProjectionMatrix());
	sphere.draw();

	for(int i = 0; i < nb_cubes; i++){
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

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  50.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, 48.0f, 0.0f, 0.0f, 0.0f,
             1.0f,  50.0f, 0.0f, 1.0f, 1.0f,
             1.0f, 48.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}