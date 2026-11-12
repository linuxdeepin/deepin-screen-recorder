// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-License-Identifier: GPL-3.0-or-later

#include "dmaframegpuconverter.h"

#include "../utils/log.h"

#include <QDebug>
#include <QVarLengthArray>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <drm/drm_fourcc.h>
#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>
#include <GL/glext.h>
#include <gbm.h>

#include <cstring>

namespace {

QString eglErrorString(const char *stage)
{
    return QStringLiteral("%1 failed, EGL error: 0x%2")
            .arg(QString::fromLatin1(stage))
            .arg(static_cast<unsigned>(eglGetError()), 0, 16);
}

bool hasExtension(const char *extensions, const char *name)
{
    if (!extensions || !name)
        return false;
    return std::strstr(extensions, name) != nullptr;
}

} // namespace

struct DmaFrameGpuConverter::Private
{
    explicit Private(gbm_device *gbmDevice)
        : device(gbmDevice)
    {
    }

    bool ensureInitialized(QString *error)
    {
        if (initialized)
            return true;
        if (!device) {
            if (error)
                *error = QStringLiteral("DMA-BUF GPU 转换缺少 GBM 设备");
            return false;
        }

        display = eglGetPlatformDisplay(EGL_PLATFORM_GBM_KHR, device, nullptr);
        if (display == EGL_NO_DISPLAY) {
            if (error)
                *error = eglErrorString("eglGetPlatformDisplay");
            return false;
        }

        EGLint major = 0;
        EGLint minor = 0;
        if (!eglInitialize(display, &major, &minor)) {
            if (error)
                *error = eglErrorString("eglInitialize");
            display = EGL_NO_DISPLAY;
            return false;
        }

        const char *extensions = eglQueryString(display, EGL_EXTENSIONS);
        if (!hasExtension(extensions, "EGL_EXT_image_dma_buf_import")) {
            if (error)
                *error = QStringLiteral("EGL 不支持 DMA-BUF 导入");
            return false;
        }
        modifiersSupported = hasExtension(
                extensions, "EGL_EXT_image_dma_buf_import_modifiers");

        if (!eglBindAPI(EGL_OPENGL_API)) {
            if (error)
                *error = eglErrorString("eglBindAPI");
            return false;
        }

        // Mesa 的 GBM 平台通常只暴露 window/pbuffer 之外的 config，甚至
        // 完全没有 EGL_PBUFFER_BIT。这里创建 surfaceless context，最终
        // 渲染到应用自己的 FBO，不依赖平台窗口或 pbuffer 支持。
        const EGLint configAttrs[] = {
            EGL_RENDERABLE_TYPE, EGL_OPENGL_BIT,
            EGL_RED_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_BLUE_SIZE, 8,
            EGL_ALPHA_SIZE, 8,
            EGL_NONE
        };
        EGLint configCount = 0;
        if (!eglChooseConfig(display, configAttrs, &config, 1, &configCount)
                || configCount < 1) {
            if (error)
                *error = eglErrorString("eglChooseConfig");
            return false;
        }

        const EGLint contextAttrs[] = {
            EGL_CONTEXT_MAJOR_VERSION, 2,
            EGL_CONTEXT_MINOR_VERSION, 1,
            EGL_NONE
        };
        context = eglCreateContext(display, config, EGL_NO_CONTEXT, contextAttrs);
        if (context == EGL_NO_CONTEXT) {
            if (error)
                *error = eglErrorString("eglCreateContext");
            return false;
        }

        targetTexture = reinterpret_cast<PFNGLEGLIMAGETARGETTEXTURE2DOESPROC>(
                eglGetProcAddress("glEGLImageTargetTexture2DOES"));
        if (!targetTexture) {
            if (error)
                *error = QStringLiteral("缺少 glEGLImageTargetTexture2DOES");
            return false;
        }

        initialized = true;
        qCWarning(dsrApp) << "DmaFrameGpuConverter initialized, EGL:" << major << minor
                          << "extensions:" << extensions;
        return true;
    }

    bool ensureCurrent(int width, int height, QString *error)
    {
        if (!ensureInitialized(error))
            return false;

        if (!eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, context)) {
            if (error)
                *error = eglErrorString("eglMakeCurrent(surfaceless)");
            return false;
        }

        if (fbo == 0 || fboWidth != width || fboHeight != height) {
            if (fbo != 0)
                glDeleteFramebuffers(1, &fbo);
            if (fboTexture != 0)
                glDeleteTextures(1, &fboTexture);
            fbo = 0;
            fboTexture = 0;

            glGenTextures(1, &fboTexture);
            glBindTexture(GL_TEXTURE_2D, fboTexture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0,
                         GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

            glGenFramebuffers(1, &fbo);
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D, fboTexture, 0);
            const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (status != GL_FRAMEBUFFER_COMPLETE) {
                if (error)
                    *error = QStringLiteral("DMA-BUF GPU FBO 不完整：0x%1")
                                     .arg(static_cast<unsigned>(status), 0, 16);
                return false;
            }
            fboWidth = width;
            fboHeight = height;
        } else {
            glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        }
        return true;
    }

    EGLImage createImage(int fd, int width, int height, int stride, uint32_t offset,
                         uint32_t format, uint64_t modifier, QString *error)
    {
        if (modifier != DRM_FORMAT_MOD_INVALID && !modifiersSupported) {
            if (error)
                *error = QStringLiteral("EGL 不支持显式 DMA-BUF modifier 导入");
            return EGL_NO_IMAGE;
        }

        QVarLengthArray<EGLAttrib, 16> attrs;
        attrs.append(EGL_WIDTH);
        attrs.append(width);
        attrs.append(EGL_HEIGHT);
        attrs.append(height);
        attrs.append(EGL_LINUX_DRM_FOURCC_EXT);
        attrs.append(static_cast<EGLAttrib>(format));
        attrs.append(EGL_DMA_BUF_PLANE0_FD_EXT);
        attrs.append(static_cast<EGLAttrib>(fd));
        attrs.append(EGL_DMA_BUF_PLANE0_OFFSET_EXT);
        attrs.append(static_cast<EGLAttrib>(offset));
        attrs.append(EGL_DMA_BUF_PLANE0_PITCH_EXT);
        attrs.append(static_cast<EGLAttrib>(stride));
        if (modifier != DRM_FORMAT_MOD_INVALID) {
            attrs.append(EGL_DMA_BUF_PLANE0_MODIFIER_LO_EXT);
            attrs.append(static_cast<EGLAttrib>(modifier & 0xffffffffULL));
            attrs.append(EGL_DMA_BUF_PLANE0_MODIFIER_HI_EXT);
            attrs.append(static_cast<EGLAttrib>(modifier >> 32));
        }
        attrs.append(EGL_NONE);
        EGLImage image = eglCreateImage(display, EGL_NO_CONTEXT,
                                        EGL_LINUX_DMA_BUF_EXT, nullptr, attrs.constData());
        if (image == EGL_NO_IMAGE) {
            if (error)
                *error = eglErrorString("eglCreateImage");
        }
        return image;
    }

    bool importTexture(int fd, int width, int height, int stride, uint32_t offset,
                       uint32_t format, uint64_t modifier, GLuint *texture,
                       QString *error)
    {
        if (!ensureCurrent(qMax(1, width), qMax(1, height), error))
            return false;

        const EGLImage image = createImage(fd, width, height, stride, offset,
                                           format, modifier, error);
        if (image == EGL_NO_IMAGE)
            return false;

        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        targetTexture(GL_TEXTURE_2D, image);
        const GLenum glError = glGetError();
        eglDestroyImage(display, image);
        if (glError != GL_NO_ERROR) {
            glDeleteTextures(1, &tex);
            if (error)
                *error = QStringLiteral("EGLImage 绑定纹理失败，GL error: 0x%1")
                                 .arg(static_cast<unsigned>(glError), 0, 16);
            return false;
        }

        *texture = tex;
        return true;
    }

    void destroy(GLuint texture)
    {
        if (texture != 0)
            glDeleteTextures(1, &texture);
    }

    void release()
    {
        if (display != EGL_NO_DISPLAY) {
            if (context != EGL_NO_CONTEXT
                    && eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE,
                                      context)) {
                if (fbo != 0)
                    glDeleteFramebuffers(1, &fbo);
                if (fboTexture != 0)
                    glDeleteTextures(1, &fboTexture);
            }
            eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
            if (context != EGL_NO_CONTEXT)
                eglDestroyContext(display, context);
            eglTerminate(display);
        }
        display = EGL_NO_DISPLAY;
        context = EGL_NO_CONTEXT;
        fbo = 0;
        fboTexture = 0;
        fboWidth = 0;
        fboHeight = 0;
        targetTexture = nullptr;
        initialized = false;
    }

    gbm_device *device = nullptr;
    EGLDisplay display = EGL_NO_DISPLAY;
    EGLContext context = EGL_NO_CONTEXT;
    EGLConfig config = nullptr;
    PFNGLEGLIMAGETARGETTEXTURE2DOESPROC targetTexture = nullptr;
    GLuint fbo = 0;
    GLuint fboTexture = 0;
    int fboWidth = 0;
    int fboHeight = 0;
    bool initialized = false;
    bool modifiersSupported = false;
};

DmaFrameGpuConverter::DmaFrameGpuConverter(gbm_device *device)
    : d(new Private(device))
{
}

DmaFrameGpuConverter::~DmaFrameGpuConverter()
{
    d->release();
    delete d;
}

bool DmaFrameGpuConverter::isAvailable(QString *error)
{
    if (error)
        error->clear();
    return d->ensureInitialized(error);
}

bool DmaFrameGpuConverter::canImport(int fd, int width, int height, int stride,
                                     uint32_t offset, uint32_t format,
                                     uint64_t modifier, QString *error)
{
    if (error)
        error->clear();
    GLuint texture = 0;
    if (!d->importTexture(fd, width, height, stride, offset, format, modifier,
                          &texture, error)) {
        return false;
    }
    d->destroy(texture);
    return true;
}

bool DmaFrameGpuConverter::convertToRgba(int fd, int width, int height, int stride,
                                         uint32_t offset, uint32_t format,
                                         uint64_t modifier, QByteArray *output,
                                         QString *error)
{
    if (error)
        error->clear();
    if (!output || fd < 0 || width <= 0 || height <= 0 || stride <= 0) {
        if (error)
            *error = QStringLiteral("DMA-BUF GPU 转换参数无效");
        return false;
    }
    if (!d->ensureCurrent(width, height, error))
        return false;

    GLuint texture = 0;
    if (!d->importTexture(fd, width, height, stride, offset, format, modifier,
                          &texture, error)) {
        return false;
    }

    glViewport(0, 0, width, height);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0.0, 1.0, 0.0, 1.0, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(0.0f, 0.0f);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(1.0f, 0.0f);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(1.0f, 1.0f);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(0.0f, 1.0f);
    glEnd();

    output->resize(width * height * 4);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, output->data());
    const GLenum glError = glGetError();
    d->destroy(texture);

    if (glError != GL_NO_ERROR) {
        output->clear();
        if (error)
            *error = QStringLiteral("DMA-BUF GPU 回读失败，GL error: 0x%1")
                             .arg(static_cast<unsigned>(glError), 0, 16);
        return false;
    }
    return true;
}
