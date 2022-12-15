#include<iostream>

//include glad before GLFW to avoid header conflict or define "#define GLFW_INCLUDE_NONE"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <map>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "./camera.h"
#include "./simple_shader.h"
#include "./tess_shader.h"
#include "./object.h"
#include "./terrain_generation.h"

const int src_width = 700;
const int src_height = 700;

Camera camera(glm::vec3(-2.0, 58.0, -5.0), glm::vec3(0.0, 0.5, 0.0), 90.0);

float lastX = src_width / 2.0f;
float lastY = src_height / 2.0f;
bool firstMouse = true;

GLuint compileShader(std::string shaderCode, GLenum shaderType);
GLuint compileProgram(GLuint vertexShader, GLuint fragmentShader);
void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
std::vector<Vertex> make_grid(int length, float density_per_cell);
void draw_water(Shader water_shader,Object plane,glm::mat4 view,glm::mat4 perspective,glm::vec3 materialColour, glm::vec3 light_pos, double now );
void setup_water(Shader water_shader,float ambient, float diffuse, float specular);
void loadCubemapFace(const char * path, const GLenum& targetFace);

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
	GLFWwindow* window = glfwCreateWindow(src_width, src_height, "Project", nullptr, nullptr);
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

	//glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glViewport(0,0,src_width,src_height);
	glfwSetFramebufferSizeCallback(window,framebuffer_size_callback);
	glEnable(GL_DEPTH_TEST);

	//comment to use texture
	//glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
	//comment to stop using the mouse to move
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	GLint maxTessLevel;
	glGetIntegerv(GL_MAX_TESS_GEN_LEVEL,&maxTessLevel);
	printf("Max available tesselation level : %d", maxTessLevel);
	glPatchParameteri(GL_PATCH_VERTICES,4);

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
	
	Terrain terrain = Terrain();
	Shader water_shader(PATH_TO_SHADER "/water/water.vs", PATH_TO_SHADER "/simple.fs");
	Shader simple_shader(PATH_TO_SHADER "/simple.vs", PATH_TO_SHADER "/simple.fs");
	Shader skybox_shader(PATH_TO_SHADER "/sky_box/sky.vs", PATH_TO_SHADER "/sky_box/sky.fs");

	int length = 10;
	float density_per_cell = 4.0;
	Object plane = Object();
	plane.makeObject(make_grid(length,density_per_cell),length*length*6*density_per_cell*density_per_cell,water_shader);
	plane.model = glm::translate(plane.model, glm::vec3(0.0,55,0.0));

	Object skybox(PATH_TO_OBJECTS "/cube.obj");
	skybox.makeObject(skybox_shader);

	Object sphere(PATH_TO_OBJECTS "/sphere_smooth.obj");
	sphere.makeObject(simple_shader);
	sphere.model = glm::translate(sphere.model, glm::vec3(4.0,55,0.0));

	//Create the cubemap texture
	GLuint sky_texture;
	glGenTextures(1, &sky_texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, sky_texture);

	// texture parameters
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	

	//This is the image you will use as your skybox
	std::string pathToCubeMap = PATH_TO_TEXTURE "/sky_box/";

	std::map<std::string, GLenum> facesToLoad = { 
		{pathToCubeMap + "right.png",GL_TEXTURE_CUBE_MAP_POSITIVE_X},
		{pathToCubeMap + "top.png",GL_TEXTURE_CUBE_MAP_POSITIVE_Y},
		{pathToCubeMap + "front.png",GL_TEXTURE_CUBE_MAP_POSITIVE_Z},
		{pathToCubeMap + "left.png",GL_TEXTURE_CUBE_MAP_NEGATIVE_X},
		{pathToCubeMap + "bottom.png",GL_TEXTURE_CUBE_MAP_NEGATIVE_Y},
		{pathToCubeMap + "back.png",GL_TEXTURE_CUBE_MAP_NEGATIVE_Z},
	};
	//load the six faces, you need to complete the function
	for (std::pair<std::string, GLenum> pair : facesToLoad) {
		loadCubemapFace(pair.first.c_str(), pair.second);
	}

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

	glm::mat4 view = camera.GetViewMatrix();
	glm::mat4 perspective = camera.GetProjectionMatrix();

	float ambient = 0.1;
	float diffuse = 0.8;
	float specular = 1.0;

	glm::vec3 light_pos = glm::vec3(5.0, 59.0, 5);
	glm::vec3 materialColour = glm::vec3(0.,0.,0.5);	

	//Rendering
	setup_water(water_shader,ambient,diffuse,specular);
	setup_water(simple_shader,ambient,diffuse,specular);
	
	glfwSwapInterval(1);
	while (!glfwWindowShouldClose(window)) {
		processInput(window);
		view = camera.GetViewMatrix();
		glfwPollEvents();
		double now = glfwGetTime();
		glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		
		//Draw the cubemap
		draw_water( water_shader,plane,view,perspective,materialColour,light_pos, now );
		terrain.Use(camera,src_width,src_height);

		//Use the shader for the cube map

		glDepthFunc(GL_LEQUAL);
		skybox_shader.use();
		//Set the relevant uniform
		skybox_shader.setMatrix4("V", view);
		skybox_shader.setMatrix4("P", perspective);
		skybox_shader.setInteger("cubemapTexture", 0);
		
		//Activate and bind the texture for the cubemap
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP,sky_texture);
		skybox.draw();
		glDepthFunc(GL_LESS);

		fps(now);
		glfwSwapBuffers(window);
	}
	//clean up ressource
	terrain.Destroy();

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


void setup_water(Shader water_shader,float ambient, float diffuse, float specular){
	water_shader.use();
	water_shader.setFloat("shininess", 22.0f);
	water_shader.setFloat("light.ambient_strength", ambient);
	water_shader.setFloat("light.diffuse_strength", diffuse);
	water_shader.setFloat("light.specular_strength", specular);
	water_shader.setFloat("light.constant", 1.0);
	water_shader.setFloat("light.linear", 0.14);
	water_shader.setFloat("light.quadratic", 0.07);
}

void draw_water(Shader water_shader,Object plane,glm::mat4 view,glm::mat4 perspective,glm::vec3 materialColour, glm::vec3 light_pos, double now ){
	water_shader.use();
	water_shader.setMatrix4("M", plane.model);
	water_shader.setMatrix4("itM", glm::inverseTranspose(plane.model));
	water_shader.setVector3f("materialColour", materialColour);
	plane.draw();
	water_shader.setMatrix4("V", view);
	water_shader.setMatrix4("P", perspective);
	water_shader.setVector3f("u_view_pos", camera.Position);

	// auto delta = light_pos + glm::vec3(0.0,0.0,2 * std::sin(now));
	water_shader.setVector3f("light.light_pos", light_pos);
	water_shader.setFloat("time",now);


}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	if (width < height)
	glViewport(0,0,width,width);
	else
	glViewport((width - height)/2,0,height,height);
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

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

std::vector<Vertex> make_grid(int length, float density_per_cell){
	std::vector<Vertex> vertices;
	float size = length * density_per_cell;
	float j,i;
	float addition = 1 / density_per_cell;
	for(float q=0; q< size; q++){
		j = q / density_per_cell;
		for(float w=0; w<size; w++){
			i = w / density_per_cell;
			Vertex v1,v2,v3,v4,v5,v6;
			v1.Position = glm::vec3(i,0,j+addition);
			v1.Texture = glm::vec2(0.0);
			v1.Normal = glm::vec3(0.0,0.0,1.0);

			v2.Position = glm::vec3(i,0,j);
			v2.Texture = glm::vec2(0.0);
			v2.Normal = glm::vec3(0.0,0.0,1.0);

			v3.Position = glm::vec3(i+addition,0,j);
			v3.Texture = glm::vec2(0.0);
			v3.Normal = glm::vec3(0.0,0.0,1.0);

			v4.Position = glm::vec3(i,0,j+addition);
			v4.Texture = glm::vec2(0.0);
			v4.Normal = glm::vec3(0.0,0.0,1.0);

			v5.Position = glm::vec3(i+addition,0,j+addition);
			v5.Texture = glm::vec2(0.0);
			v5.Normal = glm::vec3(0.0,0.0,1.0);

			v6.Position = glm::vec3(i+addition,0,j);
			v6.Texture = glm::vec2(0.0);
			v6.Normal = glm::vec3(0.0,0.0,1.0);

			vertices.push_back(v1);
			vertices.push_back(v2);
			vertices.push_back(v3);
			vertices.push_back(v4);
			vertices.push_back(v5);
			vertices.push_back(v6);
		}
	}
	return vertices;
}

void loadCubemapFace(const char * path, const GLenum& targetFace){
	int imWidth, imHeight, imNrChannels;
	//Load the image using stbi_load
	unsigned char* data = stbi_load(path, &imWidth, &imHeight, &imNrChannels, 0);
	if (data)
	{
		//Send the image to the the buffer
		glTexImage2D(targetFace, 0, GL_RGB, imWidth, imHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	else {
		std::cout << "Failed to Load texture" << std::endl;
		const char* reason = stbi_failure_reason();
		std::cout << reason << std::endl;
	}
	//Don't forget to free the memory
	stbi_image_free(data);
}