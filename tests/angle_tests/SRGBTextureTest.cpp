#include "ANGLETest.h"

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
typedef ::testing::Types<TFT<Gles::Two, Rend::D3D11>, TFT<Gles::Two, Rend::D3D9>> TestFixtureTypes;
TYPED_TEST_CASE(SRGBTextureTest, TestFixtureTypes);

template<typename T>
class SRGBTextureTest : public ANGLETest
{
protected:
    SRGBTextureTest() : ANGLETest(T::GetGlesMajorVersion(), T::GetRequestedRenderer())
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    virtual void SetUp()
    {
        ANGLETest::SetUp();
    }

    virtual void TearDown()
    {
        ANGLETest::TearDown();
    }
};

TYPED_TEST(SRGBTextureTest, SRGBValidation)
{
    bool supported = extensionEnabled("GL_EXT_sRGB") || getClientVersion() == 3;

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    GLubyte pixel[3] = { 0 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, 1, 1, 0, GL_SRGB, GL_UNSIGNED_BYTE, pixel);
    if (supported)
    {
        EXPECT_GL_NO_ERROR();

        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, GL_SRGB, GL_UNSIGNED_BYTE, pixel);
        EXPECT_GL_NO_ERROR();

        glGenerateMipmap(GL_TEXTURE_2D);
        EXPECT_GL_ERROR(GL_INVALID_OPERATION);
    }
    else
    {
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }

    glDeleteTextures(1, &tex);
}

TYPED_TEST(SRGBTextureTest, SRGBAValidation)
{
    bool supported = extensionEnabled("GL_EXT_sRGB") || getClientVersion() == 3;

    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    GLubyte pixel[4] = { 0 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB_ALPHA_EXT, 1, 1, 0, GL_SRGB_ALPHA_EXT, GL_UNSIGNED_BYTE, pixel);
    if (supported)
    {
        EXPECT_GL_NO_ERROR();

        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, GL_SRGB_ALPHA_EXT, GL_UNSIGNED_BYTE, pixel);
        EXPECT_GL_NO_ERROR();

        glGenerateMipmap(GL_TEXTURE_2D);
        if (getClientVersion() == 2)
        {
            EXPECT_GL_ERROR(GL_INVALID_OPERATION);
        }
        else
        {
            EXPECT_GL_NO_ERROR();
        }
    }
    else
    {
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }

    glDeleteTextures(1, &tex);
}

TYPED_TEST(SRGBTextureTest, SRGBARenderbuffer)
{
    bool supported = extensionEnabled("GL_EXT_sRGB") || getClientVersion() == 3;

    GLuint rbo = 0;
    glGenRenderbuffers(1, &rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo);

    glRenderbufferStorage(GL_RENDERBUFFER, GL_SRGB8_ALPHA8_EXT, 1, 1);
    if (supported)
    {
        EXPECT_GL_NO_ERROR();
    }
    else
    {
        EXPECT_GL_ERROR(GL_INVALID_ENUM);

        // Make sure the rbo has a size for future tests
        glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8_OES, 1, 1);
        EXPECT_GL_NO_ERROR();
    }

    GLuint fbo = 0;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, rbo);
    EXPECT_GL_NO_ERROR();

    GLint colorEncoding = 0;
    glGetFramebufferAttachmentParameteriv(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                                          GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING_EXT, &colorEncoding);
    if (supported)
    {
        EXPECT_GL_NO_ERROR();
        EXPECT_EQ(GL_SRGB_EXT, colorEncoding);
    }
    else
    {
        EXPECT_GL_ERROR(GL_INVALID_ENUM);
    }

    glDeleteFramebuffers(1, &fbo);
    glDeleteRenderbuffers(1, &rbo);
}
