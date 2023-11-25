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

#define MAX_PARTICLES 1000
#define PARTICLE_LIFETIME 10.0f

#define PARTICLE_TYPE_LAUNCHER 0.0f
#define PARTICLE_TYPE_SHELL 1.0f
#define PARTICLE_TYPE_SECONDARY_SHELL 2.0f



struct Particle
{
    float Type; // 发射器/粒子
    glm::vec3 Pos;
    glm::vec3 Vel; 
    float LifetimeMillis; 
};


class ParticleSystem
{
public:
    ParticleSystem() {
        m_currTFB = 0;
        m_currTFB = 1;
        m_isFirst = true;
        m_time = 0;
        m_pTexture = NULL;            
    
        
    }

    ~ParticleSystem();

    bool InitParticleSystem(const glm::vec3& Pos){
        Particle Particles[MAX_PARTICLES];

        Particles[0].Type = PARTICLE_TYPE_LAUNCHER; // 设置第一个粒子为发射器
        Particles[0].Pos = Pos;
        Particles[0].Vel = glm::vec3(0.0f, 0.0001f, 0.0f);
        Particles[0].LifetimeMillis = 0.0f;

        glGenTransformFeedbacks(2, m_transformFeedback);   
        
        
        

    }

    void Render(int DeltaTimeMillis, const glm::mat4& VP, const glm::vec3& CameraPos);

private:

    bool m_isFirst;
    unsigned int m_currVB;
    unsigned int m_currTFB;
    GLuint m_particleBuffer[2];
    GLuint m_transformFeedback[2];
    PSUpdateTechnique m_updateTechnique;
    BillboardTechnique m_billboardTechnique;
    RandomTexture m_randomTexture;
    Texture* m_pTexture;
    int m_time;
};