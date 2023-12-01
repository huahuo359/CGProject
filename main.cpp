// #include <glad/glad.h>
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


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
GLuint loadTexture(GLchar* path, int imagecase); // case 1 for png, case 2 for jpg
GLuint loadCubemap(vector<const GLchar*> faces);

// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 5.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
// glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
glm::vec3 lightDir(-0.2f, -1.0f, -0.3f);

// Light attributes
glm::vec3 lightPos(0.0f, 0.0f, -10.0f);


GLFWwindow* Init()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return NULL;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

 
    // if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    // {
    //     std::cout << "Failed to initialize GLAD" << std::endl;
    //     return NULL;
    // }

    if (glewInit() != GLEW_OK) {
        // 处理初始化失败
        std::cout << "Failed to initialize GLEW" << std::endl;
        return NULL;
    }


    stbi_set_flip_vertically_on_load(true);

    glEnable(GL_DEPTH_TEST);

    return window;

}




// Load Obj Class
class Obj {

public:
    Model objModel;
    Shader objShader;
    GLuint TextureID;
    GLuint TextureNormal;
    GLuint TextureSpecular;


    Obj(): objModel("plane/ship_03.obj"), objShader("shaders/earth.vs", "shaders/earth.fs") {
        
     
        std::cout << "load our obj" << endl;
        TextureID = loadTexture("plane/diffuse2.png", 1);
        TextureNormal = loadTexture("plane/normal2.png", 1);
        TextureSpecular = loadTexture("plane/specular2.png",1);

    }

    void Draw() {
        objShader.use();

        
        objShader.setVec3("light.ambient", glm::vec3(0.05f, 0.05f, 0.05f)); 
        objShader.setVec3("light.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
        objShader.setVec3("light.specular", glm::vec3(0.5f, 0.5f, 0.5f));
        // earthShader.setVec3("light.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
        objShader.setVec3("viewPos", camera.Position);
        
        // set textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureID);
        objShader.setInt("texture_diffuse", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, TextureNormal);
        objShader.setInt("texture_normal", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, TextureSpecular);
        objShader.setInt("texture_specular", 2);
       
        glm::mat4 view;
        view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view       = glm::translate(view, glm::vec3(0.0f, 0.0f, -4.0f));
        glm::mat4 model = glm::mat4(1.0f);

        
        float angle = (GLfloat)glfwGetTime() * 3.5f;
        //model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.3f));

        objShader.setMat4("projection", projection); 
        objShader.setMat4("view", view);
        objShader.setMat4("model", model);

        objShader.setVec3("light.direction", glm::vec3(0.0f, 0.0f, 10.0f));


        objModel.Draw(objShader);
    }


};


class Car {

public:
    Model objModel;
    Shader objShader;
    GLuint TextureID;
    GLuint TextureNormal;
    GLuint TextureSpecular;


    Car(): objModel("truck/monster truck 3d model.obj"), objShader("shaders/car.vs", "shaders/car.fs") {
        
     
        std::cout << "load our car" << endl;
        TextureID = loadTexture("truck/truck2.png", 1);
        TextureNormal = loadTexture("truck/normal.jpg", 2);
        TextureSpecular = loadTexture("truck/specular.png",1);
        std::cout << "load is ok" << endl;

    }

    void Draw() {
        objShader.use();

        
        objShader.setVec3("light.ambient", glm::vec3(1.0f, 1.0f, 1.0f)); 
        objShader.setVec3("light.diffuse", glm::vec3(0.0f, 0.0f, 0.0f));
        objShader.setVec3("light.specular", glm::vec3(0.0f, 0.0f, 0.0f));
        // earthShader.setVec3("light.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
        objShader.setVec3("viewPos", camera.Position);
        
        // set textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureID);
        objShader.setInt("texture_diffuse", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, TextureNormal);
        objShader.setInt("texture_normal", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, TextureSpecular);
        objShader.setInt("texture_specular", 2);
       
        glm::mat4 view;
        view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view       = glm::translate(view, glm::vec3(0.0f, 0.0f, -4.0f));
        glm::mat4 model = glm::mat4(1.0f);

        
        float angle = (GLfloat)glfwGetTime() * 3.5f;
        model = glm::scale(model, glm::vec3(0.01f, 0.01f, 0.01f));
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.3f));

        objShader.setMat4("projection", projection); 
        objShader.setMat4("view", view);
        objShader.setMat4("model", model);

        objShader.setVec3("light.direction", glm::vec3(0.0f, 0.0f, -10.0f));


        objModel.Draw(objShader);
    }


};




std::vector<GLfloat> controlPoints = {
    -1.5f, 0.5f, 0.0f,
    -0.5f, 0.5f, 0.0f,
     0.5f, 0.5f, 0.0f,
     1.5f, 0.5f, 0.0f,

    -1.5f, 1.0f, 0.0f,
    -0.5f, 1.0f, -1.0f,
     0.5f, 1.0f, -1.0f,
     1.5f, 1.0f, 0.0f,

    -1.5f, 1.5f, 0.0f,
    -0.5f, 1.5f, -2.0f,
     0.5f, 1.5f, -2.0f,
     1.5f, 1.5f, 0.0f,

    -1.5f, 2.0f, 0.0f,
    -0.5f, 2.0f, 0.0f,
     0.5f, 2.0f, 0.0f,
     1.5f, 2.0f, 0.0f,
}; 

class Bezier {
public:


    std::vector<float> points;
    Shader shader;
    unsigned int VBO, VAO;
    GLuint EBO;

    int Fact(int n) {
        int res = 1;
        if(n==0 || n==1) { return 1;}
        
        for(int i=1; i<=n; i++) {
            res *= i;
        }

        return res;
    }

    int BinomialCoeff(int n, int i) {
        return Fact(n)/(Fact(i)*Fact(n-i));
    }

    Bezier(): shader("shaders/bezier.vs", "shaders/bezier.fs") {
        for(int i=0; i<400; ++i) {
            float u = (float)i/400;
            for(int j=0; j<400; ++j) {
                float v = (float)j/400;
                 // 计算二次贝塞尔曲面上的点
                glm::vec3 point(0.0f, 0.0f, 0.0f);
                for (int row = 0; row < 4; ++row) {
                    float blendU = (float)BinomialCoeff(3, row) * glm::pow(1.0f - u, 3 - row) * glm::pow(u, row);
                    for (int col = 0; col < 4; ++col) {
                        
                        float blendV = (float)BinomialCoeff(3, col) * glm::pow(1.0f - v, 3 - col) * glm::pow(v, col);
                        point += blendU * blendV * glm::vec3(controlPoints[(row * 4 + col) * 3], controlPoints[(row * 4 + col) * 3 + 1], controlPoints[(row * 4 + col) * 3 + 2]);
                }
            }


            points.push_back(point.x);
            points.push_back(point.y);
            points.push_back(point.z);


            }
        }

        

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(float), &points[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // unbind
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

    }

    void Draw() {
        shader.use();
        shader.setInt("texture1", 1);
       
        glm::mat4 view;
        view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view       = glm::translate(view, glm::vec3(0.0f, 0.0f, -3.0f));
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    
    
        shader.setMat4("projection", projection); 
        shader.setMat4("view", view);
        shader.setMat4("model", model);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, points.size() / 3);
        glBindVertexArray(0);
    }


};

class Planet {
public:
    std::vector<float> sphereVertices;
    std::vector<int> sphereIndices;
    Shader sunShader;
    int PRECISE = 550;
    unsigned int VBO, VAO;
    GLuint EBO;
    GLuint TextureID;

    Planet(GLchar* path, int imagecase): sunShader("shaders/sphere.vs", "shaders/sphere.fs") {
        float PI = 3.14159;
        for(int r=0; r<=PRECISE; r++) {
        for(int c=0; c<=PRECISE; c++) {
            float theta = (float)c / float(PRECISE);
            float phi = (float)r / float(PRECISE);
            float xPos = std::sin(phi * PI) * std::cos(theta * 2.0f * PI);
            float yPos = std::cos(phi * PI);
            float zPos = std::sin(phi * PI) * std::sin(theta * 2.0f * PI);

            sphereVertices.push_back(xPos);
            sphereVertices.push_back(yPos);
            sphereVertices.push_back(zPos);
            sphereVertices.push_back(theta);
            sphereVertices.push_back(phi);

        }
    }

        for(int i=0; i<PRECISE; i++) {
            for(int j=0; j<PRECISE; j++) {
                sphereIndices.push_back(i * (PRECISE + 1) + j);
                sphereIndices.push_back((i+1) * (PRECISE + 1) + j);
                sphereIndices.push_back((i+1) * (PRECISE + 1) + j + 1);
                sphereIndices.push_back(i * (PRECISE + 1) + j);
                sphereIndices.push_back((i+1) * (PRECISE + 1) + j + 1);
                sphereIndices.push_back(i * (PRECISE + 1) + j + 1);
            }
        }

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), &sphereVertices[0], GL_STATIC_DRAW);

        glGenBuffers(1, &EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(int), &sphereIndices[0], GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // unbind
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

        TextureID = loadTexture(path, imagecase);

    }

    void Draw() {
        sunShader.use();
        sunShader.setInt("texture1", 1);
        // draw sun
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, TextureID);
       
        glm::mat4 view;
        view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view       = glm::translate(view, glm::vec3(0.0f, 0.0f, -4.0f));
        glm::mat4 model = glm::mat4(1.0f);
        float x_sun = 0;
        float z_sun = -10;
        model = glm::translate(model, glm::vec3( x_sun,  0.0f, z_sun));
        float angle = (GLfloat)glfwGetTime() * 25.0f;
       
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.3f));

        sunShader.setMat4("projection", projection); 
        sunShader.setMat4("view", view);
        sunShader.setMat4("model", model);

        glEnable(GL_CULL_FACE); 
        glCullFace(GL_BACK);
        glBindVertexArray(VAO);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, PRECISE*PRECISE*6, GL_UNSIGNED_INT, 0);

    

    }


};

class Earth : public Planet {
public:

    Shader earthShader;
    GLuint TextureNormal;
    GLuint TextureSpecular;
    Earth(GLchar* path, int imagecase): Planet(path, imagecase), earthShader("shaders/earth.vs", "shaders/earth.fs") {
        TextureNormal = loadTexture("image/planet/earth_normal.png", 1);
        TextureSpecular = loadTexture("image/planet/earth_specular.png",1);
    }

    void Draw() {
        
        earthShader.use();

        
        earthShader.setVec3("light.ambient", glm::vec3(0.05f, 0.05f, 0.05f)); 
        earthShader.setVec3("light.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
        earthShader.setVec3("light.specular", glm::vec3(0.5f, 0.5f, 0.5f));
        // earthShader.setVec3("light.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
        earthShader.setVec3("viewPos", camera.Position);
        
        // set texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureID);
        earthShader.setInt("texture_diffuse", 0);

        // set texture
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, TextureNormal);
        earthShader.setInt("texture_normal", 1);

        // set texture
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, TextureSpecular);
        earthShader.setInt("texture_specular", 2);
       
        glm::mat4 view;
        view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view       = glm::translate(view, glm::vec3(0.0f, 0.0f, -4.0f));
        glm::mat4 model = glm::mat4(1.0f);

        float x_earth1 = 0 + 3*std::cos((GLfloat)glfwGetTime() * 0.5f);
        float z_earth1 = -10 + 3*std::sin((GLfloat)glfwGetTime() * 0.5f);
        x_earth1 = -3;
        z_earth1 = -7;
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        model = glm::translate(model, glm::vec3( x_earth1/0.5f,  0.3f/0.5f, z_earth1/0.5f));
        
        float angle = (GLfloat)glfwGetTime() * 3.5f;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.3f));

        earthShader.setMat4("projection", projection); 
        earthShader.setMat4("view", view);
        earthShader.setMat4("model", model);

        glm::vec3 earthPos = glm::vec3(x_earth1, 0.3f, z_earth1);
        glm::vec3 sunPos = glm::vec3(0.0f, 0.0f, -10.0f);
        glm::vec3 lightDirection = earthPos - sunPos;
        earthShader.setVec3("light.direction", lightDirection);



        glEnable(GL_CULL_FACE); 
        glCullFace(GL_BACK);
        glBindVertexArray(VAO);
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, PRECISE*PRECISE*6, GL_UNSIGNED_INT, 0);
    }

};

class Moon: public Planet {

public:

    GShader moonShader;
    GLuint TextureNormal;
    GLuint TextureSpecular;
    Moon(GLchar* path, int imagecase): Planet(path, imagecase), moonShader("shaders/moon.vs", "shaders/moon.fs", "shaders/moon.gs") {
        TextureNormal = loadTexture("image/planet/earth_normal.png", 1);
        TextureSpecular = loadTexture("image/planet/earth_specular.png",1);
    }

    void Draw() {
        moonShader.use();
        moonShader.setVec3("light.ambient", glm::vec3(0.05f, 0.05f, 0.05f)); 
        moonShader.setVec3("light.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
        moonShader.setVec3("light.specular", glm::vec3(0.5f, 0.5f, 0.5f));
        moonShader.setVec3("viewPos", camera.Position);

        // set texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureID);
        moonShader.setInt("texture_diffuse", 0);

        // set texture
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, TextureNormal);
        moonShader.setInt("texture_normal", 1);

        // set texture
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, TextureSpecular);
        moonShader.setInt("texture_specular", 2);

        glm::mat4 view;
        view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 4.1f, 100.0f);
        view       = glm::translate(view, glm::vec3(0.0f, 0.0f, -5.0f));
        glm::mat4 model = glm::mat4(1.0f);

        float x_moon1 = 0 + 3*std::cos((GLfloat)glfwGetTime() * 0.5f);
        float z_moon1 = -30 + 3*std::sin((GLfloat)glfwGetTime() * 0.5f);
        x_moon1 = -3;
        z_moon1 = -7;
        // x_moon1 = 0;
        // z_moon1 = 0;
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        model = glm::translate(model, glm::vec3( x_moon1/0.5f,  0.3f/0.5f, z_moon1/0.5f));
        
        float angle = (GLfloat)glfwGetTime() * 3.5f;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.3f));

        moonShader.setMat4("projection", projection); 
        moonShader.setMat4("view", view);
        moonShader.setMat4("model", model);

        glm::vec3 earthPos = glm::vec3(x_moon1, 0.3f, z_moon1);
        glm::vec3 sunPos = glm::vec3(0.0f, 0.0f, -10.0f);
        glm::vec3 lightDirection = earthPos - sunPos;
        moonShader.setVec3("light.direction", lightDirection);

        // Set time
        moonShader.setFloat("time", static_cast<float>(glfwGetTime()));



        glEnable(GL_CULL_FACE); 
        glCullFace(GL_BACK);
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, PRECISE*PRECISE*6, GL_UNSIGNED_INT, 0);




    }

    


};

class Jupiter {

};

class Ground {
    /* 地面场景绘制 */
};

class Tree {
    /* 在近景中添加树的元素 */
};

class Aircraft {
    /* load 一个飞行器的模型 */
};


class Elevator {
    /* load 太空电梯场景 */
};

class SpaceStation {
    /* load 空间站的模型 */
};


class SkyBox {
public:

    Shader skyboxShader;
    GLuint skyboxVAO, skyboxVBO;
    GLuint cubemapTexture;
    vector<const GLchar*> faces;
     
    GLfloat skyboxVertices[108] = {
        // Positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
  
        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
  
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
   
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
  
        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,
  
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };

    SkyBox(vector<const GLchar*> faces):skyboxShader("shaders/skybox.vs", "shaders/skybox.fs"),faces(faces) {
    
        // Setup skybox VAO
       
        glGenVertexArrays(1, &skyboxVAO);
        glGenBuffers(1, &skyboxVBO);
        glBindVertexArray(skyboxVAO);
        glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        glBindVertexArray(0);
    
        cubemapTexture = loadCubemap(faces);
       

    }

    void Draw() {
        // Draw skybox first
        glDepthMask(GL_FALSE);// Remember to turn depth writing off
        skyboxShader.use();		
        // 这样设置 view 可以让天空盒看起来是无限大的
        glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix()));	// Remove any translation component of the view matrix
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        // glUniformMatrix4fv(glGetUniformLocation(skyboxShader.program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        // glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
        // skybox cube
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        //glUniform1i(glGetUniformLocation(shader.Program, "skybox"), 0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthMask(GL_TRUE);
    }

};

void mainLoop(GLFWwindow* window ) {


    // vector<const GLchar*> faces;
    // faces.push_back("image/skybox/right.jpg");
    // faces.push_back("image/skybox/left.jpg");
    // faces.push_back("image/skybox/bottom.jpg");
    // faces.push_back("image/skybox/top.jpg");
    // faces.push_back("image/skybox/back.jpg");
    // faces.push_back("image/skybox/front.jpg");
   
   
    // SkyBox skybox1(faces);
    // Obj planet;
    // Car car;
    Planet sun("image/planet/sun.jpg", 2);
    Earth earth("image/planet/earth_diffuse.png", 1);
    Moon moon("image/planet/earth_diffuse.png", 1);
    // Bezier bezier;
    bool flag = true;
    //bool flag = false;

    Bezier surface;
    
    while (!glfwWindowShouldClose(window))
    {
        
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

    
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        

        if(flag) {
            // close shot
            //skybox1.Draw();
            //planet.Draw();
            //car.Draw();
            sun.Draw();
            earth.Draw();
            //surface.Draw();

        } else {
            // near shot
            //sun.Draw();
            // earth.Draw();
            //moon.Draw();
        }
        
       
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
}

int main()
{
   
    GLFWwindow* window = Init();
   
    mainLoop(window);

    glfwTerminate();
    return 0;
}


void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);

}


void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    
    glViewport(0, 0, width, height);
}


void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; 

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}


void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}


GLuint loadTexture(GLchar* path, int imagecase)
{
    //Generate texture ID and load texture data 
    GLuint texture1;
    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char *data = stbi_load(path, &width, &height, &nrChannels, 0);
    if (data)
    {
        std::cout << "success to load texture1" << std::endl;
        if(imagecase == 1) {
            // png
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        } else {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    return texture1;
}

GLuint loadCubemap(vector<const GLchar*> faces)
{
    GLuint textureID;
    glGenTextures(1, &textureID);

    int width,height,nrChannels;;
    unsigned char* image;
    
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    std::cout << "[INFO] in loadCubemap()" << std::endl;
    stbi_set_flip_vertically_on_load(true);
    for(GLuint i = 0; i < faces.size(); i++)
    {
        image = stbi_load(faces[i], &width, &height, &nrChannels, 0);
        if(image) {
            std::cout<< "[INFO] success to load image" << std::endl;
        } else {
            std::cout<< "[INFO] faile to load image" << std::endl;
        }
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);


        stbi_image_free(image);
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return textureID;
}
