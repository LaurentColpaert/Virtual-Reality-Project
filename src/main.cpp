#include<iostream>

//include glad before GLFW to avoid header conflict or define "#define GLFW_INCLUDE_NONE"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


#include "./camera.h"
#include "./shader.h"
#include "./object.h"

const int width = 700;
const int height = 700;


GLuint compileShader(std::string shaderCode, GLenum shaderType);
GLuint compileProgram(GLuint vertexShader, GLuint fragmentShader);
void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);


#ifndef NDEBUG
void APIENTRY glDebugOutput(GLenum source,
	GLenum type,
	unsigned int id,
	GLenum severity,
	GLsizei length,
	const char* message,
	const void* userParam)
{
	// ignore non-significant error/warning codes
	if (id == 131169 || id == 131185 || id == 131218 || id == 131204) return;

	std::cout << "---------------" << std::endl;
	std::cout << "Debug message (" << id << "): " << message << std::endl;

	switch (source)
	{
	case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
	case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
	case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
	case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
	case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
	case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
	} std::cout << std::endl;

	switch (type)
	{
	case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
	case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
	case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break;
	case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
	case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
	case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
	case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
	case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
	case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
	} std::cout << std::endl;

	switch (severity)
	{
	case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
	case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
	case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
	case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
	} std::cout << std::endl;
	std::cout << std::endl;
}
#endif

Camera camera(glm::vec3(1.0, 2.0, -6.0), glm::vec3(0.0, 3.0, -1.0), 90.0);

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

	//Create the window
	GLFWwindow* window = glfwCreateWindow(width, height, "Project", nullptr, nullptr);
	if (window == NULL)
	{
		glfwTerminate();
		throw std::runtime_error("Failed to create GLFW window\n");
	}

	glfwMakeContextCurrent(window);
	
	//load openGL function
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		throw std::runtime_error("Failed to initialize GLAD");
	}

	glViewport(0,0,width,height);
	glfwSetFramebufferSizeCallback(window,framebuffer_size_callback);
	glEnable(GL_DEPTH_TEST);


#ifndef NDEBUG
	int flags;
	glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
	if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(glDebugOutput, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}
#endif
	
	char vertexPath[] = PATH_TO_SHADER "/vertSrc.vs";
	char fragPath[] = PATH_TO_SHADER "/fragSrc.fs";
	Shader shader(vertexPath, fragPath);

	char path[] = PATH_TO_OBJECTS "/sphere_smooth.obj";
	char plane_path[] = PATH_TO_OBJECTS "/plane.obj";
	char tree_path[] = PATH_TO_OBJECTS "/lowpoly_tree.obj";

	Object sphere(path);
	sphere.makeObject(shader);
	sphere.model = glm::translate(sphere.model,glm::vec3(0.0,2.0,0.0));

	Object plane(plane_path);
	plane.makeObject(shader);
	plane.model = glm::translate(plane.model,glm::vec3(0.0,0.0,0.0));
	plane.model = glm::scale(plane.model, glm::vec3(4.,4.,4.));


	Object tree(tree_path);
	tree.makeObject(shader);
	tree.model = glm::translate(tree.model,glm::vec3(1.0,0.0,-1.0));
	tree.model = glm::scale(tree.model, glm::vec3(0.2,0.2,0.2));

	double prev = 0;
	int deltaFrame = 0;
	//fps function
	auto fps = [&](double now) {
		double deltaTime = now - prev;
		deltaFrame++;
		if (deltaTime > 0.5) {
			prev = now;
			const double fpsCount = (double)deltaFrame / deltaTime;
			deltaFrame = 0;
			std::cout << "\r FPS: " << fpsCount;
			std::cout.flush();
		}
	};

	glm::vec3 light_pos = glm::vec3(1.0, 2.0, 1.5);
	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 perspective = camera.GetProjectionMatrix();

	float ambient = 0.4;
	float diffuse = 0.5;
	float specular = 0.8;

	glm::vec3 materialColour = glm::vec3(0.2,0.6,0.8);

	//Rendering
	shader.use();
	shader.setFloat("shininess", 32.0f);
	shader.setVector3f("materialColour", materialColour);
	shader.setFloat("light.ambient_strength", ambient);
	shader.setFloat("light.diffuse_strength", diffuse);
	shader.setFloat("light.specular_strength", specular);
	shader.setFloat("light.constant", 1.0);
	shader.setFloat("light.linear", 0.14);
	shader.setFloat("light.quadratic", 0.07);
	
	while (!glfwWindowShouldClose(window)) {
		processInput(window);
		view = camera.GetViewMatrix();
		glfwPollEvents();
		double now = glfwGetTime();
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shader.use();
		shader.setMatrix4("M", sphere.model);
		shader.setMatrix4("itM", glm::inverseTranspose(sphere.model));
		sphere.draw();
		shader.setMatrix4("M", plane.model);
		shader.setMatrix4("itM", glm::inverseTranspose(plane.model));
		plane.draw();
		//shader.setMatrix4("itM", glm::inverseTranspose(tree.model));
		//tree.draw();
		shader.setMatrix4("V", view);
		shader.setMatrix4("P", perspective);
		shader.setVector3f("u_view_pos", camera.Position);

		auto delta = light_pos + glm::vec3(0.0,0.0,2 * std::sin(now));
		shader.setVector3f("light.light_pos", delta);
		
		fps(now);
		glfwSwapBuffers(window);
	}
	//clean up ressource
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}


void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)		    camera.ProcessKeyboardMovement(LEFT, 0.1);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)		    camera.ProcessKeyboardMovement(RIGHT, 0.1);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)		    camera.ProcessKeyboardMovement(FORWARD, 0.1);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)		    camera.ProcessKeyboardMovement(BACKWARD, 0.1);
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)		camera.ProcessKeyboardRotation(1, 0.0, 1);
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)		camera.ProcessKeyboardRotation(-1, 0.0, 1);
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)		    camera.ProcessKeyboardRotation(0.0, 1.0, 1);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)		camera.ProcessKeyboardRotation(0.0, -1.0, 1);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	if (width < height)
	glViewport(0,0,width,width);
	else
	glViewport((width - height)/2,0,height,height);
}


