#include "ANGLETest.h"

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
typedef ::testing::Types<TFT<Gles::Three, Rend::D3D11>, TFT<Gles::Two, Rend::D3D11>, TFT<Gles::Two, Rend::D3D9>> TestFixtureTypes;
TYPED_TEST_CASE(ClearTest, TestFixtureTypes);

typedef ::testing::Types<TFT<Gles::Three, Rend::D3D11>> TestFixtureTypesES3;
TYPED_TEST_CASE(ClearTestES3, TestFixtureTypesES3);

template<typename T>
class ClearTestBase : public ANGLETest
{
  protected:
    ClearTestBase() : ANGLETest(T::GetGlesMajorVersion(), T::GetRequestedRenderer())
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    virtual void SetUp()
    {
        ANGLETest::SetUp();

        const std::string vertexShaderSource = SHADER_SOURCE
        (
            precision highp float;
            attribute vec4 position;

            void main()
            {
                gl_Position = position;
            }
        );

        const std::string fragmentShaderSource = SHADER_SOURCE
        (
            precision highp float;

            void main()
            {
                gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
            }
        );

        mProgram = CompileProgram(vertexShaderSource, fragmentShaderSource);
        if (mProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        glGenFramebuffers(1, &mFBO);

        ASSERT_GL_NO_ERROR();
    }

    virtual void TearDown()
    {
        glDeleteProgram(mProgram);
        glDeleteFramebuffers(1, &mFBO);

        ANGLETest::TearDown();
    }

    GLuint mProgram;
    GLuint mFBO;
};

template <typename T>
class ClearTest : public ClearTestBase<T>
{};

template <typename T>
class ClearTestES3 : public ClearTestBase<T>
{};

TYPED_TEST(ClearTest, ClearIssue)
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glClearColor(0.0, 1.0, 0.0, 1.0);
    glClearDepthf(0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

    GLuint rbo;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB565, 16, 16);

    EXPECT_GL_NO_ERROR();

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);

    EXPECT_GL_NO_ERROR();

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClearDepthf(1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    EXPECT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    drawQuad(mProgram, "position", 0.5f);

    EXPECT_PIXEL_EQ(0, 0, 0, 255, 0, 255);
}

// Requires ES3
// This tests a bug where in a masked clear when calling "ClearBuffer", we would
// mistakenly clear every channel (including the masked-out ones)
TYPED_TEST(ClearTestES3, MaskedClearBufferBug)
{
    unsigned char pixelData[] = { 255, 255, 255, 255 };

    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

    GLuint textures[2];
    glGenTextures(2, &textures[0]);

    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[0], 0);

    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelData);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, textures[1], 0);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(0, 0, 255, 255, 255, 255);

    float clearValue[] = { 0, 0.5f, 0.5f, 1.0f };
    GLenum drawBuffers[] = { GL_NONE, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, drawBuffers);
    glColorMask(GL_TRUE, GL_TRUE, GL_FALSE, GL_TRUE);
    glClearBufferfv(GL_COLOR, 1, clearValue);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(0, 0, 255, 255, 255, 255);

    // TODO: glReadBuffer support
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, 0, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[1], 0);
    EXPECT_PIXEL_EQ(0, 0, 0, 127, 255, 255);

    glDeleteTextures(2, textures);
}

TYPED_TEST(ClearTestES3, BadFBOSerialBug)
{
    // First make a simple framebuffer, and clear it to green
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);

    GLuint textures[2];
    glGenTextures(2, &textures[0]);

    glBindTexture(GL_TEXTURE_2D, textures[0]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, getWindowWidth(), getWindowHeight());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[0], 0);

    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
    glDrawBuffers(1, drawBuffers);

    float clearValues1[] = { 0.0f, 1.0f, 0.0f, 1.0f };
    glClearBufferfv(GL_COLOR, 0, clearValues1);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(0, 0, 0, 255, 0, 255);

    // Next make a second framebuffer, and draw it to red
    // (Triggers bad applied render target serial)
    GLuint fbo2;
    glGenFramebuffers(1, &fbo2);
    ASSERT_GL_NO_ERROR();

    glBindFramebuffer(GL_FRAMEBUFFER, fbo2);

    glBindTexture(GL_TEXTURE_2D, textures[1]);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, getWindowWidth(), getWindowHeight());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textures[1], 0);

    glDrawBuffers(1, drawBuffers);

    drawQuad(mProgram, "position", 0.5f);

    ASSERT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(0, 0, 255, 0, 0, 255);

    // Check that the first framebuffer is still green.
    glBindFramebuffer(GL_FRAMEBUFFER, mFBO);
    EXPECT_PIXEL_EQ(0, 0, 0, 255, 0, 255);

    glDeleteTextures(2, textures);
    glDeleteFramebuffers(1, &fbo2);
}
