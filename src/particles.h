#ifndef PARTICLE_H
#define PARTICLE_H
#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "./simple_shader.h"
#include "./spirit.h"
#include "./camera.h"

// Represents a single particle and its state
struct Particle {
    glm::vec3 Position, Velocity;
    glm::vec4 Color;
    float Life;

    Particle() : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(10.0f) {}
};

// ParticleGenerator acts as a container for rendering a large number of 
// particles by repeatedly spawning and updating particles and killing 
// them after a given amount of time.
class ParticleGenerator{
public:
    std::vector<Particle> particles;
    unsigned int amount;
    unsigned int VAO;
    unsigned int texture;
    unsigned int lastUsedParticle;
    
    Shader shader = Shader(PATH_TO_SHADER "/particle/particle.vs", PATH_TO_SHADER "/particle/particle.fs");
    Spirit* spirit;
    Camera* camera;

    ParticleGenerator(unsigned int amount,Spirit* spirit,Camera* camera){
        this->spirit = spirit;
        this->amount = amount;
        this->camera = camera;
        texture = loadTexture(PATH_TO_TEXTURE "/round_particle.png",8);
        
        init();
    }

    void Update(float dt, unsigned int newParticles,Object* object, glm::vec3 offset = glm::vec3(0.0f)){
        // add new particles 
        for (unsigned int i = 0; i < newParticles; ++i){
            int unusedParticle = firstUnusedParticle();
            respawnParticle(particles[unusedParticle],object, offset);
        }
        // update all particles
        for (unsigned int i = 0; i < amount; ++i){
            Particle &p = particles[i];
            p.Life -= dt; // reduce life
            if (p.Life > 0.0f)  {	// particle is alive, thus update
                p.Position += p.Velocity * dt; 
                p.Color.a -= dt * 2.5f;
            }
        }
    }

    void draw(){
        // use additive blending to give it a 'glow' effect
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        shader.use();
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, texture);
        shader.setInteger("sprite",8);
        shader.setMatrix4("projection",camera->GetProjectionMatrix());
        shader.setMatrix4("view",camera->GetViewMatrix());

        for (Particle particle : particles)
        {
            if (particle.Life > 0.0f){
                shader.setVector3f("offset", particle.Position);
                shader.setVector4f("color", particle.Color);
                glBindVertexArray(VAO);
                glDrawArrays(GL_TRIANGLES, 0, 6);
                glBindVertexArray(0);
            }
        }
        // don't forget to reset to default blending mode
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void init(){
        // set up mesh and attribute properties
        unsigned int VBO;
        float particle_quad[] = {
            0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 0.0f,1.0f, 0.0f,
            0.0f, 0.0f, 0.0f,0.0f, 0.0f,

            0.0f, 1.0f, 0.0f,0.0f, 1.0f,
            1.0f, 1.0f, 0.0f,1.0f, 1.0f,
            1.0f, 0.0f, 0.0f,1.0f, 0.0f
        }; 
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        // fill mesh buffer
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);
        // set mesh attributes
        auto att_pos = glGetAttribLocation(shader.ID, "position");
		glEnableVertexAttribArray(att_pos);
		glVertexAttribPointer(att_pos, 3, GL_FLOAT, false, 5 * sizeof(float), (void*)0);
        auto att_tex = glGetAttribLocation(shader.ID, "tex_coord");
        glEnableVertexAttribArray(att_tex);
        glVertexAttribPointer(att_tex, 2, GL_FLOAT, false, 5 * sizeof(float), (void*)(3 * sizeof(float)));
		
        //desactive the buffer
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);

         // create this->amount default particle instances
        for (unsigned int i = 0; i < this->amount; ++i)
            this->particles.push_back(Particle());
    }

    unsigned int firstUnusedParticle(){
        // first search from last used particle, this will usually return almost instantly
        for (unsigned int i = lastUsedParticle; i < amount; ++i){
            if (particles[i].Life <= 0.0f){
                lastUsedParticle = i;
                return i;
            }
        }
        // otherwise, do a linear search
        for (unsigned int i = 0; i < lastUsedParticle; ++i){
            if (particles[i].Life <= 0.0f){
                lastUsedParticle = i;
                return i;
            }
        }
        // all particles are taken, override the first one (note that if it repeatedly hits this case, more particles should be reserved)
        lastUsedParticle = 0;
        return 0;
    }

    void respawnParticle(Particle &particle,Object* object, glm::vec3 offset = glm::vec3(0.0)){
        float random = ((rand() % 100) - 50) / 20.0f;
        float rColor = 0.5f + ((rand() % 100) / 100.0f);
        particle.Position = object->transform.getWorldTranslation()+glm::vec3(-0.5,2,0);
        particle.Color = glm::vec4(rColor, rColor, rColor, 1.0f);
        particle.Life = 1.0f;
        if (rand() % 100 < 50){
            particle.Velocity = glm::vec3(0.2,0.4,-0.2) * random ;
        }
        else{ 
            // particle.Position = spirit->getObject()->transform.getWorldTranslation() - random +glm::vec3(-0.5,2,0);
            particle.Velocity = glm::vec3(-0.2,0.4,0.2) * random ;
        }
    }

    unsigned int loadTexture(char const * path, int texture_nb){
        unsigned int textureID;
        glGenTextures(1, &textureID);
        glActiveTexture(GL_TEXTURE0+texture_nb);

        int width, height, nrComponents;
        unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
        if (data)
        {
            GLenum format;
            if (nrComponents == 1)
                format = GL_RED;
            else if (nrComponents == 3)
                format = GL_RGB;
            else if (nrComponents == 4)
                format = GL_RGBA;

            glActiveTexture(GL_TEXTURE0+texture_nb);
            glBindTexture(GL_TEXTURE_2D, textureID);
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT); // for this tutorial: use GL_CLAMP_TO_EDGE to prevent semi-transparent borders. Due to interpolation it takes texels from next repeat 
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            stbi_image_free(data);
        }
        else
        {
            std::cout << "Texture failed to load at path: " << path << std::endl;
            stbi_image_free(data);
        }
        return textureID;
    }
};

#endif