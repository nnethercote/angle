//
// Copyright (c) 2012 The ANGLE Project Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

#ifndef ANGLE_TESTS_ANGLE_TEST_H_
#define ANGLE_TESTS_ANGLE_TEST_H_

#include "gtest/gtest.h"

#define GL_GLEXT_PROTOTYPES

#include "angle_gl.h"
#include <algorithm>

#include "shared_utils.h"
#include "shader_utils.h"

#include "testfixturetypes.h"

#define EXPECT_GL_ERROR(err) EXPECT_EQ((err), glGetError())
#define EXPECT_GL_NO_ERROR() EXPECT_GL_ERROR(GL_NO_ERROR)

#define ASSERT_GL_ERROR(err) ASSERT_EQ((err), glGetError())
#define ASSERT_GL_NO_ERROR() ASSERT_GL_ERROR(GL_NO_ERROR)

#define EXPECT_PIXEL_EQ(x, y, r, g, b, a) \
{ \
    GLubyte pixel[4]; \
    glReadPixels((x), (y), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel); \
    EXPECT_GL_NO_ERROR(); \
    EXPECT_EQ((r), pixel[0]); \
    EXPECT_EQ((g), pixel[1]); \
    EXPECT_EQ((b), pixel[2]); \
    EXPECT_EQ((a), pixel[3]); \
}

#define EXPECT_PIXEL_NEAR(x, y, r, g, b, a, abs_error) \
{ \
    GLubyte pixel[4]; \
    glReadPixels((x), (y), 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel); \
    EXPECT_GL_NO_ERROR(); \
    EXPECT_NEAR((r), pixel[0], abs_error); \
    EXPECT_NEAR((g), pixel[1], abs_error); \
    EXPECT_NEAR((b), pixel[2], abs_error); \
    EXPECT_NEAR((a), pixel[3], abs_error); \
}

class EGLWindow;
class OSWindow;

class ANGLETest : public testing::Test
{
  protected:
    ANGLETest(EGLint requestedRenderer, EGLint glesMajorVersion);

  public:
    bool resizeWindow(int width, int height);
    void setWindowVisible(bool isVisible);

  protected:
    virtual void SetUp();
    virtual void TearDown();

    virtual void swapBuffers();

    static void drawQuad(GLuint program, const std::string& positionAttribName, GLfloat quadDepth);
    static GLuint compileShader(GLenum type, const std::string &source);
    static bool extensionEnabled(const std::string &extName);

    void setWindowWidth(int width);
    void setWindowHeight(int height);
    void setConfigRedBits(int bits);
    void setConfigGreenBits(int bits);
    void setConfigBlueBits(int bits);
    void setConfigAlphaBits(int bits);
    void setConfigDepthBits(int bits);
    void setConfigStencilBits(int bits);
    void setMultisampleEnabled(bool enabled);

    int getClientVersion() const;
    int getWindowWidth() const;
    int getWindowHeight() const;
    bool isMultisampleEnabled() const;

  private:
    bool initTestWindow();
    bool destroyTestWindow();
    bool createEGLContext();
    bool destroyEGLContext();

    EGLWindow *mEGLWindow;
    OSWindow *mOSWindow;
};

#endif  // ANGLE_TESTS_ANGLE_TEST_H_
