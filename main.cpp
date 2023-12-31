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
// #include "ObjLoader.hpp"
#include "Loader.hpp"
#include <math.h>
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
bool AABB_flag = false;



#define FOURCC_DXT1 0x31545844 // Equivalent to "DXT1" in ASCII
#define FOURCC_DXT3 0x33545844 // Equivalent to "DXT3" in ASCII
#define FOURCC_DXT5 0x35545844 // Equivalent to "DXT5" in ASCII

GLuint loadDDS(const char * imagepath)
{
    unsigned char header[124];

    FILE *fp; 

    /* try to open the file */ 
    fp = fopen(imagepath, "rb"); 
    if (fp == NULL)
    {
        printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar(); 
        return 0;
    }

    /* verify the type of file */ 
    char filecode[4]; 
    fread(filecode, 1, 4, fp); 
    if (strncmp(filecode, "DDS ", 4) != 0) 
    {
        fclose(fp); 
        return 0; 
    }

    /* get the surface desc */ 
    fread(&header, 124, 1, fp); 

    unsigned int height      = *(unsigned int*)&(header[8 ]);
    unsigned int width       = *(unsigned int*)&(header[12]);
    unsigned int linearSize  = *(unsigned int*)&(header[16]);
    unsigned int mipMapCount = *(unsigned int*)&(header[24]);
    unsigned int fourCC      = *(unsigned int*)&(header[80]);

    unsigned char * buffer;
    unsigned int bufsize;
    /* how big is it going to be including all mipmaps? */ 
    bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize; 
    buffer = (unsigned char*)malloc(bufsize * sizeof(unsigned char)); 
    fread(buffer, 1, bufsize, fp); 
    /* close the file pointer */ 
    fclose(fp);

    unsigned int components  = (fourCC == FOURCC_DXT1) ? 3 : 4; 
    unsigned int format;
    switch(fourCC) 
    { 
    case FOURCC_DXT1: 
        format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT; 
        break; 
    case FOURCC_DXT3: 
        format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT; 
        break; 
    case FOURCC_DXT5: 
        format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT; 
        break; 
    default: 
        free(buffer); 
        return 0; 
    }

    // Create one OpenGL texture
    GLuint textureID;
    glGenTextures(1, &textureID);

    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, textureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT,1);   

    unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16; 
    unsigned int offset = 0;

    /* load the mipmaps */ 
    for (unsigned int level = 0; level < mipMapCount && (width || height); ++level) 
    { 
        unsigned int size = ((width+3)/4)*((height+3)/4)*blockSize; 
        glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height,  
            0, size, buffer + offset); 

        offset += size; 
        width  /= 2; 
        height /= 2; 

        // Deal with Non-Power-Of-Two textures. This code is not included in the webpage to reduce clutter.
        if(width < 1) width = 1;
        if(height < 1) height = 1;
    } 

    std::cout << "success to load DDS file" << std::endl;

    free(buffer); 
    return textureID;
}


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


/* Planet 父类生成球体的顶点属性 */
class Planet {

public:
    std::vector<float> sphereVertices;  // 顶点属性
    std::vector<int> sphereIndices;     // 顶点索引
    unsigned int VBO, VAO;
    GLuint EBO;
    int PRECISE = 64;

    Planet() {
        float PI = 3.14159;
        // 生成球体的相关点数据
        for(int i=0; i<=PRECISE; ++i) {
            GLfloat pitch = -PI/2 + PI*i/PRECISE;
            for(int j=0; j<=PRECISE; ++j) {
                GLfloat yaw = 2*PI*j / PRECISE;

                /* 下面有 11 个数据需要生成*/

                // 生成顶点坐标 x,y,z 
                GLfloat x = std::cos(pitch) * std::cos(yaw);
                GLfloat y = std::sin(pitch);
                GLfloat z = std::cos(pitch) * -std::sin(yaw); 

                // 生成法向量 nx, ny, nz
                GLfloat nx = std::cos(pitch) * std::cos(yaw);
                GLfloat ny = std::sin(pitch);
                GLfloat nz = std::cos(pitch) * -std::sin(yaw);

                // 生成纹理坐标 ux, uy
                GLfloat ux = (GLfloat)j/PRECISE;
                GLfloat uy = (GLfloat)-i/PRECISE;

                // 生成切线向量 fx, fy, fz
                GLfloat fx = -std::sin(yaw);
                GLfloat fy = 0.0f;
                GLfloat fz = -std::cos(yaw);

                sphereVertices.push_back(x);
                sphereVertices.push_back(y);
                sphereVertices.push_back(z);

                sphereVertices.push_back(nx);
                sphereVertices.push_back(ny);
                sphereVertices.push_back(nz);

                sphereVertices.push_back(ux);
                sphereVertices.push_back(uy);

                sphereVertices.push_back(fx);
                sphereVertices.push_back(fy);
                sphereVertices.push_back(fz);
                
            }
        }


        // 索引生成
        for(int i=0; i<PRECISE; i++) {
            for(int j=0; j<PRECISE; j++) {
                sphereIndices.push_back(i * (PRECISE + 1) + j);
                sphereIndices.push_back((i+1) * (PRECISE + 1) + j);
                sphereIndices.push_back(i * (PRECISE + 1) + j + 1);
                sphereIndices.push_back((i+1) * (PRECISE + 1) + j);
                sphereIndices.push_back(i * (PRECISE + 1) + j + 1);
                sphereIndices.push_back((i+1) * (PRECISE + 1) + j + 1);
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

        // 绑定顶点坐标
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11*sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        // 绑定法向量
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        // 绑定纹理坐标
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11*sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        // 绑定切线向量
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11*sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(3);

        // unbind
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);

    }

    

};

class Sun: Planet {

public:
    Shader sunShader;
    GLuint TextureDiffuse;
    GLuint TextureSpecular;
    GLuint TextureNormal;
    static GLfloat xsun;
    static GLfloat ysun;
    static GLfloat zsun;



    Sun(): sunShader("shaders/earth2.vs", "shaders/earth2.fs") {
        TextureDiffuse = loadDDS("image/planet/molten_02_diffuse.dds");
        TextureNormal = loadDDS("image/planet/molten_02_normal.dds");
        TextureSpecular = loadDDS("image/planet/molten_02_specular.dds");
        // xsun = 0.0f;
        // ysun = 0.0f;
        // zsun = -4.0f;
    }

    void Draw() {
        
        sunShader.use();

        
        sunShader.setVec3("LightInfo.ambient", glm::vec3(1.0f, 1.0f, 1.0f)); 
        sunShader.setVec3("LightInfo.diffuse", glm::vec3(0.5f, 0.5f, 0.5f));
        sunShader.setVec3("LightInfo.specular", glm::vec3(1.0f, 1.0f, 1.0f));
        sunShader.setVec3("viewPos", camera.Position);
        
        // set texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureDiffuse);
        sunShader.setInt("texture_diffuse", 0);

        // set texture
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, TextureNormal);
        sunShader.setInt("texture_normal", 1);

        // set texture
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, TextureSpecular);
        sunShader.setInt("texture_specular", 2);
       
        glm::mat4 view;
        view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view       = glm::translate(view, glm::vec3(0.0f, 0.0f, -4.0f));
        glm::mat4 model = glm::mat4(1.0f);

        GLfloat PI = 3.14159;
        xsun = 0.0f;
        ysun = 0.0f;
        zsun = -10.0f;
        model = glm::translate(model, glm::vec3( xsun,  ysun, zsun));
        
        // 太阳自转
        float angle = (GLfloat)glfwGetTime() * 3.5f;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.3f));

        sunShader.setMat4("projection", projection); 
        sunShader.setMat4("view", view);
        sunShader.setMat4("model", model);

        sunShader.setFloat("LightInfo.constant", 1.0f);
        sunShader.setFloat("LightInfo.linear", 0.19f);
        sunShader.setFloat("LightInfo.quadratic", 0.032f);
        sunShader.setInt("lightNum", 0);

       
        sunShader.setVec3("lightDir", glm::vec3(0.0f, 0.0f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, PRECISE*PRECISE*6, GL_UNSIGNED_INT, 0);
    }


};

GLfloat Sun::xsun = 0.0f;
GLfloat Sun::ysun = 0.0f;
GLfloat Sun::zsun = -10.0f;


class Earth : Planet{
    
public:
    Shader earthShader;
    GLuint TextureDiffuse;
    GLuint TextureSpecular;
    GLuint TextureNormal;
    GLuint TextureDisp;
    static GLfloat xearth;
    static GLfloat yearth;
    static GLfloat zearth;
   

    Earth(): earthShader("newshaders/disp.vs", "newshaders/disp.fs"){
       
        TextureDiffuse = loadDDS("image/planet/earth_diffuse.dds");
        TextureNormal = loadDDS("image/planet/earth_normal.dds");
        TextureSpecular = loadDDS("image/planet/earth_specular.dds");
        TextureDisp = loadTexture("image/planet/earth_disp.jpg",2);  // for jpg


    }

    void Draw() {
        
        earthShader.use();

        
        earthShader.setVec3("LightInfo.ambient", glm::vec3(0.2f, 0.2f, 0.2f)); 
        earthShader.setVec3("LightInfo.diffuse", glm::vec3(1.5f, 1.5f, 1.5f));
        earthShader.setVec3("LightInfo.specular", glm::vec3(1.0f, 1.0f, 1.0f));
        earthShader.setVec3("viewPos", camera.Position);
        
        // set texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureDiffuse);
        earthShader.setInt("texture_diffuse", 0);

        // set texture
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, TextureNormal);
        earthShader.setInt("texture_normal", 1);

        // set texture
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, TextureSpecular);
        earthShader.setInt("texture_specular", 2);

        // set texture
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, TextureDisp);
        earthShader.setInt("texture_disp", 3);
       
        glm::mat4 view;
        view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view       = glm::translate(view, glm::vec3(0.0f, 0.0f, -4.0f));
        glm::mat4 model = glm::mat4(1.0f);


        xearth = Sun::xsun + 4*std::cos((GLfloat)glfwGetTime() * 0.5f);
        yearth = 1.0f;
        zearth = Sun::zsun + 3*std::sin((GLfloat)glfwGetTime() * 0.5f);
        xearth = Sun::xsun + 4*std::cos(3.14159f);
        yearth = 1.0f;
        zearth = Sun::zsun + 3*std::sin(3.14159f);
     
     
    
        model = glm::scale(model, glm::vec3(0.7f, 0.7f, 0.7f));
        model = glm::translate(model, glm::vec3( xearth/0.7f,  yearth/0.7f, zearth/0.7f));
     
        float angle = (GLfloat)glfwGetTime() * 5.5f;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.3f));

        earthShader.setMat4("projection", projection); 
        earthShader.setMat4("view", view);
        earthShader.setMat4("model", model);

        GLfloat lightX = 3*std::cos((GLfloat)glfwGetTime() * 0.1f);
        GLfloat lightZ = 3*std::sin((GLfloat)glfwGetTime() * 0.1f);

        // 设置点光源衰减的系数
        earthShader.setFloat("LightInfo.constant", 1.0f);
        earthShader.setFloat("LightInfo.linear", 0.19f);
        earthShader.setFloat("LightInfo.quadratic", 0.032f);
        earthShader.setInt("lightNum", 2);
        earthShader.setVec3("pointPose[0]", glm::vec3(Sun::xsun, Sun::ysun, Sun::zsun));
        earthShader.setVec3("pointPose[1]", glm::vec3(0.0f, 0.0f, -1.0f));
       
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, PRECISE*PRECISE*6, GL_UNSIGNED_INT, 0);
    }


};
GLfloat Earth::xearth = Sun::xsun + 4*std::cos(3.14159f);
GLfloat Earth::yearth = 1.0f;
GLfloat Earth::zearth = Sun::zsun + 3*std::sin(3.14159f);


class Moon: Planet {
public:
    Shader moonShader;
    GLuint TextureDiffuse;
    GLuint TextureSpecular;
    GLuint TextureNormal;
    static GLfloat xmoon;
    static GLfloat ymoon;
    static GLfloat zmoon;
    GShader moonGshader;

    Moon(): moonShader("newshaders/planet.vs", "newshaders/planet.fs"), moonGshader("shaders/moon.vs","shaders/moon.fs","shaders/moon.gs"){
       
        TextureDiffuse = loadDDS("image/planet/arctic_01_diffuse.dds");
        TextureNormal = loadDDS("image/planet/arctic_01_normal.dds");
        TextureSpecular = loadDDS("image/planet/arctic_01_specular.dds");
       
    }

    void Draw() {
        
        moonShader.use();

        
        moonShader.setVec3("LightInfo.ambient", glm::vec3(0.1f, 0.1f, 0.1f)); 
        moonShader.setVec3("LightInfo.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
        moonShader.setVec3("LightInfo.specular", glm::vec3(1.0f, 1.0f, 1.0f));
        moonShader.setVec3("viewPos", camera.Position);
        
        // set texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureDiffuse);
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
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view       = glm::translate(view, glm::vec3(0.0f, 0.0f, -4.0f));
        glm::mat4 model = glm::mat4(1.0f);

        // float x_earth1 = 0 + 3*std::cos((GLfloat)glfwGetTime() * 0.5f);
        // float z_earth1 = -10 + 3*std::sin((GLfloat)glfwGetTime() * 0.5f);
        xmoon = Earth::xearth + 1*std::cos((GLfloat)glfwGetTime() * 0.5f);
        ymoon = 1.5f;
        zmoon = Earth::zearth + 1*std::sin((GLfloat)glfwGetTime() * 0.5f);
        xmoon = Earth::xearth + 1*std::cos(3.14159f);
        ymoon = 1.5f;
        zmoon = Earth::zearth + 1*std::sin(3.14159f);
     
    
        model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
        model = glm::translate(model, glm::vec3( xmoon/0.3f,  ymoon/0.3f, zmoon/0.3f));
     
        float angle = (GLfloat)glfwGetTime() * 0.5f;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.3f));

        moonShader.setMat4("projection", projection); 
        moonShader.setMat4("view", view);
        moonShader.setMat4("model", model);

        GLfloat lightX = 3*std::cos((GLfloat)glfwGetTime() * 0.1f);
        GLfloat lightZ = 3*std::sin((GLfloat)glfwGetTime() * 0.1f);
        moonShader.setInt("lightNum", 1);
        moonShader.setVec3("pointPose[0]", glm::vec3(Sun::xsun, Sun::ysun, Sun::zsun));

        // 设置点光源衰减的因子
        moonShader.setFloat("LightInfo.constant", 1.0f);
        moonShader.setFloat("LightInfo.linear", 0.19f);
        moonShader.setFloat("LightInfo.quadratic", 0.032f);
       
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, PRECISE*PRECISE*6, GL_UNSIGNED_INT, 0);
    }

    // 使用 GShader 显示爆炸效果
    void Draw2() {

        moonGshader.use();
        moonGshader.setVec3("light.ambient", glm::vec3(0.1f, 0.1f, 0.1f)); 
        moonGshader.setVec3("light.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
        moonGshader.setVec3("light.specular", glm::vec3(1.0f, 1.0f, 1.0f));
        moonGshader.setVec3("viewPos", camera.Position);

         // set texture
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureDiffuse);
        moonGshader.setInt("texture_diffuse", 0);

        // set texture
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, TextureNormal);
        moonGshader.setInt("texture_normal", 1);

        // set texture
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, TextureSpecular);
        moonGshader.setInt("texture_specular", 2);

        glm::mat4 view;
        view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view       = glm::translate(view, glm::vec3(0.0f, 0.0f, -4.0f));
        glm::mat4 model = glm::mat4(1.0f);

        // float x_earth1 = 0 + 3*std::cos((GLfloat)glfwGetTime() * 0.5f);
        // float z_earth1 = -10 + 3*std::sin((GLfloat)glfwGetTime() * 0.5f);
        xmoon = Earth::xearth + 1*std::cos((GLfloat)glfwGetTime() * 0.5f);
        ymoon = 1.5f;
        zmoon = Earth::zearth + 1*std::sin((GLfloat)glfwGetTime() * 0.5f);
        xmoon = Earth::xearth + 1*std::cos(3.14159f);
        ymoon = 1.5f;
        zmoon = Earth::zearth + 1*std::sin(3.14159f);
     
    
        model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));
        model = glm::translate(model, glm::vec3( xmoon/0.3f,  ymoon/0.3f, zmoon/0.3f));
     
        float angle = (GLfloat)glfwGetTime() * 0.5f;
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.3f));

        moonGshader.setMat4("projection", projection); 
        moonGshader.setMat4("view", view);
        moonGshader.setMat4("model", model);

        moonGshader.setVec3("light.direction", glm::vec3(Earth::xearth-xmoon, Earth::yearth-ymoon, Earth::zearth-zmoon));


        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, PRECISE*PRECISE*6, GL_UNSIGNED_INT, 0);




    }




};
GLfloat Moon::xmoon = 0.0f;
GLfloat Moon::ymoon = 0.0f;
GLfloat Moon::zmoon = 0.0f;


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

class ShipYard {
public:
    Shader spaceShader;
    GLuint TextureDiffuse;
    GLuint TextureNormal;
    GLuint TextureSpecular;
    ObjLoader obj;

    ShipYard(): spaceShader("shaders/space.vs", "shaders/space.fs"), obj("MegaShipYard/megashipyard.obj") {
      
        TextureDiffuse = loadDDS("MegaShipYard/megashipyard_diffuse.dds");
        TextureNormal = loadDDS("MegaShipYard/megashipyard_normal.dds");
        TextureSpecular = loadDDS("MegaShipYard/megashipyard_specular.dds");

    }



    void Draw() {
        spaceShader.use();
        spaceShader.setVec3("light.ambient", glm::vec3(0.75f, 0.75f, 0.75f)); 
        spaceShader.setVec3("light.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
        spaceShader.setVec3("light.specular", glm::vec3(0.5f, 0.5f, 0.5f));
        // spaceShader.setVec3("light.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
        spaceShader.setVec3("viewPos", camera.Position);;

        // set textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureDiffuse);
        spaceShader.setInt("texture_diffuse", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, TextureNormal);
        spaceShader.setInt("texture_normal", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, TextureSpecular);
        spaceShader.setInt("texture_specular", 2);

        glm::mat4 view;
        view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view       = glm::translate(view, glm::vec3(0.0f, 0.0f, -4.0f));
        glm::mat4 model = glm::mat4(1.0f);

        
        float angle = (GLfloat)glfwGetTime() * 3.5f;
        model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
        float xspace, yspace, zspace;
        
        xspace = -1.0f;
        yspace = -4.5f;
        zspace = -5.0f;
        model = glm::translate(model, glm::vec3(xspace/0.05f, yspace/0.05f, zspace/0.05f));
       

        spaceShader.setMat4("projection", projection); 
        spaceShader.setMat4("view", view);
        spaceShader.setMat4("model", model);



        spaceShader.setVec3("light.direction", glm::vec3(Earth::xearth-xspace, Earth::yearth-yspace, Earth::zearth-zspace));

        obj.Draw();

    }
    


};


class StarBase {
public:
    Shader spaceShader;
    GLuint TextureDiffuse;
    GLuint TextureNormal;
    GLuint TextureSpecular;
    ObjLoader obj;
    float xspace, yspace, zspace;
      // 默认构造函数
    StarBase(): StarBase(-4.0f, -4.5f, -5.0f) {}


    StarBase(float xspace, float yspace, float zspace):xspace(xspace), yspace(yspace), zspace(zspace),spaceShader("shaders/space.vs", "shaders/space.fs"), obj("starbase/starbase_08.obj") {
        // this->xspace = xspace;
        // this->yspace = yspace;
        // this->zspace = zspace;
        TextureDiffuse = loadDDS("starbase/starbase_08_diffuse.dds");
        TextureNormal = loadDDS("starbase/starbase_08_normal.dds");
        TextureSpecular = loadDDS("starbase/starbase_08_specular.dds");

    }

    

    void Draw() {
        spaceShader.use();
        spaceShader.setVec3("light.ambient", glm::vec3(0.75f, 0.75f, 0.75f)); 
        spaceShader.setVec3("light.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
        spaceShader.setVec3("light.specular", glm::vec3(0.5f, 0.5f, 0.5f));
        // spaceShader.setVec3("light.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
        spaceShader.setVec3("viewPos", camera.Position);;

        // set textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureDiffuse);
        spaceShader.setInt("texture_diffuse", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, TextureNormal);
        spaceShader.setInt("texture_normal", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, TextureSpecular);
        spaceShader.setInt("texture_specular", 2);

        glm::mat4 view;
        view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view       = glm::translate(view, glm::vec3(0.0f, 0.0f, -4.0f));
        glm::mat4 model = glm::mat4(1.0f);

        
        float angle = (GLfloat)glfwGetTime() * 3.5f;
        model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
        
        
        // xspace = -1.0f;
        // yspace = -6.5f;
        // zspace = -5.0f;
        model = glm::translate(model, glm::vec3(xspace/0.05f, yspace/0.05f, zspace/0.05f));
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
       

        spaceShader.setMat4("projection", projection); 
        spaceShader.setMat4("view", view);
        spaceShader.setMat4("model", model);



        spaceShader.setVec3("light.direction", glm::vec3(Earth::xearth-xspace, Earth::yearth-yspace, Earth::zearth-zspace));

        obj.Draw();

    }
    


};

class SpaceStation {
    /* load 空间站的模型 */
public:
    Shader spaceShader;
    GLuint TextureDiffuse;
    GLuint TextureNormal;
    GLuint TextureSpecular;
    ObjLoader obj;

    SpaceStation(): spaceShader("shaders/space.vs", "shaders/space.fs"), obj("starbase/spyorb.obj") {
        std::cout << "load our SpaceStation" << endl;
        TextureDiffuse = loadDDS("starbase/diffuse.dds");
        TextureNormal = loadDDS("starbase/normal.dds");
        TextureSpecular = loadDDS("starbase/specular.dds");
        std::cout << "load is ok" << endl;

    }

    void Draw() {
        spaceShader.use();
        spaceShader.setVec3("light.ambient", glm::vec3(0.05f, 0.05f, 0.05f)); 
        spaceShader.setVec3("light.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
        spaceShader.setVec3("light.specular", glm::vec3(0.5f, 0.5f, 0.5f));
        // spaceShader.setVec3("light.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
        spaceShader.setVec3("viewPos", camera.Position);;

        // set textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureDiffuse);
        spaceShader.setInt("texture_diffuse", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, TextureNormal);
        spaceShader.setInt("texture_normal", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, TextureSpecular);
        spaceShader.setInt("texture_specular", 2);

        glm::mat4 view;
        view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view       = glm::translate(view, glm::vec3(0.0f, 0.0f, -4.0f));
        glm::mat4 model = glm::mat4(1.0f);

        
        float angle = (GLfloat)glfwGetTime() * 3.5f;
        model = glm::scale(model, glm::vec3(0.05f, 0.05f, 0.05f));
        float xspace, yspace, zspace;
        
        xspace = Earth::xearth + 2*std::cos(3.14159f);
        yspace = 2.0f;
        zspace = Earth::zearth + 2*std::sin(3.14159f);
        model = glm::translate(model, glm::vec3(xspace/0.05f, yspace/0.05f, zspace/0.05f));
        model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
       

        spaceShader.setMat4("projection", projection); 
        spaceShader.setMat4("view", view);
        spaceShader.setMat4("model", model);



        spaceShader.setVec3("light.direction", glm::vec3(Earth::xearth-xspace, Earth::yearth-yspace, Earth::zearth-zspace));

        obj.Draw();

    }
    
    

};

// 定位导弹，可以直接追踪UFO
class Missile {
    /* load 空间站的模型 */
public:
    Shader spaceShader;
    GLuint TextureDiffuse;
    GLuint TextureNormal;
    GLuint TextureSpecular;
    ObjLoader obj;
    float xspace, yspace, zspace;

    Missile(): spaceShader("shaders/space.vs", "shaders/space.fs"), obj("missile/missile_01.obj") {
       
        TextureDiffuse = loadDDS("missile/missile_01_diffuse.dds");
        TextureNormal = loadDDS("missile/missile_01_normal.dds");
        TextureSpecular = loadDDS("missile/missile_01_specular.dds");

    }

    void Draw() {
        spaceShader.use();
        spaceShader.setVec3("light.ambient", glm::vec3(0.55f, 0.55f, 0.55f)); 
        spaceShader.setVec3("light.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
        spaceShader.setVec3("light.specular", glm::vec3(0.5f, 0.5f, 0.5f));
        // spaceShader.setVec3("light.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
        spaceShader.setVec3("viewPos", camera.Position);;

        // set textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureDiffuse);
        spaceShader.setInt("texture_diffuse", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, TextureNormal);
        spaceShader.setInt("texture_normal", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, TextureSpecular);
        spaceShader.setInt("texture_specular", 2);

        glm::mat4 view;
        view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view       = glm::translate(view, glm::vec3(0.0f, 0.0f, -4.0f));
        glm::mat4 model = glm::mat4(1.0f);

        
        float angle = (GLfloat)glfwGetTime() * 3.5f;
        model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
        
        
        xspace = -1.0f;
        yspace = -3.0f;
        zspace = -5.0f;
        model = glm::translate(model, glm::vec3(xspace/0.5f, yspace/0.5f, zspace/0.5f));
       // model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
       

        spaceShader.setMat4("projection", projection); 
        spaceShader.setMat4("view", view);
        spaceShader.setMat4("model", model);



        spaceShader.setVec3("light.direction", glm::vec3(Earth::xearth-xspace, Earth::yearth-yspace, Earth::zearth-zspace));

        obj.Draw();

    }
    
    

};

// 存储子弹的移动方向和坐标信息
class Bullet {
public:
    GLfloat x;
    GLfloat y;
    GLfloat z;
    glm::vec3 direction;    // 子弹移动方向
    bool flag;  // 子弹发生一次碰撞后就不会再次移动


    Bullet(GLfloat x, GLfloat y, GLfloat z, glm::vec3 direction): x(x), y(y), z(z), direction(direction){
        flag = true;
    }

};

class BulletManager {
public:
    std::vector<Bullet> bullets;

    void Draw() {
         bullets.erase(std::remove_if(bullets.begin(), bullets.end(), [](const Bullet& b) {
            return !b.flag;
        }), bullets.end());
        for(int i=0; i<bullets.size(); i++) {
            glm::vec3 pos = glm::vec3(bullets[i].x, bullets[i].y, bullets[i].z);
            pos += 0.005f * bullets[i].direction;
            if(bullets[i].flag == true) {
                //std::cout << "run stullet" << std::endl;
                bullets[i].x = pos.x;
                bullets[i].y = pos.y;
                bullets[i].z = pos.z;
            } else {
                bullets[i].x = bullets[i].x;
                bullets[i].y = bullets[i].y;
                bullets[i].z = bullets[i].z;
            }


            glm::mat4 view;
            view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
            glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            view       = glm::translate(view, glm::vec3(0.0f, 0.0f, -4.0f));
            glm::mat4 model = glm::mat4(1.0f);
        }
    }

};

BulletManager bullets;

class Plane {
    /* load 飞行器的模型 */
public:
    Shader spaceShader;
    Shader AABBShader;
    GLuint TextureDiffuse;
    GLuint TextureNormal;
    GLuint TextureSpecular;
    ObjLoader obj;
    GLuint cubeVAO, cubeVBO, cubeEBO;
    glm::vec4 vertices[8];  // 记录 AABBB 包围盒的坐标信息  
    static GLfloat Planex;
    static GLfloat Planey;
    static GLfloat Planez;
    static glm::vec3 direction; //飞行器移动方向
    static GLfloat theta1; // 飞行器绕 y 轴旋转的角度
    static GLfloat theta2; // 飞行器绕 x 轴旋转的角度
    GShader spaceGShader;

    Plane(): spaceShader("newshaders/plane.vs", "newshaders/plane.fs"), obj("plane/plane_04.obj"), 
    AABBShader("shaders/AABB.vs", "shaders/AABB.fs"), spaceGShader("shaders/moon.vs","shaders/moon.fs","shaders/moon.gs") {
        std::cout << "load our SpaceStation" << endl;
        TextureDiffuse = loadDDS("plane/plane_04_diffuse.dds");
        TextureNormal = loadDDS("plane/plane_04_normal.dds");
        TextureSpecular = loadDDS("plane/plane_04_specular.dds");


        GLfloat cubeVertices[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        };

        GLuint cubeIndices[] = {
            0, 1, 1, 2, 2, 3, 3, 0,
            4, 5, 5, 6, 6, 7, 7, 4,
            0, 4, 1, 5, 2, 6, 3, 7
        };

        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        glGenBuffers(1, &cubeEBO);

        glBindVertexArray(cubeVAO);

        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

        // 位置属性
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);

        // 解绑VAO
        glBindVertexArray(0);




        std::cout << "load is ok" << endl;

    }

    void Draw() {
        spaceShader.use();
        spaceShader.setVec3("LightInfo.ambient", glm::vec3(0.55f, 0.55f, 0.55f)); 
        spaceShader.setVec3("LightInfo.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
        spaceShader.setVec3("LightInfo.specular", glm::vec3(0.5f, 0.5f, 0.5f));
        // spaceShader.setVec3("light.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
        spaceShader.setVec3("viewPos", camera.Position);;

        // set textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureDiffuse);
        spaceShader.setInt("texture_diffuse", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, TextureNormal);
        spaceShader.setInt("texture_normal", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, TextureSpecular);
        spaceShader.setInt("texture_specular", 2);

        glm::mat4 view;
        view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view       = glm::translate(view, glm::vec3(0.0f, 0.0f, -2.0f));
        glm::mat4 model = glm::mat4(1.0f);

        
        float angle = (GLfloat)glfwGetTime() * 3.5f;

        model = glm::translate(model, glm::vec3(Planex, Planey, Planez));
        model = glm::rotate(model, glm::radians(theta1), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(theta2), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));

        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(Plane::theta1), glm::vec3(0.0f, 1.0f, 0.0f));
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(Plane::theta2), glm::vec3(1.0f, 0.0f, 0.0f));
        // 将方向向量旋转
        Plane::direction = glm::vec3(rotationMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
       
        
        spaceShader.setMat4("projection", projection); 
        spaceShader.setMat4("view", view);
        spaceShader.setMat4("model", model);
        spaceShader.setInt("lightNum", 1);
        spaceShader.setVec3("PointPos[0]", glm::vec3(Sun::xsun, Sun::ysun, Sun::zsun));

        // 设置点光源衰减的因子
        spaceShader.setFloat("LightInfo.constant", 1.0f);
        spaceShader.setFloat("LightInfo.linear", 0.19f);
        spaceShader.setFloat("LightInfo.quadratic", 0.032f);

        obj.Draw();


        // 绘制 AABB 包围盒
        AABBShader.use();
        // 绘制线框立方体
        glm::mat4 modelAABB = glm::mat4(1.0f);
        
        
        glm::vec3 aabbSize = obj.maxCoords - obj.minCoords;
        glm::vec3 aabbCenter = (obj.maxCoords + obj.minCoords) * 0.5f;
        aabbSize *= 0.15f;
        aabbCenter *= 0.15f;

   
        modelAABB = glm::translate(modelAABB, glm::vec3(Planex, Planey, Planez));
        modelAABB = glm::rotate(modelAABB, glm::radians(theta1), glm::vec3(0.0f, 1.0f, 0.0f));
        modelAABB = glm::rotate(modelAABB, glm::radians(theta2), glm::vec3(1.0f, 0.0f, 0.0f));
        modelAABB = glm::translate(modelAABB, aabbCenter);
        modelAABB = glm::scale(modelAABB, aabbSize);
        

        // 测试 sun 的准确坐标位置
        // modelAABB = glm::translate(modelAABB, glm::vec3(0.0f, 0.0f, Sun::zsun-2.0f));
        // modelAABB = glm::scale(modelAABB, glm::vec3(2.0f, 2.0f, 2.0f));  
        

        
        AABBShader.setMat4("projection", projection);
        AABBShader.setMat4("view", view);
        AABBShader.setMat4("model", modelAABB);

        // 记录包围盒顶点信息
        vertices[0] = modelAABB * glm::vec4(obj.minCoords.x, obj.minCoords.y, obj.minCoords.z, 1.0f);
        vertices[1] = modelAABB * glm::vec4(obj.maxCoords.x, obj.minCoords.y, obj.minCoords.z, 1.0f);
        vertices[2] = modelAABB * glm::vec4(obj.maxCoords.x, obj.maxCoords.y, obj.minCoords.z, 1.0f);
        vertices[3] = modelAABB * glm::vec4(obj.minCoords.x, obj.maxCoords.y, obj.minCoords.z, 1.0f);
        vertices[4] = modelAABB * glm::vec4(obj.minCoords.x, obj.minCoords.y, obj.maxCoords.z, 1.0f);
        vertices[5] = modelAABB * glm::vec4(obj.maxCoords.x, obj.minCoords.y, obj.maxCoords.z, 1.0f);
        vertices[6] = modelAABB * glm::vec4(obj.maxCoords.x, obj.maxCoords.y, obj.maxCoords.z, 1.0f);
        vertices[7] = modelAABB * glm::vec4(obj.minCoords.x, obj.maxCoords.y, obj.maxCoords.z, 1.0f);


        glEnable(GL_DEPTH_TEST);
        glBindVertexArray(cubeVAO);
        if(AABB_flag)
            glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
        //glBindVertexArray(0);



    }

    void Draw2() {

        spaceGShader.use();
        spaceGShader.setVec3("light.ambient", glm::vec3(0.1f, 0.1f, 0.1f)); 
        spaceGShader.setVec3("light.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
        spaceGShader.setVec3("light.specular", glm::vec3(1.0f, 1.0f, 1.0f));
        spaceGShader.setVec3("viewPos", camera.Position);

        // set textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureDiffuse);
        spaceGShader.setInt("texture_diffuse", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, TextureNormal);
        spaceGShader.setInt("texture_normal", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, TextureSpecular);
        spaceGShader.setInt("texture_specular", 2);

        glm::mat4 view;
        view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view       = glm::translate(view, glm::vec3(0.0f, 0.0f, -2.0f));
        glm::mat4 model = glm::mat4(1.0f);

        
        float angle = (GLfloat)glfwGetTime() * 3.5f;

        model = glm::translate(model, glm::vec3(Planex, Planey, Planez));
        model = glm::rotate(model, glm::radians(theta1), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(theta2), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));

        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(Plane::theta1), glm::vec3(0.0f, 1.0f, 0.0f));
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(Plane::theta2), glm::vec3(1.0f, 0.0f, 0.0f));
        // 将方向向量旋转
        Plane::direction = glm::vec3(rotationMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
       
        spaceGShader.setFloat("time", (GLfloat)glfwGetTime());

        spaceGShader.setMat4("projection", projection); 
        spaceGShader.setMat4("view", view);
        spaceGShader.setMat4("model", model);

        spaceGShader.setVec3("light.direction", glm::vec3(Earth::xearth-Planex, Earth::yearth-Planey, Earth::zearth-Planez));

        obj.Draw();



    }

    static void Shoot() {
        // 发射一枚子弹进行射击
        // 初始的位置和方向为飞行器当前的位置和方向
        Bullet bullet(Planex, Planey, Planez, direction);
        bullets.bullets.push_back(bullet);

    }




    
    

};

GLfloat Plane::Planex = -1.0f;
GLfloat Plane::Planey = -4.0f;
GLfloat Plane::Planez = -5.0f;
glm::vec3 Plane::direction = glm::vec3(0.0f, 0.0f, -1.0f);
GLfloat Plane::theta2 = 0.0f;
GLfloat Plane::theta1 = 0.0f;


// 与 stone 模块的逻辑是基本一致的，没有对里面的变量名修改 orz
class UFO {
    /* load UFO 模型*/
    
    public:
    Shader spaceShader;
    Shader AABBShader;
    GLuint TextureDiffuse;
    GLuint TextureNormal;
    GLuint TextureSpecular;
    ObjLoader obj;
    GLuint cubeVAO, cubeVBO, cubeEBO;
    glm::vec4 vertices[8];  // 记录 AABBB 包围盒的坐标信息  
    GLfloat Stonex;
    GLfloat Stoney;
    GLfloat Stonez;
    glm::vec3 direction;    // 陨石移动的方向

    glm::vec3 aabbSize;
    glm::vec3 aabbCenter;

    bool move_flag;
    GShader spaceGShader;
    float timer;
    bool die;


    UFO(): spaceShader("newshaders/plane.vs", "newshaders/plane.fs"), obj("UFO/UFO.obj"), 
    AABBShader("shaders/AABB.vs", "shaders/AABB.fs"), spaceGShader("shaders/moon.vs","shaders/moon.fs","shaders/moon.gs")  {
        timer = 0.0f;
        die = false;
        move_flag = true;   // 根据碰撞检测的结果判定 UFO 是否可以继续移动
        
        Stonex = -2.0f;
        Stoney = 0.0f;
        Stonez = -6.0f;
        TextureDiffuse = loadTexture("UFO/UFO_diffuse.jpg",2);
        TextureNormal = loadTexture("UFO/UFO_normal.jpg",2);
        TextureSpecular = loadTexture("UFO/UFO_specular.jpg",2);

        aabbSize = obj.maxCoords - obj.minCoords;
        aabbCenter = (obj.maxCoords + obj.minCoords) * 0.5f;
        // 对于不同的 scale 需要进行调整
        aabbSize *= 0.005f;
        aabbCenter *= 0.005f;
      


        GLfloat cubeVertices[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        };

        GLuint cubeIndices[] = {
            0, 1, 1, 2, 2, 3, 3, 0,
            4, 5, 5, 6, 6, 7, 7, 4,
            0, 4, 1, 5, 2, 6, 3, 7
        };

        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        glGenBuffers(1, &cubeEBO);

        glBindVertexArray(cubeVAO);

        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

        // 位置属性
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);

        // 解绑VAO
        glBindVertexArray(0);

    }

    void Draw() {
        spaceShader.use();
        spaceShader.setVec3("LightInfo.ambient", glm::vec3(0.55f, 0.55f, 0.55f)); 
        spaceShader.setVec3("LightInfo.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
        spaceShader.setVec3("LightInfo.specular", glm::vec3(0.5f, 0.5f, 0.5f));
        // spaceShader.setVec3("light.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
        spaceShader.setVec3("viewPos", camera.Position);;

        // set textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureDiffuse);
        spaceShader.setInt("texture_diffuse", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, TextureNormal);
        spaceShader.setInt("texture_normal", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, TextureSpecular);
        spaceShader.setInt("texture_specular", 2);

        glm::mat4 view;
        view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view       = glm::translate(view, glm::vec3(0.0f, 0.0f, -2.0f));
        glm::mat4 model = glm::mat4(1.0f);

        
        float angle = (GLfloat)glfwGetTime() * 3.5f;

        // 先把 obj model 的中心平移到 （0，0，0）
        // 经过平移后 （stonex, stoney, stonez）可以代表陨石的中心坐标
        model = glm::translate(model, -aabbCenter);
       model = glm::scale(model, glm::vec3(0.005f, 0.005f, 0.005f));
       model = glm::translate(model, glm::vec3(Stonex/0.005f, Stoney/0.005f, Stonez/0.005f));
    

        // model = glm::rotate(model, glm::radians(theta1), glm::vec3(0.0f, 1.0f, 0.0f));
        // model = glm::rotate(model, glm::radians(theta2), glm::vec3(1.0f, 0.0f, 0.0f));
        
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(Plane::theta1), glm::vec3(0.0f, 1.0f, 0.0f));
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(Plane::theta2), glm::vec3(1.0f, 0.0f, 0.0f));
        // 将方向向量旋转
        Plane::direction = glm::vec3(rotationMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
       
        
        spaceShader.setMat4("projection", projection); 
        spaceShader.setMat4("view", view);
        spaceShader.setMat4("model", model);
        spaceShader.setInt("lightNum", 1);
        spaceShader.setVec3("PointPos[0]", glm::vec3(Sun::xsun, Sun::ysun, Sun::zsun));

        // 设置点光源衰减的因子
        spaceShader.setFloat("LightInfo.constant", 1.0f);
        spaceShader.setFloat("LightInfo.linear", 0.19f);
        spaceShader.setFloat("LightInfo.quadratic", 0.032f);

        obj.Draw();


        // 绘制 AABB 包围盒
        AABBShader.use();
        // 绘制线框立方体
        glm::mat4 modelAABB = glm::mat4(1.0f);
      
   
       // modelAABB = glm::scale(modelAABB, glm::vec3(0.1f,0.1f,0.1f));
        modelAABB = glm::translate(modelAABB, glm::vec3(Stonex, Stoney, Stonez));
        // modelAABB = glm::rotate(modelAABB, glm::radians(theta1), glm::vec3(0.0f, 1.0f, 0.0f));
        // modelAABB = glm::rotate(modelAABB, glm::radians(theta2), glm::vec3(1.0f, 0.0f, 0.0f));
        modelAABB = glm::translate(modelAABB, aabbCenter);  // 注意生成的球体的中心坐标是 （0，0，0）在计算 obj 的坐标时有 aabbCenter 的偏移
        modelAABB = glm::scale(modelAABB, aabbSize);
        //modelAABB = glm::rotate(modelAABB,glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        
        

        
        AABBShader.setMat4("projection", projection);
        AABBShader.setMat4("view", view);
        AABBShader.setMat4("model", modelAABB);

        // 记录包围盒顶点信息
        vertices[0] = modelAABB * glm::vec4(obj.minCoords.x, obj.minCoords.y, obj.minCoords.z, 1.0f);
        vertices[1] = modelAABB * glm::vec4(obj.maxCoords.x, obj.minCoords.y, obj.minCoords.z, 1.0f);
        vertices[2] = modelAABB * glm::vec4(obj.maxCoords.x, obj.maxCoords.y, obj.minCoords.z, 1.0f);
        vertices[3] = modelAABB * glm::vec4(obj.minCoords.x, obj.maxCoords.y, obj.minCoords.z, 1.0f);
        vertices[4] = modelAABB * glm::vec4(obj.minCoords.x, obj.minCoords.y, obj.maxCoords.z, 1.0f);
        vertices[5] = modelAABB * glm::vec4(obj.maxCoords.x, obj.minCoords.y, obj.maxCoords.z, 1.0f);
        vertices[6] = modelAABB * glm::vec4(obj.maxCoords.x, obj.maxCoords.y, obj.maxCoords.z, 1.0f);
        vertices[7] = modelAABB * glm::vec4(obj.minCoords.x, obj.maxCoords.y, obj.maxCoords.z, 1.0f);


        glEnable(GL_DEPTH_TEST);
        glBindVertexArray(cubeVAO);
        if(AABB_flag)
            glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
        //glBindVertexArray(0);



    }

    void Draw2() {

        spaceGShader.use();
        spaceGShader.setVec3("light.ambient", glm::vec3(1.0f, 1.0f, 1.0f)); 
        spaceGShader.setVec3("light.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
        spaceGShader.setVec3("light.specular", glm::vec3(1.0f, 1.0f, 1.0f));
        spaceGShader.setVec3("viewPos", camera.Position);

        // set textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureDiffuse);
        spaceGShader.setInt("texture_diffuse", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, TextureNormal);
        spaceGShader.setInt("texture_normal", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, TextureSpecular);
        spaceGShader.setInt("texture_specular", 2);

        glm::mat4 view;
        view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view       = glm::translate(view, glm::vec3(0.0f, 0.0f, -2.0f));
        glm::mat4 model = glm::mat4(1.0f);

        
        float angle = (GLfloat)glfwGetTime() * 3.5f;

        // 先把 obj model 的中心平移到 （0，0，0）
        // 经过平移后 （stonex, stoney, stonez）可以代表陨石的中心坐标
        model = glm::translate(model, -aabbCenter);
       model = glm::scale(model, glm::vec3(0.005, 0.005, 0.005));
       model = glm::translate(model, glm::vec3(Stonex/0.005, Stoney/0.005, Stonez/0.005));
    

        // model = glm::rotate(model, glm::radians(theta1), glm::vec3(0.0f, 1.0f, 0.0f));
        // model = glm::rotate(model, glm::radians(theta2), glm::vec3(1.0f, 0.0f, 0.0f));
        
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(Plane::theta1), glm::vec3(0.0f, 1.0f, 0.0f));
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(Plane::theta2), glm::vec3(1.0f, 0.0f, 0.0f));
        // 将方向向量旋转
        Plane::direction = glm::vec3(rotationMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
       
        
        if(timer >= 2.5f) {
            die = true;
        } else {
            timer += 0.003f;
        }

        float res = timer >= 1.5f? 1.5f:timer;

        spaceGShader.setFloat("time", (GLfloat)res);

        spaceGShader.setMat4("projection", projection); 
        spaceGShader.setMat4("view", view);
        spaceGShader.setMat4("model", model);

        spaceGShader.setVec3("light.direction", glm::vec3(Earth::xearth-Stonex, Earth::yearth-Stoney, Earth::zearth-Stonez));

        obj.Draw();



    }



};


class UFOManager {
public:
    int num;    // 陨石总数
    std::vector<UFO> UFOs;

    UFOManager() {
        num = 2;

        for(int i=0; i<num; ++i) {
            UFO ufo;

            
            ufo.Stonex = 2.0f + (rand()%10);
            ufo.Stoney = 2.0f + (rand()%10);
            ufo.Stonez = -3.0f + (rand()%10);

            glm::vec3 pos = glm::vec3(ufo.Stonex, ufo.Stoney, ufo.Stonez);
            glm::vec3 direction = pos - pos;    // 初始化为静止的状态，绘制的过程中持续向玩家飞行器进行移动
            ufo.direction = direction;
        
            UFOs.push_back(ufo);
        }



    }

    void Draw() {
        // UFOs.erase(std::remove_if(UFOs.begin(), UFOs.end(), [](const UFO& ufo) {
        //     return ufo.die;
        // }), UFOs.end());

        // if(UFOs.size() < num) {
        //     UFO ufo;
        //     ufo.Stonex = 2.0f + (rand()%10);
        //     ufo.Stoney = 2.0f + (rand()%10);
        //     ufo.Stonez = -3.0f + (rand()%10);

        //     glm::vec3 pos = glm::vec3(ufo.Stonex, ufo.Stoney, ufo.Stonez);
        //     glm::vec3 direction = pos - pos;    // 初始化为静止的状态，绘制的过程中持续向玩家飞行器进行移动
        //     ufo.direction = direction;
        
        //     UFOs.push_back(ufo);

        // }
        for(int i=0; i<UFOs.size(); ++i) {
            if(UFOs[i].die == true) {
                UFOs[i].Stonex = 2.0f + (rand()%20) - 10.0f;
                UFOs[i].Stoney = 2.0f + (rand()%20) - 10.0f;
                UFOs[i].Stonez = -3.0f + (rand()%20) - 10.0f;

                glm::vec3 pos = glm::vec3(UFOs[i].Stonex, UFOs[i].Stoney, UFOs[i].Stonez);
                glm::vec3 direction = pos - pos;    // 初始化为静止的状态，绘制的过程中持续向玩家飞行器进行移动
                UFOs[i].direction = direction;
                UFOs[i].timer = 0.0f;
                UFOs[i].die = false;
                UFOs[i].move_flag = true;
            }
        }

        for(int i=0; i<UFOs.size(); ++i) {
         
            glm::vec3 pos = glm::vec3(UFOs[i].Stonex, UFOs[i].Stoney, UFOs[i].Stonez);
        
           glm::vec3 plane_pos = glm::vec3(Plane::Planex, Plane::Planey, Plane::Planez);
           
            glm::vec3 direction = glm::normalize(plane_pos - pos);
            UFOs[i].direction = direction;
           //glm::vec3 pos = glm::vec3(Stones[i].Stonex, Stones[i].Stoney, Stones[i].Stonez);
            if(UFOs[i].move_flag) {
                pos += 0.002f*UFOs[i].direction;
                UFOs[i].Stonex = pos.x;
                UFOs[i].Stoney = pos.y;
                UFOs[i].Stonez = pos.z;
                UFOs[i].Draw();
            } else {
                //pos -= 0.005f*UFOs[i].direction;
                UFOs[i].Stonex = pos.x;
                UFOs[i].Stoney = pos.y;
                UFOs[i].Stonez = pos.z;
                UFOs[i].Draw2();
            }


            
        }
    }



};




class Stone {
    /* load 陨石的模型 */
public:
    Shader spaceShader;
    Shader AABBShader;
    GLuint TextureDiffuse;
    GLuint TextureNormal;
    GLuint TextureSpecular;
    ObjLoader obj;
    GLuint cubeVAO, cubeVBO, cubeEBO;
    glm::vec4 vertices[8];  // 记录 AABBB 包围盒的坐标信息  
    GLfloat Stonex;
    GLfloat Stoney;
    GLfloat Stonez;
    glm::vec3 direction;    // 陨石移动的方向

    glm::vec3 aabbSize;
    glm::vec3 aabbCenter;

    bool move_flag;
    GShader spaceGShader;

    bool die;
    float timer;  // 显示爆炸效果的计数器


    Stone(): spaceShader("newshaders/plane.vs", "newshaders/plane.fs"), obj("stone/asteroid_05.obj"), 
    AABBShader("shaders/AABB.vs", "shaders/AABB.fs"), spaceGShader("shaders/moon.vs","shaders/moon.fs","shaders/moon.gs") {
        move_flag = true;   // 根据碰撞检测的结果判定陨石是否可以继续移动
        
        timer = 0.0f;
        die = false;

        Stonex = -2.0f;
        Stoney = 0.0f;
        Stonez = -6.0f;
        TextureDiffuse = loadDDS("stone/asteroid_01_diffuse.dds");
        TextureNormal = loadDDS("stone/asteroid_01_normal.dds");
        TextureSpecular = loadDDS("stone/asteroid_01_specular.dds");

        aabbSize = obj.maxCoords - obj.minCoords;
        aabbCenter = (obj.maxCoords + obj.minCoords) * 0.5f;
        // 对于不同的 scale 需要进行调整
        aabbSize *= 0.15f;
        aabbCenter *= 0.15f;
        std::cout << "Center: " << aabbCenter.x << "," << aabbCenter.y << "," << aabbCenter.z << std::endl; 



        GLfloat cubeVertices[] = {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        };

        GLuint cubeIndices[] = {
            0, 1, 1, 2, 2, 3, 3, 0,
            4, 5, 5, 6, 6, 7, 7, 4,
            0, 4, 1, 5, 2, 6, 3, 7
        };

        glGenVertexArrays(1, &cubeVAO);
        glGenBuffers(1, &cubeVBO);
        glGenBuffers(1, &cubeEBO);

        glBindVertexArray(cubeVAO);

        glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

        // 位置属性
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
        glEnableVertexAttribArray(0);

        // 解绑VAO
        glBindVertexArray(0);




        std::cout << "load is ok" << endl;

    }

    void Draw() {
        spaceShader.use();
        spaceShader.setVec3("LightInfo.ambient", glm::vec3(0.55f, 0.55f, 0.55f)); 
        spaceShader.setVec3("LightInfo.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
        spaceShader.setVec3("LightInfo.specular", glm::vec3(0.5f, 0.5f, 0.5f));
        // spaceShader.setVec3("light.direction", glm::vec3(-0.2f, -1.0f, -0.3f));
        spaceShader.setVec3("viewPos", camera.Position);;

        // set textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureDiffuse);
        spaceShader.setInt("texture_diffuse", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, TextureNormal);
        spaceShader.setInt("texture_normal", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, TextureSpecular);
        spaceShader.setInt("texture_specular", 2);

        glm::mat4 view;
        view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view       = glm::translate(view, glm::vec3(0.0f, 0.0f, -2.0f));
        glm::mat4 model = glm::mat4(1.0f);

        
        float angle = (GLfloat)glfwGetTime() * 3.5f;

        // 先把 obj model 的中心平移到 （0，0，0）
        // 经过平移后 （stonex, stoney, stonez）可以代表陨石的中心坐标
        model = glm::translate(model, -aabbCenter);
       model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));
       model = glm::translate(model, glm::vec3(Stonex/0.15f, Stoney/0.15f, Stonez/0.15f));
    

        // model = glm::rotate(model, glm::radians(theta1), glm::vec3(0.0f, 1.0f, 0.0f));
        // model = glm::rotate(model, glm::radians(theta2), glm::vec3(1.0f, 0.0f, 0.0f));
        
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(Plane::theta1), glm::vec3(0.0f, 1.0f, 0.0f));
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(Plane::theta2), glm::vec3(1.0f, 0.0f, 0.0f));
        // 将方向向量旋转
        Plane::direction = glm::vec3(rotationMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
       
        
        spaceShader.setMat4("projection", projection); 
        spaceShader.setMat4("view", view);
        spaceShader.setMat4("model", model);
        spaceShader.setInt("lightNum", 1);
        spaceShader.setVec3("PointPos[0]", glm::vec3(Sun::xsun, Sun::ysun, Sun::zsun));

        // 设置点光源衰减的因子
        spaceShader.setFloat("LightInfo.constant", 1.0f);
        spaceShader.setFloat("LightInfo.linear", 0.19f);
        spaceShader.setFloat("LightInfo.quadratic", 0.032f);

        obj.Draw();


        // 绘制 AABB 包围盒
        AABBShader.use();
        // 绘制线框立方体
        glm::mat4 modelAABB = glm::mat4(1.0f);
      
   
       // modelAABB = glm::scale(modelAABB, glm::vec3(0.1f,0.1f,0.1f));
        modelAABB = glm::translate(modelAABB, glm::vec3(Stonex, Stoney, Stonez));
        // modelAABB = glm::rotate(modelAABB, glm::radians(theta1), glm::vec3(0.0f, 1.0f, 0.0f));
        // modelAABB = glm::rotate(modelAABB, glm::radians(theta2), glm::vec3(1.0f, 0.0f, 0.0f));
        modelAABB = glm::translate(modelAABB, aabbCenter);  // 注意生成的球体的中心坐标是 （0，0，0）在计算 obj 的坐标时有 aabbCenter 的偏移
        modelAABB = glm::scale(modelAABB, aabbSize);
        //modelAABB = glm::rotate(modelAABB,glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
        
        

        
        AABBShader.setMat4("projection", projection);
        AABBShader.setMat4("view", view);
        AABBShader.setMat4("model", modelAABB);

        // 记录包围盒顶点信息
        vertices[0] = modelAABB * glm::vec4(obj.minCoords.x, obj.minCoords.y, obj.minCoords.z, 1.0f);
        vertices[1] = modelAABB * glm::vec4(obj.maxCoords.x, obj.minCoords.y, obj.minCoords.z, 1.0f);
        vertices[2] = modelAABB * glm::vec4(obj.maxCoords.x, obj.maxCoords.y, obj.minCoords.z, 1.0f);
        vertices[3] = modelAABB * glm::vec4(obj.minCoords.x, obj.maxCoords.y, obj.minCoords.z, 1.0f);
        vertices[4] = modelAABB * glm::vec4(obj.minCoords.x, obj.minCoords.y, obj.maxCoords.z, 1.0f);
        vertices[5] = modelAABB * glm::vec4(obj.maxCoords.x, obj.minCoords.y, obj.maxCoords.z, 1.0f);
        vertices[6] = modelAABB * glm::vec4(obj.maxCoords.x, obj.maxCoords.y, obj.maxCoords.z, 1.0f);
        vertices[7] = modelAABB * glm::vec4(obj.minCoords.x, obj.maxCoords.y, obj.maxCoords.z, 1.0f);


        glEnable(GL_DEPTH_TEST);
        glBindVertexArray(cubeVAO);
        if(AABB_flag)
            glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);
        //glBindVertexArray(0);



    }

    void Draw2() {

        spaceGShader.use();
        spaceGShader.setVec3("light.ambient", glm::vec3(1.0f, 1.0f, 1.0f)); 
        spaceGShader.setVec3("light.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
        spaceGShader.setVec3("light.specular", glm::vec3(1.0f, 1.0f, 1.0f));
        spaceGShader.setVec3("viewPos", camera.Position);

        // set textures
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureDiffuse);
        spaceGShader.setInt("texture_diffuse", 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, TextureNormal);
        spaceGShader.setInt("texture_normal", 1);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, TextureSpecular);
        spaceGShader.setInt("texture_specular", 2);

        glm::mat4 view;
        view = glm::lookAt(camera.Position, camera.Position + camera.Front, camera.Up);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view       = glm::translate(view, glm::vec3(0.0f, 0.0f, -2.0f));
        glm::mat4 model = glm::mat4(1.0f);

        
        float angle = (GLfloat)glfwGetTime() * 3.5f;

        // 先把 obj model 的中心平移到 （0，0，0）
        // 经过平移后 （stonex, stoney, stonez）可以代表陨石的中心坐标
        model = glm::translate(model, -aabbCenter);
       model = glm::scale(model, glm::vec3(0.15f, 0.15f, 0.15f));
       model = glm::translate(model, glm::vec3(Stonex/0.15f, Stoney/0.15f, Stonez/0.15f));
    

        // model = glm::rotate(model, glm::radians(theta1), glm::vec3(0.0f, 1.0f, 0.0f));
        // model = glm::rotate(model, glm::radians(theta2), glm::vec3(1.0f, 0.0f, 0.0f));
        
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(Plane::theta1), glm::vec3(0.0f, 1.0f, 0.0f));
        rotationMatrix = glm::rotate(rotationMatrix, glm::radians(Plane::theta2), glm::vec3(1.0f, 0.0f, 0.0f));
        // 将方向向量旋转
        Plane::direction = glm::vec3(rotationMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
       
        
        if(timer >= 3.0f) {
            die = true;
        } else {
            timer += 0.003f;
        }

        float res = timer>=1.5f? 1.5f: timer;
        spaceGShader.setFloat("time", (GLfloat)res);

        spaceGShader.setMat4("projection", projection); 
        spaceGShader.setMat4("view", view);
        spaceGShader.setMat4("model", model);

        spaceGShader.setVec3("light.direction", glm::vec3(Earth::xearth-Stonex, Earth::yearth-Stoney, Earth::zearth-Stonez));

        if(die == false) {
            obj.Draw();
        }

    }




    
    

};

// 对多个陨石进行管理
// 陨石被击中则将其删除不再显示
class StoneManager {
public:
    int num;    // 陨石总数
    std::vector<Stone> Stones;

    StoneManager() {
        num = 4;

        for(int i=0; i<4; ++i) {
            Stone newStone;
            // 对陨石生成不同的位置
            std::cout << "stone :" << i << std::endl;
            
            newStone.Stonex = Earth::xearth + (rand()%16) - 8.0f;
            newStone.Stoney = Earth::yearth + (rand()%16) - 8.0f;
            newStone.Stonez = Earth::zearth + (rand()%16) - 8.0f;

            // 陨石初始的运动方向指向当前地球的位置
            // 随着地球的移动后续会对方向进行调整
            glm::vec3 pos = glm::vec3(newStone.Stonex, newStone.Stoney, newStone.Stonez);
            glm::vec3 earthPos = glm::vec3(Earth::xearth, Earth::yearth, Earth::zearth-2.0f);
            glm::vec3 direction = earthPos - pos;
            newStone.direction = direction;
            

            Stones.push_back(newStone);
        }



    }

    void Draw() {

        Stones.erase(std::remove_if(Stones.begin(), Stones.end(), [](const Stone& s) {
            return s.die;
        }), Stones.end());

        while(Stones.size() < 4) {
            Stone newStone;
            // 对陨石生成不同的位置
            
            newStone.Stonex = -2.0f + (rand()%8) - 4.0f;
            newStone.Stoney = 0.0f + (rand()%4) - 1.0f;
            newStone.Stonez = -8.0f + (rand()%8) - 4.0f;

            // 陨石初始的运动方向指向当前地球的位置
            // 随着地球的移动后续会对方向进行调整
            glm::vec3 pos = glm::vec3(newStone.Stonex, newStone.Stoney, newStone.Stonez);
            glm::vec3 earthPos = glm::vec3(Earth::xearth, Earth::yearth, Earth::zearth-2.0f);
            glm::vec3 direction = earthPos - pos;
            newStone.direction = direction;
            Stones.push_back(newStone);

        }

        for(int i=0; i<Stones.size(); ++i) {
            // 每次绘制时将陨石按照响应的方向进行移动
            // 每间隔一段时间对方向进行调整
            glm::vec3 pos = glm::vec3(Stones[i].Stonex, Stones[i].Stoney, Stones[i].Stonez);
            
            // 地球的坐标是 (x, y, z-2)
           glm::vec3 earthPos = glm::vec3(Earth::xearth, Earth::yearth, Earth::zearth-2.0f);
           
            glm::vec3 direction = glm::normalize(earthPos - pos);
            Stones[i].direction = direction;
           //glm::vec3 pos = glm::vec3(Stones[i].Stonex, Stones[i].Stoney, Stones[i].Stonez);
            if(Stones[i].move_flag) {
                // 未检测到碰撞可以向前移动
                pos += 0.001f*Stones[i].direction;
                Stones[i].Stonex = pos.x;
                Stones[i].Stoney = pos.y;
                Stones[i].Stonez = pos.z;
                Stones[i].Draw();
            } else {
                //pos -= 0.005f*Stones[i].direction;
                Stones[i].Stonex = pos.x;
                Stones[i].Stoney = pos.y;
                Stones[i].Stonez = pos.z;
                // 发生碰撞，显示爆炸的效果
                Stones[i].Draw2();
            }


            
        }
    }



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

    SkyBox():skyboxShader("shaders/skybox.vs", "shaders/skybox.fs") {};

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



bool gameFlag = true;
bool plane_flag = true; // 如果发生的碰撞，飞行器不可以向前移动    
bool missile_flag = false;


// 管理游戏场景中的各个部件
// 管理天空场景
class ObjectManager {
    public:
        SkyBox skybox1;
        Sun sun;
        Earth earth;
        Moon moon;
        SpaceStation space;
        Plane plane;
        StoneManager stones;
        UFOManager ufos;
        ShipYard yard;
        Missile missile;
        StarBase starBase;
        StarBase starbase2;
   

        ObjectManager() {
            vector<const GLchar*> faces;
            starbase2.xspace = 2.0f;
            starbase2.yspace = -4.5f;
            starbase2.zspace = -5.0f;

            faces.push_back("image/skybox/1.png");
            faces.push_back("image/skybox/2.png");
            faces.push_back("image/skybox/4.png");
            faces.push_back("image/skybox/3.png");
            faces.push_back("image/skybox/5.png");
            faces.push_back("image/skybox/6.png");

            skybox1 = SkyBox(faces);
            // sun = Sun();
            // earth = Earth();
            // moon = Moon();
            //space = SpaceStation();
            //plane = Plane();
            // stones = StoneManager();
            // ufos = UFOManager();
        }

        // 进行碰撞检测
        void ImpactCheck() {
            // 确定太阳的坐标信息
            glm::vec3 sun_coord(Sun::xsun, Sun::ysun, Sun::zsun-2.0f);
            GLfloat r_sun = 1.0f;

            // 确定地球的坐标信息
            glm::vec3 earth_coord(Earth::xearth, Earth::yearth, Earth::zearth-2.0f);
            GLfloat r_earth = 0.7f;
            
            // 确定月球的坐标信息
            glm::vec3 moon_coord(Moon::xmoon, Moon::ymoon, Moon::zmoon-2.0f);
            GLfloat r_moon = 0.3f;

            // 确定飞行器的 AABB 包围盒的坐标
            glm::vec4 plane_vert[8];
            for(int i=0; i<8; ++i) {
                plane_vert[i] = plane.vertices[i];
            }

            bool check_sun = ImpactChecker1(sun_coord, r_sun, plane_vert);
            bool check_earth = ImpactChecker1(earth_coord, r_earth, plane_vert);
            bool check_moon = ImpactChecker1(moon_coord, r_moon, plane_vert);


            // 如果检测到了碰撞则不允许飞行器前向移动
            if(check_sun || check_earth || check_moon) {
                plane_flag = false;
            } else {
              
                plane_flag = true;
            }

            // 判断 ufo 是否被子弹射中
            for(int i=0; i<ufos.UFOs.size(); ++i) {
                if(ufos.UFOs[i].move_flag == false) {
                    continue;
                }

                glm::vec4 ufo_vert[8];
                for(int j=0; j<8; ++j) {
                    ufo_vert[j] = ufos.UFOs[i].vertices[j];
                }

                // 判断陨石是否和子弹碰撞
                bool check_bullet = false;

                for(int j=0; j<bullets.bullets.size(); j++) {
                    bool check = CheckBullet2(ufo_vert, glm::vec3(bullets.bullets[j].x, bullets.bullets[j].y, bullets.bullets[j].z), bullets.bullets[j].direction);
                    if(check == true && bullets.bullets[j].flag == true) {
                        check_bullet = true;
                        bullets.bullets[j].flag = false;
                        break;
                    }
                }

                if(check_bullet == true || ufos.UFOs[i].move_flag == false) {
                    ufos.UFOs[i].move_flag = false;
                } else {
                    ufos.UFOs[i].move_flag = true;
                }
            }


            // 进行陨石和地球的碰撞检测
            // 依然是 AABB 与球体的碰撞检测
            for(int i=0; i<stones.Stones.size(); ++i) {
                // 已经被碰撞的陨石不必再次进行检测
                if(stones.Stones[i].move_flag == false) {
                    continue;
                }
                // 获取第 i 个陨石块的 AABB 坐标信息
                glm::vec4 stone_vert[8];    // bounding box 的八个顶点坐标
                for(int j=0; j<8; ++j) {
                    stone_vert[j] = stones.Stones[i].vertices[j];
                }

                bool check_stone = ImpactChecker2(earth_coord, r_earth, stone_vert);

                bool check_bullet = false;
                // 判断陨石是否和子弹发生碰撞
                for(int j=0; j<bullets.bullets.size(); j++) {
                  
                    bool check = CheckBullet(stone_vert, glm::vec3(bullets.bullets[j].x, bullets.bullets[j].y, bullets.bullets[j].z), bullets.bullets[j].direction); 
                    std::cout << "bump num i = " << i << std::endl;
                    //bool check = CheckBullet(stone_vert, glm::vec3(Plane::Planex, Plane::Planey, Plane::Planez), Plane::direction); 
                    if(check == true && bullets.bullets[j].flag == true) {
                        
                        std::cout << "bullet j = " << j << std::endl;
                        check_bullet = true;
                        bullets.bullets[j].flag = false;
                        break;
                    }

                }

                if(check_stone || check_bullet || stones.Stones[i].move_flag == false) {
                   
                    if(stones.Stones[i].move_flag == true) {
                         std::cout << "impact happend" << std::endl;
                        std::cout << "stone i= " << i << std::endl;
                    }
                    stones.Stones[i].move_flag = false;
                } else {
                    stones.Stones[i].move_flag = true;
                }

            }

            
        }

        bool CheckBullet(glm::vec4 vertices[8], glm::vec3 coord, glm::vec3 direction) {

            glm::vec3 minCoord = glm::vec3(vertices[0]);
            glm::vec3 maxCoord = glm::vec3(vertices[0]);

            for (int i = 1; i < 8; ++i) {
                for (int j = 0; j < 3; ++j) {
                    minCoord[j] = std::min(minCoord[j], vertices[i][j]);
                    maxCoord[j] = std::max(maxCoord[j], vertices[i][j]);
                }
            }

            glm::vec3 avgCoord = glm::vec3((minCoord[0]+maxCoord[0])/2, (minCoord[1]+maxCoord[1])/2, (minCoord[2]+maxCoord[2])/2);

            glm::vec3 shootdir = avgCoord - coord;
            
            float length1 = sqrt(shootdir[0]*shootdir[0] + shootdir[1]*shootdir[1] + shootdir[2]*shootdir[2]);
            float length2 = sqrt(direction[0]*direction[0] + direction[1]*direction[1] + direction[2]*direction[2]);
            
            if((shootdir[0]*direction[0] + shootdir[1]*direction[1] + shootdir[2]*direction[2]) / (length1*length2) >= 0.08f) {
                std::cout << "res: " << (shootdir[0]*direction[0] + shootdir[1]*direction[1] + shootdir[2]*direction[2]) / (length1*length2) << std::endl;
                return true;
            } else {
                std::cout << "res error: " << (shootdir[0]*direction[0] + shootdir[1]*direction[1] + shootdir[2]*direction[2]) / (length1*length2) << std::endl;
                return false;
            }

        
        }


        bool CheckBullet2(glm::vec4 vertices[8], glm::vec3 coord, glm::vec3 direction) {

            glm::vec3 minCoord = glm::vec3(vertices[0]);
            glm::vec3 maxCoord = glm::vec3(vertices[0]);

            for (int i = 1; i < 8; ++i) {
                for (int j = 0; j < 3; ++j) {
                    minCoord[j] = std::min(minCoord[j], vertices[i][j]);
                    maxCoord[j] = std::max(maxCoord[j], vertices[i][j]);
                }
            }

            glm::vec3 avgCoord = glm::vec3((minCoord[0]+maxCoord[0])/2, (minCoord[1]+maxCoord[1])/2, (minCoord[2]+maxCoord[2])/2);

            glm::vec3 shootdir = avgCoord - coord;
            
            float length1 = sqrt(shootdir[0]*shootdir[0] + shootdir[1]*shootdir[1] + shootdir[2]*shootdir[2]);
            float length2 = sqrt(direction[0]*direction[0] + direction[1]*direction[1] + direction[2]*direction[2]);
            
            if((shootdir[0]*direction[0] + shootdir[1]*direction[1] + shootdir[2]*direction[2]) / (length1*length2) >= 0.08f) {
                std::cout << "res: " << (shootdir[0]*direction[0] + shootdir[1]*direction[1] + shootdir[2]*direction[2]) / (length1*length2) << std::endl;
                return true;
            } else {
                std::cout << "res error: " << (shootdir[0]*direction[0] + shootdir[1]*direction[1] + shootdir[2]*direction[2]) / (length1*length2) << std::endl;
                return false;
            }

        
        }

        // 进行球体和 AABB 包围盒的碰撞检测
        bool ImpactChecker1(glm::vec3 coord, GLfloat r, glm::vec4 vertices[8]) {


            glm::vec3 minCoord = glm::vec3(vertices[0]);
            glm::vec3 maxCoord = glm::vec3(vertices[0]);

            for (int i = 1; i < 8; ++i) {
                for (int j = 0; j < 3; ++j) {
                    minCoord[j] = std::min(minCoord[j], vertices[i][j]);
                    maxCoord[j] = std::max(maxCoord[j], vertices[i][j]);
                }
            }

            // 找到 AABB 中与球心最近的一个点
            GLfloat x_near = coord.x;
            x_near = x_near > maxCoord.x ? maxCoord.x : x_near;
            x_near = x_near < minCoord.x ? minCoord.x : x_near;

            GLfloat y_near = coord.y;
            y_near = y_near > maxCoord.y ? maxCoord.y : y_near;
            y_near = y_near < minCoord.y ? minCoord.y : y_near;

            GLfloat z_near = coord.z;
            z_near = z_near > maxCoord.z ? maxCoord.z : z_near;
            z_near = z_near < minCoord.z ? minCoord.z : z_near;

            GLfloat distance = (coord.x-x_near) * (coord.x-x_near) + 
                                (coord.y-y_near) * (coord.y-y_near) +
                                (coord.z-z_near) * (coord.z-z_near);

            if(distance <= r*r ) {
                //std::cout << "bump dis: " << distance << std::endl;
                return true;
            } else {
                //std::cout << "normal dis: " << distance << std::endl;
                return false;
            }

        }


         bool ImpactChecker2(glm::vec3 coord, GLfloat r, glm::vec4 vertices[8]) {

            // 若球心不在 AABB 内部，首先求出球心到这 8 个点的最短距离
            // 只有距离小于球体半径时才会发生碰撞
            GLfloat mindis = 4*r*r;

            for(int i=0; i<8; ++i) {
                GLfloat distance = (coord.x-vertices[i].x) * (coord.x-vertices[i].x) +
                                   (coord.y-vertices[i].y) * (coord.y-vertices[i].y) +
                                   (coord.z-vertices[i].z) * (coord.z-vertices[i].z);

                if(distance <= mindis) {
                    mindis = distance;
                }
            }
           
            // 判定发生碰撞
            if(mindis <= r*r) {
               
                return true;
            }

            return false;

        }


    bool IsSphereInsideAABB(glm::vec3 sphereCenter, glm::vec3 aabbMin, glm::vec3 aabbMax) {
            return (sphereCenter.x >= aabbMin.x && sphereCenter.x <= aabbMax.x &&
                    sphereCenter.y >= aabbMin.y && sphereCenter.y <= aabbMax.y &&
                    sphereCenter.z >= aabbMin.z && sphereCenter.z <= aabbMax.z);
        }



};



void mainLoop(GLFWwindow* window ) {


    ObjectManager gameObj;
    while (!glfwWindowShouldClose(window))
    {
        
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

    
        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        gameObj.ImpactCheck();

        if(gameFlag) {
           
            
            gameObj.skybox1.Draw();
         
            gameObj.plane.Draw();
            gameObj.sun.Draw();
            gameObj.moon.Draw();
            gameObj.earth.Draw();
            gameObj.stones.Draw();
            gameObj.ufos.Draw();
            bullets.Draw();
            gameObj.space.Draw();
            gameObj.yard.Draw();
            //gameObj.missile.Draw();
            gameObj.starBase.Draw();
            gameObj.starbase2.Draw();
        

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

bool keyIsPressed = false;
bool isShoot = false;
int controlnum = 0;




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
    
    // 增加 plane 移动的键盘控制
    if(glfwGetKey(window, GLFW_KEY_J)){
        // 逆时针旋转方向
        Plane::theta1 += 0.5f;
    }

    if(glfwGetKey(window, GLFW_KEY_L)) {
        // 顺时针旋转方向
        Plane::theta1 -= 0.5f;
        
    }

    if(glfwGetKey(window, GLFW_KEY_U)){
        // 逆时针旋转方向
        Plane::theta2 += 0.5f;
    }

    if(glfwGetKey(window, GLFW_KEY_O)) {
        // 顺时针旋转方向
        Plane::theta2 -= 0.5f;

        // 构造绕 y 轴逆时针旋转 theta1 角度的旋转矩阵
        glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(Plane::theta1), glm::vec3(0.0f, 1.0f, 0.0f));

        // 将方向向量旋转
        Plane::direction = glm::vec3(rotationMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
        
    }


    if(glfwGetKey(window, GLFW_KEY_I)) {
        // 向前移动
        glm::vec3 pos = glm::vec3(Plane::Planex, Plane::Planey, Plane::Planez);
       
        if(plane_flag) {
            pos += 0.01f * Plane::direction;
            Plane::Planex = pos.x;
            Plane::Planey = pos.y;
            Plane::Planez = pos.z;
        } else {
            pos -= 0.08f * Plane::direction;
            Plane::Planex = pos.x;
            Plane::Planey = pos.y;
            Plane::Planez = pos.z;
        }
    }


    if(glfwGetKey(window, GLFW_KEY_K)) {
        // 向后移动
        glm::vec3 pos = glm::vec3(Plane::Planex, Plane::Planey, Plane::Planez);
        pos -= 0.01f * Plane::direction;
        Plane::Planex = pos.x;
        Plane::Planey = pos.y;
        Plane::Planez = pos.z;
    }

    if(glfwGetKey(window, GLFW_KEY_1)) {
        if(controlnum < 1) {
            controlnum++;
            std::cout << "shoot " << std::endl;
            Plane::Shoot();
        } 
    }

    if(glfwGetKey(window, GLFW_KEY_2)) {
        if(controlnum >= 1) {
            controlnum--;
        }
    }

    if(glfwGetKey(window, GLFW_KEY_3)) {
        AABB_flag = true;
    }

    if(glfwGetKey(window, GLFW_KEY_4)) {
        AABB_flag = false;
    }
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
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
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
