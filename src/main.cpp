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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "./camera.h"
#include "./simple_shader.h"
#include "./tess_shader.h"
#include "./object.h"
#include "./terrain_generation.h"
#include "./skybox.h"
#include "./water.h"
#include "./utils/debug.h"
#include "./utils/callbacks.h"
#include "./utils/fps.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);

const int src_width = 700;
const int src_height = 700;
float lastX = src_width / 2.0f;
float lastY = src_height / 2.0f;
bool firstMouse = true;

Camera* camera = new Camera(glm::vec3(-2.0, 56.0, -5.0), glm::vec3(0.0, 0.5, 0.0), 90.0);
Callbacks callbacks  = Callbacks(camera);

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

	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);	// set texture wrapping to GL_REPEAT (default wrapping method)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_READ_COLOR_ARB);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//comment to use texture
	//glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	//comment to stop using the mouse to move
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	FPS fps = FPS(window);
#ifndef NDEBUG
	call_debug();
#endif

	Shader simple_shader(PATH_TO_SHADER "/simple.vs", PATH_TO_SHADER "/simple.fs");
	Shader simple_texture_shader(PATH_TO_SHADER "/texture/simple_texture.vs", PATH_TO_SHADER "/texture/simple_texture.fs");

	Terrain terrain = Terrain();
	Skybox skybox = Skybox();
	Water water = Water(1000,1.0, 1.0);	

	Object spirit = Object(PATH_TO_OBJECTS "/spirit.obj");
	spirit.makeObject(simple_texture_shader,true);
	spirit.model = glm::translate(spirit.model, glm::vec3(0.0,50.0,0.0));


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
	simple_texture_shader.use();
	simple_texture_shader.setInteger("ourTexture",1);
	simple_texture_shader.setFloat("shininess", 40.0f);
	simple_texture_shader.setFloat("light.ambient_strength", 1.0);
	simple_texture_shader.setFloat("light.diffuse_strength", 0.1);
	simple_texture_shader.setFloat("light.specular_strength", 0.1);
	simple_texture_shader.setFloat("light.constant", 1.4);
	simple_texture_shader.setFloat("light.linear", 0.74);
	simple_texture_shader.setFloat("light.quadratic", 0.27);

	float ambient = 0.2;
	float diffuse = 0.6;
	float specular = 1.0;

	glm::vec3 light_pos = glm::vec3(0.0, 52.0, 0.0);
	glm::vec3 materialColour = glm::vec3(0.17,0.68,0.89);	

	water.setup_water_shader(ambient,diffuse,specular);
	
	glfwSwapInterval(1);
	while (!glfwWindowShouldClose(window)) {
		callbacks.processInput(window);
		glfwPollEvents();
		double now = glfwGetTime();
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		terrain.draw(*camera,src_width,src_height);

		skybox.set();

		water.draw(*camera,materialColour,light_pos,now);
		skybox.draw(*camera);

		simple_texture_shader.use();
		simple_texture_shader.setInteger("ourTexture",1);
		simple_texture_shader.setMatrix4("M", spirit.model);
		simple_texture_shader.setMatrix4("itM", glm::inverseTranspose(spirit.model));
		simple_texture_shader.setMatrix4("V", camera->GetViewMatrix());
		simple_texture_shader.setMatrix4("P", camera->GetProjectionMatrix());
		spirit.draw();
		
		fps.display(now);
		glfwSwapBuffers(window);
	}
	terrain.destroy();

	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}


 void framebuffer_size_callback(GLFWwindow* window, int width, int height)
    {
        glViewport(0,0,width,width);
        camera->setRatio(width,height);
    }

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera->ProcessMouseScroll(static_cast<float>(yoffset));
}