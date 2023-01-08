/**
* @brief This header file defines the object class. Based on the assistant of the INFO-H-502 at the ULB
*
* @author Adela Surca & Laurent Colpaert
*
* @project OpenGL project
*
**/
#ifndef OBJECT_H
#define OBJECT_H

#include<iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>

#include <btBulletDynamicsCommon.h>
#include "./utils/transform.h"


/**The struct that defines a vertex**/
struct Vertex {
	glm::vec3 Position;
	glm::vec2 Texture;
	glm::vec3 Normal;
};

/**
 * @brief Class that parse ´.obj´ file and associate the mesh extracted to buffers and shaders. 
 * And allow the rendering of the object in the OpenGL pipeline
**/
class Object
{
public:
	std::vector<glm::vec3> positions;
	std::vector<glm::vec2> textures;
	std::vector<glm::vec3> normals;
	std::vector<Vertex> vertices;

	int numVertices;

	GLuint VBO, VAO;

	Transform transform;
	btRigidBody* rigid;
	bool verbose = false;

	/** Creates an empty object without reading a ´.obj´ file. Used for hand-made mesh**/
	Object(){
		transform = Transform();
	}

	/** Read an ´.obj´ file and parse it correctly to create an array of vertex**/
	Object(const char* path, bool verbose = false) {
		transform = Transform();
		this->verbose = verbose;
		std::ifstream infile(path);
		if (verbose) std::cout << path << std::endl;
		std::string line;
		while (std::getline(infile, line))
		{
			std::istringstream iss(line);
			std::string indice;
			iss >> indice;
			//std::cout << "indice : " << indice << std::endl;
			if (indice == "v") {
				float x, y, z;
				iss >> x >> y >> z;
				positions.push_back(glm::vec3(x, y, z));

			}
			else if (indice == "vn") {
				float x, y, z;
				iss >> x >> y >> z;
				normals.push_back(glm::vec3(x, y, z));
			}
			else if (indice == "vt") {
				float u, v;
				iss >> u >> v;
				textures.push_back(glm::vec2(u, v));
			}
			else if (indice == "f") {
				std::string f1, f2, f3;
				iss >> f1 >> f2 >> f3;

				std::string p, t, n;

				//for face 1
				Vertex v1;

				p = f1.substr(0, f1.find("/"));
				f1.erase(0, f1.find("/") + 1);

				t = f1.substr(0, f1.find("/"));
				f1.erase(0, f1.find("/") + 1);

				n = f1.substr(0, f1.find("/"));


				v1.Position = positions.at(std::stof(p) - 1);
				v1.Normal = normals.at(std::stof(n) - 1);
				v1.Texture = textures.at(std::stof(t) - 1);
				vertices.push_back(v1);

				//for face 12
				Vertex v2;

				p = f2.substr(0, f2.find("/"));
				f2.erase(0, f2.find("/") + 1);

				t = f2.substr(0, f2.find("/"));
				f2.erase(0, f2.find("/") + 1);

				n = f2.substr(0, f2.find("/"));


				v2.Position = positions.at(std::stof(p) - 1);
				v2.Normal = normals.at(std::stof(n) - 1);
				v2.Texture = textures.at(std::stof(t) - 1);
				vertices.push_back(v2);

				//for face 3
				Vertex v3;

				p = f3.substr(0, f3.find("/"));
				f3.erase(0, f3.find("/") + 1);

				t = f3.substr(0, f3.find("/"));
				f3.erase(0, f3.find("/") + 1);

				n = f3.substr(0, f3.find("/"));


				v3.Position = positions.at(std::stof(p) - 1);
				v3.Normal = normals.at(std::stof(n) - 1);
				v3.Texture = textures.at(std::stof(t) - 1);
				vertices.push_back(v3);
			}
		}
		if(verbose)	std::cout << "Load model with " << vertices.size() << std::endl;

		infile.close();
		numVertices = vertices.size();
	}


	/** Parse the array of vertices into an OpenGL readable format
	 *  and put them in the correct buffer. Link the shader and the texture if used
	**/
	void makeObject(Shader shader, bool texture = true) {
		float* data = new float[8 * numVertices];
		for (int i = 0; i < numVertices; i++) {
			Vertex v = vertices.at(i);
			data[i * 8] = v.Position.x;
			data[i * 8 + 1] = v.Position.y;
			data[i * 8 + 2] = v.Position.z;

			data[i * 8 + 3] = v.Texture.x;
			data[i * 8 + 4] = v.Texture.y;

			data[i * 8 + 5] = v.Normal.x;
			data[i * 8 + 6] = v.Normal.y;
			data[i * 8 + 7] = v.Normal.z;
		}
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		//define VBO and VAO as active buffer and active vertex array
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * numVertices, data, GL_STATIC_DRAW);

		auto att_pos = glGetAttribLocation(shader.ID, "position");
		glEnableVertexAttribArray(att_pos);
		glVertexAttribPointer(att_pos, 3, GL_FLOAT, false, 8 * sizeof(float), (void*)0);

		if (texture) {
			auto att_tex = glGetAttribLocation(shader.ID, "tex_coord");
			glEnableVertexAttribArray(att_tex);
			glVertexAttribPointer(att_tex, 2, GL_FLOAT, false, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		}
		auto att_col = glGetAttribLocation(shader.ID, "normal");
		glEnableVertexAttribArray(att_col);
		glVertexAttribPointer(att_col, 3, GL_FLOAT, false, 8 * sizeof(float), (void*)(5 * sizeof(float)));
		
		//desactive the buffer
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
		delete[] data;
	}

	/** Parse the array of vertices that was created by hand into an OpenGL readable format
	 *  and put them in the correct buffer. Link the shader and the texture if used
	**/
	void makeObject(std::vector<Vertex> vertices, int numVertices, Shader shader, bool texture = true) {
		
		this->numVertices = numVertices;
		if (verbose) printf("Load model with %d \n", numVertices);
		float* data = new float[8 * numVertices];
		for (int i = 0; i < numVertices; i++) {
			Vertex v = vertices.at(i);
			data[i * 8] = v.Position.x;
			data[i * 8 + 1] = v.Position.y;
			data[i * 8 + 2] = v.Position.z;

			data[i * 8 + 3] = v.Texture.x;
			data[i * 8 + 4] = v.Texture.y;

			data[i * 8 + 5] = v.Normal.x;
			data[i * 8 + 6] = v.Normal.y;
			data[i * 8 + 7] = v.Normal.z;
		}
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);

		//define VBO and VAO as active buffer and active vertex array
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * numVertices, data, GL_STATIC_DRAW);

		auto att_pos = glGetAttribLocation(shader.ID, "position");
		glEnableVertexAttribArray(att_pos);
		glVertexAttribPointer(att_pos, 3, GL_FLOAT, false, 8 * sizeof(float), (void*)0);
		
		if (texture) {
			auto att_tex = glGetAttribLocation(shader.ID, "tex_coord");
			glEnableVertexAttribArray(att_tex);
			glVertexAttribPointer(att_tex, 2, GL_FLOAT, false, 8 * sizeof(float), (void*)(3 * sizeof(float)));	
		}
		auto att_col = glGetAttribLocation(shader.ID, "normal");
		glEnableVertexAttribArray(att_col);
		glVertexAttribPointer(att_col, 3, GL_FLOAT, false, 8 * sizeof(float), (void*)(5 * sizeof(float)));
		
		//desactive the buffer
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}

	void makeGround(Shader shader){
		// positions
		glm::vec3 pos1(-1.0f, 0.0f, 1.0f);
		glm::vec3 pos2(-1.0f, 0.0f,-1.0f);
		glm::vec3 pos3( 1.0f, 0.0f,-1.0f);
		glm::vec3 pos4( 1.0f, 0.0f, 1.0f);
		// texture coordinates
		glm::vec2 uv1(0.0f, 1.0f);
		glm::vec2 uv2(0.0f, 0.0f);
		glm::vec2 uv3(1.0f, 0.0f);  
		glm::vec2 uv4(1.0f, 1.0f);
		// normal vector
		glm::vec3 nm(0.0f, 1.0f, 0.0f);

		// calculate tangent/bitangent vectors of both triangles
		glm::vec3 tangent1, bitangent1;
		glm::vec3 tangent2, bitangent2;
		// triangle 1
		// ----------
		glm::vec3 edge1 = pos2 - pos1;
		glm::vec3 edge2 = pos3 - pos1;
		glm::vec2 deltaUV1 = uv2 - uv1;
		glm::vec2 deltaUV2 = uv3 - uv1;

		float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

		bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

		// triangle 2
		// ----------
		edge1 = pos3 - pos1;
		edge2 = pos4 - pos1;
		deltaUV1 = uv3 - uv1;
		deltaUV2 = uv4 - uv1;

		f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

		tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
		tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
		tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);


		bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
		bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
		bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);


		float quadVertices[] = {
			// positions            // normal         // texcoords  // tangent                          // bitangent
			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

			pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
			pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
			pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
		};
		// configure plane VAO
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

		auto pos = glGetAttribLocation(shader.ID, "aPos");
		glEnableVertexAttribArray(pos);
		glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
		
		auto normal = glGetAttribLocation(shader.ID, "aNormal");
		glEnableVertexAttribArray(normal);
		glVertexAttribPointer(normal, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
		
		auto tex = glGetAttribLocation(shader.ID, "aTexCoords");
		glEnableVertexAttribArray(tex);
		glVertexAttribPointer(tex, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
		
		auto tangent = glGetAttribLocation(shader.ID, "aTangent");
		glEnableVertexAttribArray(tangent);
		glVertexAttribPointer(tangent, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
		
		auto bitangent = glGetAttribLocation(shader.ID, "aBitangent");
		glEnableVertexAttribArray(bitangent);
		glVertexAttribPointer(bitangent, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));

	}

	/**	Bind your vertex arrays and call glDrawArrays **/
	void draw() {
		glBindVertexArray(this->VAO);
		glDrawArrays(GL_TRIANGLES, 0, numVertices);
	}
};
#endif