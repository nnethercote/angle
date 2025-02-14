#include "ANGLETest.h"

#include <vector>

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
typedef ::testing::Types<TFT<Gles::Two, Rend::D3D11>, TFT<Gles::Two, Rend::D3D9>> TestFixtureTypes;
TYPED_TEST_CASE(IncompleteTextureTest, TestFixtureTypes);

template<typename T>
class IncompleteTextureTest : public ANGLETest
{
protected:
    IncompleteTextureTest() : ANGLETest(T::GetGlesMajorVersion(), T::GetRequestedRenderer())
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
    }

    virtual void TearDown()
    {
        glDeleteProgram(mProgram);

        ANGLETest::TearDown();
    }

    void fillTextureData(std::vector<GLubyte> &buffer, GLubyte r, GLubyte g, GLubyte b, GLubyte a)
    {
        size_t count = buffer.size() / 4;
        for (size_t i = 0; i < count; i++)
        {
            buffer[i * 4 + 0] = r;
            buffer[i * 4 + 1] = g;
            buffer[i * 4 + 2] = b;
            buffer[i * 4 + 3] = a;
        }
    }

    GLuint mProgram;
    GLint mTextureUniformLocation;
};

TYPED_TEST(IncompleteTextureTest, IncompleteTexture2D)
{
    GLuint tex;
    glGenTextures(1, &tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    glUseProgram(mProgram);
    glUniform1i(mTextureUniformLocation, 0);

    const GLsizei textureWidth = 2;
    const GLsizei textureHeight = 2;
    std::vector<GLubyte> textureData(textureWidth * textureHeight * 4);
    fillTextureData(textureData, 255, 0, 0, 255);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, textureWidth, textureHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, &textureData[0]);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_EQ(0, 0, 255, 0, 0, 255);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_EQ(0, 0, 0, 0, 0, 255);

    glTexImage2D(GL_TEXTURE_2D, 1, GL_RGBA, textureWidth >> 1, textureHeight >> 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &textureData[0]);

    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_EQ(0, 0, 255, 0, 0, 255);

    glDeleteTextures(1, &tex);
}

TYPED_TEST(IncompleteTextureTest, UpdateTexture)
{
    GLuint tex;
    glGenTextures(1, &tex);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    glUseProgram(mProgram);
    glUniform1i(mTextureUniformLocation, 0);

    const GLsizei redTextureWidth = 64;
    const GLsizei redTextureHeight = 64;
    std::vector<GLubyte> redTextureData(redTextureWidth * redTextureHeight * 4);
    fillTextureData(redTextureData, 255, 0, 0, 255);
    for (size_t i = 0; i < 7; i++)
    {
        glTexImage2D(GL_TEXTURE_2D, i, GL_RGBA, redTextureWidth >> i, redTextureHeight >> i, 0, GL_RGBA, GL_UNSIGNED_BYTE,
                     &redTextureData[0]);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_EQ(0, 0, 255, 0, 0, 255);

    const GLsizei greenTextureWidth = 32;
    const GLsizei greenTextureHeight = 32;
    std::vector<GLubyte> greenTextureData(greenTextureWidth * greenTextureHeight * 4);
    fillTextureData(greenTextureData, 0, 255, 0, 255);

    for (size_t i = 0; i < 6; i++)
    {
        glTexSubImage2D(GL_TEXTURE_2D, i, greenTextureWidth >> i, greenTextureHeight >> i,
                        greenTextureWidth >> i, greenTextureHeight >> i, GL_RGBA, GL_UNSIGNED_BYTE,
                        &greenTextureData[0]);
    }

    drawQuad(mProgram, "position", 0.5f);
    EXPECT_PIXEL_EQ(getWindowWidth() - greenTextureWidth, getWindowHeight() - greenTextureWidth, 0, 255, 0, 255);

    glDeleteTextures(1, &tex);
}
