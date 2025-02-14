#include "ANGLETest.h"

#include <cstdint>

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
typedef ::testing::Types<TFT<Gles::Two, Rend::D3D11>, TFT<Gles::Two, Rend::D3D9>> TestFixtureTypes;
TYPED_TEST_CASE(BufferDataTest, TestFixtureTypes);

template<typename T>
class BufferDataTest : public ANGLETest
{
  protected:
    BufferDataTest() : ANGLETest(T::GetGlesMajorVersion(), T::GetRequestedRenderer())
    {
        setWindowWidth(16);
        setWindowHeight(16);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);

        mBuffer = 0;
        mProgram = 0;
        mAttribLocation = -1;
    }

    virtual void SetUp()
    {
        ANGLETest::SetUp();

        const char * vsSource = SHADER_SOURCE
        (
            attribute vec4 position;
            attribute float in_attrib;
            varying float v_attrib;
            void main()
            {
                v_attrib = in_attrib;
                gl_Position = position;
            }
        );

        const char * fsSource = SHADER_SOURCE
        (
            precision mediump float;
            varying float v_attrib;
            void main()
            {
                gl_FragColor = vec4(v_attrib, 0, 0, 1);
            }
        );

        glGenBuffers(1, &mBuffer);
        ASSERT_NE(mBuffer, 0U);

        mProgram = CompileProgram(vsSource, fsSource);
        ASSERT_NE(mProgram, 0U);

        mAttribLocation = glGetAttribLocation(mProgram, "in_attrib");
        ASSERT_NE(mAttribLocation, -1);

        glClearColor(0, 0, 0, 0);
        glClearDepthf(0.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glDisable(GL_DEPTH_TEST);

        ASSERT_GL_NO_ERROR();
    }

    virtual void TearDown()
    {
        glDeleteBuffers(1, &mBuffer);
        glDeleteProgram(mProgram);

        ANGLETest::TearDown();
    }

    GLuint mBuffer;
    GLuint mProgram;
    GLint mAttribLocation;
};

TYPED_TEST(BufferDataTest, NULLData)
{
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    EXPECT_GL_NO_ERROR();

    const int numIterations = 128;
    for (int i = 0; i < numIterations; ++i)
    {
        GLsizei bufferSize = sizeof(GLfloat) * (i + 1);
        glBufferData(GL_ARRAY_BUFFER, bufferSize, NULL, GL_STATIC_DRAW);
        EXPECT_GL_NO_ERROR();

        for (int j = 0; j < bufferSize; j++)
        {
            for (int k = 0; k < bufferSize - j; k++)
            {
                glBufferSubData(GL_ARRAY_BUFFER, k, j, NULL);
                EXPECT_GL_NO_ERROR();
            }
        }
    }
}

TYPED_TEST(BufferDataTest, ZeroNonNULLData)
{
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    EXPECT_GL_NO_ERROR();

    char *zeroData = new char[0];
    glBufferData(GL_ARRAY_BUFFER, 0, zeroData, GL_STATIC_DRAW);
    EXPECT_GL_NO_ERROR();

    glBufferSubData(GL_ARRAY_BUFFER, 0, 0, zeroData);
    EXPECT_GL_NO_ERROR();

    delete [] zeroData;
}

TYPED_TEST(BufferDataTest, NULLResolvedData)
{
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, 128, NULL, GL_DYNAMIC_DRAW);

    glUseProgram(mProgram);
    glVertexAttribPointer(mAttribLocation, 1, GL_FLOAT, GL_FALSE, 4, NULL);
    glEnableVertexAttribArray(mAttribLocation);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    drawQuad(mProgram, "position", 0.5f);
}

TYPED_TEST(BufferDataTest, HugeSetDataShouldNotCrash)
{
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    EXPECT_GL_NO_ERROR();

    GLsizei allocSize = std::numeric_limits<GLsizei>::max() >> 2;

    uint8_t *data = NULL;
    while (data == NULL && allocSize >= 4)
    {
        data = new (std::nothrow) uint8_t[allocSize];

        if (data == NULL)
        {
            allocSize >>= 1;
        }
    }

    ASSERT_NE(static_cast<uint8_t*>(NULL), data);
    memset(data, 0, allocSize);

    float * fValue = reinterpret_cast<float*>(data);
    for (unsigned int f = 0; f < 6; f++)
    {
        fValue[f] = 1.0f;
    }

    glBufferData(GL_ARRAY_BUFFER, allocSize, data, GL_STATIC_DRAW);

    GLenum error = glGetError();
    if (error == GL_NO_ERROR)
    {
        // If we didn't fail because of an out of memory error, try drawing a quad
        // using the large buffer

        // DISABLED because it takes a long time, but left for posterity

        //glUseProgram(mProgram);
        //glVertexAttribPointer(mAttribLocation, 1, GL_FLOAT, GL_FALSE, 4, NULL);
        //glEnableVertexAttribArray(mAttribLocation);
        //glBindBuffer(GL_ARRAY_BUFFER, 0);
        //drawQuad(mProgram, "position", 0.5f);
        //swapBuffers();

        //// Draw operations can also generate out-of-memory, which is in-spec
        //error = glGetError();
        //if (error == GL_NO_ERROR)
        //{
        //    GLint viewportSize[4];
        //    glGetIntegerv(GL_VIEWPORT, viewportSize);

        //    GLint midPixelX = (viewportSize[0] + viewportSize[2]) / 2;
        //    GLint midPixelY = (viewportSize[1] + viewportSize[3]) / 2;

        //    EXPECT_PIXEL_EQ(midPixelX, midPixelY, 255, 0, 0, 255);
        //}
        //else
        //{
        //    EXPECT_EQ(GL_OUT_OF_MEMORY, error);
        //}
    }
    else
    {
        EXPECT_EQ(GL_OUT_OF_MEMORY, error);
    }

    delete[] data;
}

// Use this to select which configurations (e.g. which renderer, which GLES major version) these tests should be run against.
typedef ::testing::Types<TFT<Gles::Three, Rend::D3D11>> TestFixtureTypesIndexedBufferCopyTest;
TYPED_TEST_CASE(IndexedBufferCopyTest, TestFixtureTypesIndexedBufferCopyTest);

template<typename T>
class IndexedBufferCopyTest : public ANGLETest
{
  protected:
    IndexedBufferCopyTest() : ANGLETest(T::GetGlesMajorVersion(), T::GetRequestedRenderer())
    {
        setWindowWidth(16);
        setWindowHeight(16);
        setConfigRedBits(8);
        setConfigGreenBits(8);
        setConfigBlueBits(8);
        setConfigAlphaBits(8);
        setConfigDepthBits(24);
    }

    virtual void SetUp()
    {
        ANGLETest::SetUp();

        const char * vsSource = SHADER_SOURCE
        (
            attribute vec3 in_attrib;
            varying vec3 v_attrib;
            void main()
            {
                v_attrib = in_attrib;
                gl_Position = vec4(0.0, 0.0, 0.5, 1.0);
                gl_PointSize = 100.0;
            }
        );

        const char * fsSource = SHADER_SOURCE
        (
            precision mediump float;
            varying vec3 v_attrib;
            void main()
            {
                gl_FragColor = vec4(v_attrib, 1);
            }
        );

        glGenBuffers(2, mBuffers);
        ASSERT_NE(mBuffers[0], 0U);
        ASSERT_NE(mBuffers[1], 0U);

        glGenBuffers(1, &mElementBuffer);
        ASSERT_NE(mElementBuffer, 0U);

        mProgram = CompileProgram(vsSource, fsSource);
        ASSERT_NE(mProgram, 0U);

        mAttribLocation = glGetAttribLocation(mProgram, "in_attrib");
        ASSERT_NE(mAttribLocation, -1);

        glClearColor(0, 0, 0, 0);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);

        ASSERT_GL_NO_ERROR();
    }

    virtual void TearDown()
    {
        glDeleteBuffers(2, mBuffers);
        glDeleteBuffers(1, &mElementBuffer);
        glDeleteProgram(mProgram);

        ANGLETest::TearDown();
    }

    GLuint mBuffers[2];
    GLuint mElementBuffer;
    GLuint mProgram;
    GLint mAttribLocation;
};

// The following test covers an ANGLE bug where our index ranges
// weren't updated from CopyBufferSubData calls
// https://code.google.com/p/angleproject/issues/detail?id=709
TYPED_TEST(IndexedBufferCopyTest, IndexRangeBug)
{
    unsigned char vertexData[] = { 255, 0, 0, 0, 0, 0 };
    unsigned int indexData[] = { 0, 1 };

    glBindBuffer(GL_ARRAY_BUFFER, mBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(char) * 6, vertexData, GL_STATIC_DRAW);

    glUseProgram(mProgram);
    glVertexAttribPointer(mAttribLocation, 3, GL_UNSIGNED_BYTE, GL_TRUE, 3, NULL);
    glEnableVertexAttribArray(mAttribLocation);

    ASSERT_GL_NO_ERROR();

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mElementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * 1, indexData, GL_STATIC_DRAW);

    glUseProgram(mProgram);

    ASSERT_GL_NO_ERROR();

    glDrawElements(GL_POINTS, 1, GL_UNSIGNED_INT, NULL);

    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(0, 0, 255, 0, 0, 255);

    glBindBuffer(GL_COPY_READ_BUFFER, mBuffers[1]);
    glBufferData(GL_COPY_READ_BUFFER, 4, &indexData[1], GL_STATIC_DRAW);

    glBindBuffer(GL_COPY_WRITE_BUFFER, mElementBuffer);

    glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0, 0, sizeof(int));

    ASSERT_GL_NO_ERROR();

    glClear(GL_COLOR_BUFFER_BIT);
    EXPECT_PIXEL_EQ(0, 0, 0, 0, 0, 0);

    unsigned char newData[] = { 0, 255, 0 };
    glBufferSubData(GL_ARRAY_BUFFER, 3, 3, newData);

    glDrawElements(GL_POINTS, 1, GL_UNSIGNED_INT, NULL);

    EXPECT_GL_NO_ERROR();
    EXPECT_PIXEL_EQ(0, 0, 0, 255, 0, 255);
}
