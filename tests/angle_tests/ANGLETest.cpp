#include "ANGLETest.h"
#include "EGLWindow.h"
#include "OSWindow.h"

ANGLETest::ANGLETest(EGLint glesMajorVersion, EGLint requestedRenderer)
    : mEGLWindow(NULL),
      mOSWindow(NULL)
{
    mEGLWindow = new EGLWindow(1280, 720, glesMajorVersion, requestedRenderer);
}

void ANGLETest::SetUp()
{
    if (!initTestWindow())
    {
        FAIL() << "Failed to create ANGLE test window.";
    }

    if (!resizeWindow(mEGLWindow->getWidth(), mEGLWindow->getHeight()))
    {
        FAIL() << "Failed to resize ANGLE test window.";
    }

    if (!createEGLContext())
    {
        FAIL() << "egl context creation failed.";
    }
}

void ANGLETest::TearDown()
{
    swapBuffers();
    mOSWindow->messageLoop();

    if (!destroyEGLContext())
    {
        FAIL() << "egl context destruction failed.";
    }

    // Check for quit message
    Event myEvent;
    while (mOSWindow->popEvent(&myEvent))
    {
        if (myEvent.Type == Event::EVENT_CLOSED)
        {
            exit(0);
        }
    }

    if (!destroyTestWindow())
    {
        FAIL() << "ANGLE test window destruction failed.";
    }
}

void ANGLETest::swapBuffers()
{
    mEGLWindow->swap();
}

void ANGLETest::drawQuad(GLuint program, const std::string& positionAttribName, GLfloat quadDepth)
{
    GLint positionLocation = glGetAttribLocation(program, positionAttribName.c_str());

    glUseProgram(program);

    const GLfloat vertices[] =
    {
        -1.0f,  1.0f, quadDepth,
        -1.0f, -1.0f, quadDepth,
         1.0f, -1.0f, quadDepth,

        -1.0f,  1.0f, quadDepth,
         1.0f, -1.0f, quadDepth,
         1.0f,  1.0f, quadDepth,
    };

    glVertexAttribPointer(positionLocation, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(positionLocation);

    glDrawArrays(GL_TRIANGLES, 0, 6);

    glDisableVertexAttribArray(positionLocation);
    glVertexAttribPointer(positionLocation, 4, GL_FLOAT, GL_FALSE, 0, NULL);

    glUseProgram(0);
}

GLuint ANGLETest::compileShader(GLenum type, const std::string &source)
{
    GLuint shader = glCreateShader(type);

    const char *sourceArray[1] = { source.c_str() };
    glShaderSource(shader, 1, sourceArray, NULL);
    glCompileShader(shader);

    GLint compileResult;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult);

    if (compileResult == 0)
    {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

        std::vector<GLchar> infoLog(infoLogLength);
        glGetShaderInfoLog(shader, infoLog.size(), NULL, &infoLog[0]);

        std::cerr << "shader compilation failed: " << &infoLog[0];

        glDeleteShader(shader);
        shader = 0;
    }

    return shader;
}

bool ANGLETest::extensionEnabled(const std::string &extName)
{
    const char* extString = reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS));
    return strstr(extString, extName.c_str()) != NULL;
}

void ANGLETest::setWindowWidth(int width)
{
    mEGLWindow->setWidth(width);
}

void ANGLETest::setWindowHeight(int height)
{
    mEGLWindow->setHeight(height);
}

void ANGLETest::setConfigRedBits(int bits)
{
    mEGLWindow->setConfigRedBits(bits);
}

void ANGLETest::setConfigGreenBits(int bits)
{
    mEGLWindow->setConfigGreenBits(bits);
}

void ANGLETest::setConfigBlueBits(int bits)
{
    mEGLWindow->setConfigBlueBits(bits);
}

void ANGLETest::setConfigAlphaBits(int bits)
{
    mEGLWindow->setConfigAlphaBits(bits);
}

void ANGLETest::setConfigDepthBits(int bits)
{
    mEGLWindow->setConfigDepthBits(bits);
}

void ANGLETest::setConfigStencilBits(int bits)
{
    mEGLWindow->setConfigStencilBits(bits);
}

void ANGLETest::setMultisampleEnabled(bool enabled)
{
    mEGLWindow->setMultisample(enabled);
}

int ANGLETest::getClientVersion() const
{
    return mEGLWindow->getClientVersion();
}

int ANGLETest::getWindowWidth() const
{
    return mEGLWindow->getWidth();
}

int ANGLETest::getWindowHeight() const
{
    return mEGLWindow->getHeight();
}

bool ANGLETest::isMultisampleEnabled() const
{
    return mEGLWindow->isMultisample();
}

bool ANGLETest::createEGLContext()
{
    return mEGLWindow->initializeGL(mOSWindow);
}

bool ANGLETest::destroyEGLContext()
{
    mEGLWindow->destroyGL();
    return true;
}

bool ANGLETest::initTestWindow()
{
    mOSWindow = CreateOSWindow();
    if (!mOSWindow->initialize("ANGLE_TEST", 128, 128))
    {
        return false;
    }

    mOSWindow->setVisible(false);

    return true;
}

bool ANGLETest::destroyTestWindow()
{
    if (mOSWindow)
    {
        mOSWindow->destroy();
        delete mOSWindow;
        mOSWindow = NULL;
    }

    return true;
}

bool ANGLETest::resizeWindow(int width, int height)
{
    return mOSWindow->resize(width, height);
}

void ANGLETest::setWindowVisible(bool isVisible)
{
    mOSWindow->setVisible(isVisible);
}
