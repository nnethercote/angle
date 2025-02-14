#include "ANGLETest.h"

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
typedef ::testing::Types<TFT<Gles::Two, Rend::D3D11>, TFT<Gles::Two, Rend::D3D9>> TestFixtureTypes;
TYPED_TEST_CASE(FramebufferFormatsTest, TestFixtureTypes);

template<typename T>
class FramebufferFormatsTest : public ANGLETest
{
protected:
    FramebufferFormatsTest() : ANGLETest(T::GetGlesMajorVersion(), T::GetRequestedRenderer())
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    void checkBitCount(GLuint fbo, GLenum channel, GLint minBits)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);

        GLint bits = 0;
        glGetIntegerv(channel, &bits);

        if (minBits == 0)
        {
            EXPECT_EQ(minBits, bits);
        }
        else
        {
            EXPECT_GE(bits, minBits);
        }
    }

    void testBitCounts(GLuint fbo, GLint minRedBits, GLint minGreenBits, GLint minBlueBits,
                       GLint minAlphaBits, GLint minDepthBits, GLint minStencilBits)
    {
        checkBitCount(fbo, GL_RED_BITS, minRedBits);
        checkBitCount(fbo, GL_GREEN_BITS, minGreenBits);
        checkBitCount(fbo, GL_BLUE_BITS, minBlueBits);
        checkBitCount(fbo, GL_ALPHA_BITS, minAlphaBits);
        checkBitCount(fbo, GL_DEPTH_BITS, minDepthBits);
        checkBitCount(fbo, GL_STENCIL_BITS, minStencilBits);
    }

    void testTextureFormat(GLenum internalFormat, GLint minRedBits, GLint minGreenBits, GLint minBlueBits,
                           GLint minAlphaBits)
    {
        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexStorage2DEXT(GL_TEXTURE_2D, 1, internalFormat, 1, 1);

        GLuint fbo = 0;
        glGenFramebuffers(1, &fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex, 0);

        testBitCounts(fbo, minRedBits, minGreenBits, minBlueBits, minAlphaBits, 0, 0);

        glDeleteTextures(1, &tex);
        glDeleteFramebuffers(1, &fbo);
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

TYPED_TEST(FramebufferFormatsTest, RGBA4)
{
    testTextureFormat(GL_RGBA4, 4, 4, 4, 4);
}

TYPED_TEST(FramebufferFormatsTest, RGB565)
{
    testTextureFormat(GL_RGB565, 5, 6, 5, 0);
}

TYPED_TEST(FramebufferFormatsTest, RGB8)
{
    testTextureFormat(GL_RGB8_OES, 8, 8, 8, 0);
}

TYPED_TEST(FramebufferFormatsTest, BGRA8)
{
    testTextureFormat(GL_BGRA8_EXT, 8, 8, 8, 8);
}

TYPED_TEST(FramebufferFormatsTest, RGBA8)
{
    testTextureFormat(GL_RGBA8_OES, 8, 8, 8, 8);
}

