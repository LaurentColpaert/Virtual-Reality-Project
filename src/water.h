#ifndef WATER_H
#define WATER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

#include "./simple_shader.h"
#include "./object.h"

class Water{
public:
    Object plane;
    Shader water_shader = Shader(PATH_TO_SHADER "/water/water.vs", PATH_TO_SHADER "/water/water.fs"); 

    Water(int length,float density_per_cell, glm::vec3 translate_vector){
        plane = Object();
        plane.makeObject(make_grid(length,density_per_cell),length*length*6*density_per_cell*density_per_cell,water_shader);
        plane.model = glm::translate(plane.model, translate_vector);
    }

    void setup_water_shader(float ambient, float diffuse, float specular){
        water_shader.use();
        water_shader.setFloat("shininess", 22.0f);
        water_shader.setFloat("light.ambient_strength", ambient);
        water_shader.setFloat("light.diffuse_strength", diffuse);
        water_shader.setFloat("light.specular_strength", specular);
        water_shader.setFloat("light.constant", 1.0);
        water_shader.setFloat("light.linear", 0.14);
        water_shader.setFloat("light.quadratic", 0.07);
    }


    void draw(Camera camera, glm::vec3 materialColour, glm::vec3 light_pos, double now ){
        water_shader.use();
        water_shader.setMatrix4("M", plane.model);
        water_shader.setMatrix4("itM", glm::inverseTranspose(plane.model));
        water_shader.setVector3f("materialColour", materialColour);
    	water_shader.setMatrix4("V", camera.GetViewMatrix());
		water_shader.setMatrix4("P", camera.GetProjectionMatrix());
        water_shader.setVector3f("u_view_pos", camera.Position);
        water_shader.setVector3f("light.light_pos", light_pos);
        water_shader.setFloat("time",now);

        plane.draw();
    }

private:
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

};
#endif