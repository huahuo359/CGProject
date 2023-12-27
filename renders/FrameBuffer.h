#pragma once

#include <glad/glad.h>
#include <cstdint>

class FrameBuffer {
  private:
    GLuint framebufferID;
    GLuint depthTexture;

    GLuint width;
    GLuint height;

  public:
    FrameBuffer(uint32_t width, uint32_t height);
    void addDepthTexture();
    bool isOkay();

    inline GLuint getDepthTexture() const {return depthTexture; };
    inline GLuint getDepthBuffer() const {return framebufferID; };

    inline int getWidth() const {return width; };
    inline int getHeight() const {return height; };

    virtual void bind();
    virtual void unbind();
};
