#include <jni.h>
#include <GLES3/gl3.h>
#include <android/log.h>

#define LOG_TAG "APOCALYPSE_ENGINE"
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, LOG_TAG, __VA_ARGS__)

namespace {
GLuint program = 0;
GLuint vertexBuffer = 0;
GLint positionLocation = -1;
GLint colorLocation = -1;
int surfaceWidth = 1;
int surfaceHeight = 1;

GLuint compileShader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok != GL_TRUE) {
        char log[1024] = {};
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        LOGE("Shader compile failed: %s", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}
}

extern "C" JNIEXPORT void JNICALL
Java_com_apocalypse_game_NativeGameActivity_00024NativeRenderer_nativeInit(JNIEnv*, jclass) {
    const char* vertexSource =
        "#version 300 es\n"
        "layout(location=0) in vec2 aPosition;\n"
        "void main(){ gl_Position=vec4(aPosition,0.0,1.0); }\n";
    const char* fragmentSource =
        "#version 300 es\n"
        "precision mediump float;\n"
        "out vec4 fragColor;\n"
        "uniform vec4 uColor;\n"
        "void main(){ fragColor=uColor; }\n";

    GLuint vertex = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragment = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (!vertex || !fragment) return;

    program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    GLint linked = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked != GL_TRUE) {
        char log[1024] = {};
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        LOGE("Program link failed: %s", log);
        glDeleteProgram(program);
        program = 0;
        return;
    }

    const GLfloat triangle[] = { 0.0f, 0.62f, -0.58f, -0.48f, 0.58f, -0.48f };
    glGenBuffers(1, &vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangle), triangle, GL_STATIC_DRAW);
    positionLocation = 0;
    colorLocation = glGetUniformLocation(program, "uColor");
}

extern "C" JNIEXPORT void JNICALL
Java_com_apocalypse_game_NativeGameActivity_00024NativeRenderer_nativeResize(JNIEnv*, jclass, jint width, jint height) {
    surfaceWidth = width > 0 ? width : 1;
    surfaceHeight = height > 0 ? height : 1;
    glViewport(0, 0, surfaceWidth, surfaceHeight);
}

extern "C" JNIEXPORT void JNICALL
Java_com_apocalypse_game_NativeGameActivity_00024NativeRenderer_nativeRender(JNIEnv*, jclass) {
    glViewport(0, 0, surfaceWidth, surfaceHeight);
    glClearColor(0.025f, 0.045f, 0.09f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    if (!program || !vertexBuffer || colorLocation < 0) return;

    glUseProgram(program);
    glUniform4f(colorLocation, 0.10f, 0.72f, 1.0f, 1.0f);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glEnableVertexAttribArray(static_cast<GLuint>(positionLocation));
    glVertexAttribPointer(static_cast<GLuint>(positionLocation), 2, GL_FLOAT, GL_FALSE, 0, nullptr);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glDisableVertexAttribArray(static_cast<GLuint>(positionLocation));
}