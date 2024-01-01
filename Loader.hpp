#pragma once
//#include <glad/glad.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

class ObjLoader {

public:
    GLuint VAO, VBO;
    int totalNum;

    // 计算 AABB 包围盒
    glm::vec3 minCoords;
    glm::vec3 maxCoords;

public:
    ObjLoader(string file) {
        // obj 文件数据存储
        vector<GLfloat> vertexCoords;   // 存放顶点坐标
        vector<GLfloat> textureCoords;  // 存放纹理坐标
        vector<GLfloat> normalVectors;  // 存放法向量
        vector<GLint> vertexIndex;
        vector<GLint> textureIndex;
        vector<GLint> normalIndex;
        vector<GLfloat> buffer;

        std::ifstream objFile(file);
        std::string str;
        
        // 对 obj 文件进行读取
        while(getline(objFile, str)) {
            // 读取到纹理坐标
            if(str.substr(0, 2) == "vt") {
                GLfloat x, y;
                std::istringstream s(str.substr(3));
                s >> x;
                s >> y;
                textureCoords.push_back(x);
                textureCoords.push_back(y);

            } else if(str.substr(0, 2) == "vn") {
                // 读取法向量
                GLfloat x, y, z;
                std::istringstream s(str.substr(3));
                s >> x;
                s >> y;
                s >> z;
                normalVectors.push_back(x);
                normalVectors.push_back(y);
                normalVectors.push_back(z);
            } else if(str.substr(0, 1) == "v") {
                // 读取点的坐标
                GLfloat x, y, z;
                std::istringstream s(str.substr(2));
                s >> x;
                s >> y;
                s >> z;
                vertexCoords.push_back(x);
                vertexCoords.push_back(y);
                vertexCoords.push_back(z);

            } else if(str.substr(0, 1) == "f") {
                // 读取顶点索引，纹理坐标索引，法向量索引
                std::istringstream indexStr(str.substr(2));
                int V[4] = {}; // 顶点坐标
                int VT[4] = {}; // 纹理坐标
                int VN[4] = {}; // 法向量
                int i;
                string readStr[4] = { "", "", "", ""};
                for(i=0; i<4; ++i) {
                    indexStr >> readStr[i];
                    if(readStr[i] == "") {
                        // 读取结束
                        break;
                    }

                    int split1 = readStr[i].find_first_of('/');
                    int split2 = readStr[i].find_last_of('/');
                    V[i] = atoi(readStr[i].substr(0, split1).c_str()) - 1;
                    VT[i] = atoi(readStr[i].substr(split1+1, split2-split1-1).c_str()) - 1;
                    VN[i] = atoi(readStr[i].substr(split2+1).c_str()) - 1;

                    vertexIndex.push_back(V[i]);
                    textureIndex.push_back(VT[i]);
                    normalIndex.push_back(VN[i]);
                }

                if(i == 4) {
                    vertexIndex.push_back(V[2]);
                    vertexIndex.push_back(V[3]);
                    vertexIndex.push_back(V[0]);
                    textureIndex.push_back(VT[2]);
                    textureIndex.push_back(VT[3]);
                    textureIndex.push_back(VT[0]);
                    normalIndex.push_back(VN[2]);
                    normalIndex.push_back(VN[3]);
                    normalIndex.push_back(VN[0]);
                }
            

            }
        }

        // 文件内容读取结束
        objFile.close();

        minCoords = glm::vec3(std::numeric_limits<float>::max());
        maxCoords = glm::vec3(std::numeric_limits<float>::lowest());


        totalNum = vertexIndex.size();

            for(int i=0; i<totalNum; ++i) {
                // buffer 中添加顶点坐标
                buffer.push_back(vertexCoords[3 * vertexIndex[i] + 0]);
                buffer.push_back(vertexCoords[3 * vertexIndex[i] + 1]);
                buffer.push_back(vertexCoords[3 * vertexIndex[i] + 2]);

                glm::vec3 vertex = glm::vec3(vertexCoords[3 * vertexIndex[i] + 0],
                                             vertexCoords[3 * vertexIndex[i] + 1],
                                             vertexCoords[3 * vertexIndex[i] + 2]);
                // 在遍历的过程中求出最大最小的顶点坐标
                minCoords = glm::min(minCoords, vertex);
                maxCoords = glm::max(maxCoords, vertex);

                // buffer 中添加纹理坐标
                if(textureIndex[i] == -1) {
                    buffer.push_back(0.0f);
                    buffer.push_back(0.0f);
                } else {
                    buffer.push_back(textureCoords[2*textureIndex[i] + 0]);
                    buffer.push_back(1.0f - textureCoords[2*textureIndex[i] + 1]);
                }

               
                buffer.push_back(normalVectors[3 * normalIndex[i] + 0]);
                buffer.push_back(normalVectors[3 * normalIndex[i] + 1]);
                buffer.push_back(normalVectors[3 * normalIndex[i] + 2]);
            }

          
            glGenVertexArrays(1, &VAO);
            glGenBuffers(1, &VBO);
        
            glBindVertexArray(VAO);
            glBindBuffer(GL_ARRAY_BUFFER, VBO);
            glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(GLfloat), buffer.data(), GL_STATIC_DRAW);
           
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
            glEnableVertexAttribArray(0);
         
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(5 * sizeof(float)));
            glEnableVertexAttribArray(2);

        


    }

    void Draw(void) {
        glEnable(GL_DEPTH_TEST);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, totalNum);
    }

};

