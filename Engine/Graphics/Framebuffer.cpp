/**
 * @file Framebuffer.cpp
 * @brief Implementation of OpenGL framebuffer abstraction
 * @author Adi (100%)
 *
 * Implements framebuffer creation, binding, and resource management. 
 * Uses OpenGL framebuffer objects with color texture and depth/stencil attachments.
 */

#include "Framebuffer.hpp"
#include "../Core/Logger.hpp"
#include <glad/glad.h>

namespace GP2Engine {

    Framebuffer::~Framebuffer() {
        // Ensure OpenGL resources are cleaned up
        Delete();
    }

    bool Framebuffer::Create(int width, int height) {
        m_width = width;
        m_height = height;

        // Generate and bind framebuffer object
        glGenFramebuffers(1, &m_framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);

        // Create color texture attachment for rendering output
        glGenTextures(1, &m_textureColorBuffer);
        glBindTexture(GL_TEXTURE_2D, m_textureColorBuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        // Set texture filtering
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Clamp to edge to prevent texture wrapping artifacts
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Attach texture to framebuffer color attachment point
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_textureColorBuffer, 0);

        // Create depth/stencil renderbuffer for depth testing support
        glGenRenderbuffers(1, &m_renderbuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, m_renderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

        // Attach renderbuffer to framebuffer depth/stencil attachment point
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_renderbuffer);

        // Verify framebuffer is complete and ready to use
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            LOG_ERROR("Framebuffer is not complete!");
            Delete();
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            return false;
        }

        // Unbind framebuffer and return to default framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        return true;
    }

    void Framebuffer::Resize(int width, int height) {
        if (m_framebuffer == 0) return;

        Delete();
        Create(width, height);
    }

    void Framebuffer::Bind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, m_framebuffer);
        glViewport(0, 0, m_width, m_height);
    }

    void Framebuffer::Unbind() const {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Framebuffer::Delete() {
        if (m_framebuffer != 0) {
            glDeleteFramebuffers(1, &m_framebuffer);
            m_framebuffer = 0;
        }
        if (m_textureColorBuffer != 0) {
            glDeleteTextures(1, &m_textureColorBuffer);
            m_textureColorBuffer = 0;
        }
        if (m_renderbuffer != 0) {
            glDeleteRenderbuffers(1, &m_renderbuffer);
            m_renderbuffer = 0;
        }
    }

} // namespace GP2Engine
