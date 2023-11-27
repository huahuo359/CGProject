#include <GL/glew.h>
#include <GLFW/glfw3.h>


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "tools/shader.h"
#include "tools/camera.h"
#include "tools/model.h"
#include "tools/Gshader.h"

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "tools/shader.h"
#include "tools/camera.h"
#include "tools/model.h"
#include "tools/Gshader.h"

#include <iostream>

// Represents a single particle and its state
struct Particle {
    glm::vec2 Position, Velocity;
    glm::vec4 Color;
    GLfloat Life;

    Particle() : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f) { }
};


// ParticleGenerator acts as a container for rendering a large number of 
// particles by repeatedly spawning and updating particles and killing 
// them after a given amount of time.
class ParticleGenerator
{
public:
    // Constructor
    ParticleGenerator(Shader shader, GLuint texture, GLuint amount):shader(shader), texture(texture), amount(amount) {
        // Set up mesh and attribute properties
        GLuint VBO;
        GLfloat particle_quad[] = {
            0.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 0.0f,

            0.0f, 1.0f, 0.0f, 1.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            1.0f, 0.0f, 1.0f, 0.0f
        }; 
        glGenVertexArrays(1, &this->VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(this->VAO);
        // Fill mesh buffer
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(particle_quad), particle_quad, GL_STATIC_DRAW);
        // Set mesh attributes
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid*)0);
        glBindVertexArray(0);

        // Create this->amount default particle instances
        for (GLuint i = 0; i < this->amount; ++i){
            glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            this->particles.push_back(Particle());
        }
            
    }
    // Update all particles
    void Update(GLfloat dt, GLuint newParticles, glm::vec2 offset = glm::vec2(0.0f, 0.0f)) {
        // Add new particles 
        for (GLuint i = 0; i < newParticles; ++i) {
            int unusedParticle = this->firstUnusedParticle();
            this->respawnParticle(this->particles[unusedParticle], offset);
        }
        // Update all particles
        for (GLuint i = 0; i < this->amount; ++i) {
            Particle &p = this->particles[i];
            p.Life -= dt; // reduce life
            if (p.Life > 0.0f) {	// particle is alive, thus update
                p.Position -= p.Velocity * dt; 
                p.Color.a -= dt * 2.5;
            }
        }
    }
    // Render all particles
    void Draw() {
        // Use additive blending to give it a 'glow' effect
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        this->shader.use();
        for(Particle particle : this->particles) {
            if(particle.Life > 0.0f) {
                // Draw partical
                // 已经死亡的粒子不会进行绘制

            }
        }

        // reset to default blending mode
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
    }
  
private:
    // State
    std::vector<Particle> particles;
    GLuint amount;
    // Render state
    Shader shader;
    GLuint texture;
    GLuint VAO;
    GLuint lastUsedParticle = 0;
    // Initializes buffer and vertex attributes
    
    // Returns the first Particle index that's currently unused e.g. Life <= 0.0f or 0 if no particle is currently inactive
    GLuint firstUnusedParticle() {
        for(GLuint i=lastUsedParticle; i<this->amount; ++i) {
            if(this->particles[i].Life <= 0.0f) {
                lastUsedParticle = i;
                return i;
            }
        }

        for(GLuint i=0; i<lastUsedParticle; ++i) {
            if(this->particles[i].Life <= 0.0f) {
                lastUsedParticle = i;
                return i;
            }
        }

        lastUsedParticle = 0;
        return 0;
    }
    // Respawns particle
    // void respawnParticle(Particle &particle, GameObject &object, glm::vec2 offset = glm::vec2(0.0f, 0.0f))
    void respawnParticle(Particle &particle, glm::vec2 offset = glm::vec2(0.0f, 0.0f)) {
        GLfloat random = ((rand() % 100) - 50) / 10.0f;
        GLfloat rColor = 0.5 + ((rand() % 100) / 100.0f);
        //particle.Position = object.Position + random + offset;
        particle.Position = glm::vec2(0,0) + random + offset;
        particle.Color = glm::vec4(rColor, rColor, rColor, 1.0f);
        particle.Life = 1.0f;
        particle.Velocity = glm::vec2(1.0f, 1.0f)*0.1f;
    }
};
