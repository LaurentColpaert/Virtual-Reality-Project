/**
* @brief This header file defines the callbacks class.
*
* @author Adela Surca & Laurent Colpaert
*
* @project OpenGL project
*
**/
#ifndef CALLBACKS_H
#define CALLBACKS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

/**
* @brief Class that handle the different input such as the keyboard
**/
class Callbacks{
public:
    Camera* camera;

    Callbacks(Camera* camera){
        this->camera = camera;
    }
    /**Handle the input of the keyboard and launch the corresponding function**/
    void processInput(GLFWwindow* window) {
	    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)		glfwSetWindowShouldClose(window, true);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)		    camera->ProcessKeyboardMovement(LEFT, 0.1);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)		    camera->ProcessKeyboardMovement(RIGHT, 0.1);
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)		    camera->ProcessKeyboardMovement(FORWARD, 0.1);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)		    camera->ProcessKeyboardMovement(BACKWARD, 0.1);
        if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)		camera->ProcessKeyboardRotation(1, 0.0, 1);
        if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)		camera->ProcessKeyboardRotation(-1, 0.0, 1);
        if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)		    camera->ProcessKeyboardRotation(0.0, 1.0, 1);
        if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)		camera->ProcessKeyboardRotation(0.0, -1.0, 1);	
    }
};
#endif