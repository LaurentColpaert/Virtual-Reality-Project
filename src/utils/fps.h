/**
* @brief This header file defines the FPS class.
*
* @author Adela Surca & Laurent Colpaert.
*
* @project OpenGL project
*
**/
#ifndef FPS_H
#define FPS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

/**
* @brief Class that handle the calculation of the frame rate and display it in the title's window
**/
class FPS{
public:
    double prev = 0;
    int deltaFrame = 0;
    GLFWwindow* window = nullptr;

    /** Constructor **/
    FPS(GLFWwindow* window){
        this->window = window;
    }

    /** Compute the number of frame rate per second and display the result in the title **/
    void display(double now) {
        double deltaTime = now - prev;
        deltaFrame++;
        if (deltaTime > 0.5) {
            prev = now;
            const double fpsCount = (double)deltaFrame / deltaTime;
            deltaFrame = 0;
            std::string title = "Project - " + std::to_string(fpsCount) + " fps";
            glfwSetWindowTitle(window,title.c_str());
        }
    }
};
#endif

