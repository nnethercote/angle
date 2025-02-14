#include "ANGLETest.h"

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
typedef ::testing::Types<TFT<Gles::Two, Rend::D3D11>, TFT<Gles::Two, Rend::D3D9>> TestFixtureTypes;
TYPED_TEST_CASE(DepthStencilFormatsTest, TestFixtureTypes);

typedef ::testing::Types<TFT<Gles::Three, Rend::D3D11>> TestFixtureTypesES3;
TYPED_TEST_CASE(DepthStencilFormatsTestES3, TestFixtureTypesES3);

template<typename T>
class DepthStencilFormatsTestBase : public ANGLETest
{
  protected:
    DepthStencilFormatsTestBase() : ANGLETest(T::GetGlesMajorVersion(), T::GetRequestedRenderer())
    {
        setWindowWidth(128);
        setWindowHeight(128);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
    }

    bool checkTexImageFormatSupport(GLenum format, GLenum type)
    {
        EXPECT_GL_NO_ERROR();

        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, format, 1, 1, 0, format, type, NULL);
        glDeleteTextures(1, &tex);

        return (glGetError() == GL_NO_ERROR);
    }

    bool checkTexStorageFormatSupport(GLenum internalFormat)
    {
        EXPECT_GL_NO_ERROR();

        GLuint tex = 0;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexStorage2DEXT(GL_TEXTURE_2D, 1, internalFormat, 1, 1);
        glDeleteTextures(1, &tex);

        return (glGetError() == GL_NO_ERROR);
    }

    bool checkRenderbufferFormatSupport(GLenum internalFormat)
    {
        EXPECT_GL_NO_ERROR();

        GLuint rb = 0;
        glGenRenderbuffers(1, &rb);
        glBindRenderbuffer(GL_RENDERBUFFER, rb);
        glRenderbufferStorage(GL_RENDERBUFFER, internalFormat, 1, 1);
        glDeleteRenderbuffers(1, &rb);

        return (glGetError() == GL_NO_ERROR);
    }

    virtual void SetUp()
    {
        ANGLETest::SetUp();

        const std::string vertexShaderSource = SHADER_SOURCE
        (
            precision highp float;
            attribute vec4 position;
            varying vec2 texcoord;

            void main()
            {
                gl_Position = position;
                texcoord = (position.xy * 0.5) + 0.5;
            }
        );

        const std::string fragmentShaderSource = SHADER_SOURCE
        (
            precision highp float;
            uniform sampler2D tex;
            varying vec2 texcoord;

            void main()
            {
                gl_FragColor = texture2D(tex, texcoord);
            }
        );

        mProgram = CompileProgram(vertexShaderSource, fragmentShaderSource);
        if (mProgram == 0)
        {
            FAIL() << "shader compilation failed.";
        }

        mTextureUniformLocation = glGetUniformLocation(mProgram, "tex");
        EXPECT_NE(-1, mTextureUniformLocation);

        glGenTextures(1, &mTexture);
        ASSERT_GL_NO_ERROR();
    }

    virtual void TearDown()
    {
        glDeleteProgram(mProgram);
        glDeleteTextures(1, &mTexture);

        ANGLETest::TearDown();
    }

    GLuint mProgram;
    GLuint mTexture;
    GLint mTextureUniformLocation;
};

template <typename T>
class DepthStencilFormatsTest : public DepthStencilFormatsTestBase<T>
{};

template <typename T>
class DepthStencilFormatsTestES3 : public DepthStencilFormatsTestBase<T>
{};

TYPED_TEST(DepthStencilFormatsTest, DepthTexture)
{
    bool shouldHaveTextureSupport = extensionEnabled("GL_ANGLE_depth_texture");
    EXPECT_EQ(shouldHaveTextureSupport, checkTexImageFormatSupport(GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT));
    EXPECT_EQ(shouldHaveTextureSupport, checkTexImageFormatSupport(GL_DEPTH_COMPONENT, GL_UNSIGNED_INT));

    if (extensionEnabled("GL_EXT_texture_storage"))
    {
        EXPECT_EQ(shouldHaveTextureSupport, checkTexStorageFormatSupport(GL_DEPTH_COMPONENT16));
        EXPECT_EQ(shouldHaveTextureSupport, checkTexStorageFormatSupport(GL_DEPTH_COMPONENT32_OES));
    }
}

TYPED_TEST(DepthStencilFormatsTest, PackedDepthStencil)
{
    // Expected to fail in D3D9 if GL_OES_packed_depth_stencil is not present.
    // Expected to fail in D3D11 if GL_OES_packed_depth_stencil or GL_ANGLE_depth_texture is not present.

    bool shouldHaveRenderbufferSupport = extensionEnabled("GL_OES_packed_depth_stencil");
    EXPECT_EQ(shouldHaveRenderbufferSupport, checkRenderbufferFormatSupport(GL_DEPTH24_STENCIL8_OES));

    bool shouldHaveTextureSupport = extensionEnabled("GL_OES_packed_depth_stencil") &&
                                    extensionEnabled("GL_ANGLE_depth_texture");
    EXPECT_EQ(shouldHaveTextureSupport, checkTexImageFormatSupport(GL_DEPTH_STENCIL_OES, GL_UNSIGNED_INT_24_8_OES));

    if (extensionEnabled("GL_EXT_texture_storage"))
    {
        EXPECT_EQ(shouldHaveTextureSupport, checkTexStorageFormatSupport(GL_DEPTH24_STENCIL8_OES));
    }
}

TYPED_TEST(DepthStencilFormatsTestES3, DrawWithDepthStencil)
{
    GLushort data[16];
    for (unsigned int i = 0; i < 16; i++)
    {
        data[i] = std::numeric_limits<GLushort>::max();
    }
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexStorage2D(GL_TEXTURE_2D, 1, GL_DEPTH_COMPONENT16, 4, 4);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 4, 4, GL_DEPTH_COMPONENT, GL_UNSIGNED_SHORT, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glUseProgram(mProgram);
    glUniform1i(mTextureUniformLocation, 0);

    glClear(GL_COLOR_BUFFER_BIT);
    drawQuad(mProgram, "position", 0.5f);

    ASSERT_GL_NO_ERROR();

    EXPECT_PIXEL_NEAR(0, 0, 255, 0, 0, 255, 2.0);
}
