#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#define _USE_MATH_DEFINES
#include "pipeline.h"
#include <cmath>
#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

GLuint VBO;
GLuint IBO;
GLuint gWVPLocation;

// Объявляем версию языка шейдеров, которую будем использовать
// Устанавливаем расположение атрибута Position в вершинном буфере данных
// Объявляем uniform-переменную для передачи матрицы преобразования
// Преобразуем позицию вершины с помощью матрицы преобразования и устанавливаем в gl_Position
static const char* pVS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
layout (location = 0) in vec3 Position;                                             \n\
                                                                                    \n\
uniform mat4 gWVP;                                                                  \n\
                                                                                    \n\
out vec4 Color;                                                                     \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    gl_Position = gWVP * vec4(Position, 1.0);                                       \n\
    Color = vec4(clamp(Position, 0.0, 1.0), 1.0);                                   \n\
}";
// Устанавливаем значение цвета для выходной переменной Color
// clamp используется для ограничения значений компонентов цвета в диапазоне от 0 до 1
// Устанавливаем значение цвета для выходной переменной FragColor
static const char* pFS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
in vec4 Color;                                                                      \n\
                                                                                    \n\
out vec4 FragColor;                                                                 \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    FragColor = Color;                                                              \n\
}";
// Функция отрисовки сцены
static void RenderSceneCB()
{// Очистка буфера цвета
    glClear(GL_COLOR_BUFFER_BIT);

    static float Scale = 0.0f;// Изменение масштаба сцены

    Scale += 0.1f;
    // Создание конвейера для преобразования сцены
    Pipeline p;
    p.Rotate(0.0f, Scale, 0.0f);// Поворот сцены
    p.WorldPos(0.0f, 0.0f, 1.0f);// Перемещение сцены
    Vector3f CameraPos(0.0f, -2.0f,-2.0f); // Положение камеры 0.0.-3 po z ot treugolnika rastiyania
    Vector3f CameraTarget(0.0f, 3.0f, 6.0f);// Точка, на которую смотрит камера 0.1.0 vverh, 0. - 1.0 cam vniz
    Vector3f CameraUp(0.0f, 3.0f, -3.0f);  // Вектор направления "вверх" камеры 0.1.0 normal, 0.-1.0 cam perevernuta  4.1.0 povorot po y protiv chasovoy
    p.SetCamera(CameraPos, CameraTarget, CameraUp);
    p.SetPerspectiveProj(60.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 1.0f, 100.0f);

    glUniformMatrix4fv(gWVPLocation, 1, GL_TRUE, (const GLfloat*)p.GetTrans());// Установка матрицы преобразования в шейдере

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

    glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

    glDisableVertexAttribArray(0);

    glutSwapBuffers();
}


static void InitializeGlutCallbacks()
{
    glutDisplayFunc(RenderSceneCB);
    glutIdleFunc(RenderSceneCB);
}

static void CreateVertexBuffer()
{// Определение координат вершин треугольника
    Vector3f Vertices[4];
    Vertices[0] = Vector3f(-1.0f, -1.0f, 0.5773f);
    Vertices[1] = Vector3f(0.0f, -1.0f, -1.15475);
    Vertices[2] = Vector3f(1.0f, -1.0f, 0.5773f);
    Vertices[3] = Vector3f(0.0f, 1.0f, 0.0f);
    // Создание буфера вершин и связывание с ним
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
    // Копирование данных о вершинах в буфер вершин
}

static void CreateIndexBuffer()
{// Определение индексов для вершин треугольника
    unsigned int Indices[] = { 0, 3, 1,
                               1, 3, 2,
                               2, 3, 0,
                               0, 2, 1 };

    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType) //Функция AddShader компилирует шейдер, переданный в виде текста
{
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0) {
        fprintf(stderr, "Error creating shader type %d\n", ShaderType);
        exit(0);
    }

    const GLchar* p[1];
    p[0] = pShaderText;
    GLint Lengths[1];
    Lengths[0] = strlen(pShaderText);
    glShaderSource(ShaderObj, 1, p, Lengths);
    glCompileShader(ShaderObj);
    GLint success;
    glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
    if (!success) {
        GLchar InfoLog[1024];
        glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
        fprintf(stderr, "Error compiling shader type %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }

    glAttachShader(ShaderProgram, ShaderObj);
}

static void CompileShaders() //Функция CompileShaders создает программу шейдеров и компилирует вершинный и фрагментный шейдеры.
{
    GLuint ShaderProgram = glCreateProgram();

    if (ShaderProgram == 0) {
        fprintf(stderr, "Error creating shader program\n");
        exit(1);
    }

    AddShader(ShaderProgram, pVS, GL_VERTEX_SHADER);
    AddShader(ShaderProgram, pFS, GL_FRAGMENT_SHADER);

    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };

    glLinkProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
    if (Success == 0) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Error linking shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    glValidateProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success);
    if (!Success) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "Invalid shader program: '%s'\n", ErrorLog);
        exit(1);
    }

    glUseProgram(ShaderProgram);

    gWVPLocation = glGetUniformLocation(ShaderProgram, "gWVP");
    assert(gWVPLocation != 0xFFFFFFFF);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("camera");

    InitializeGlutCallbacks();

    GLenum res = glewInit();
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    CreateVertexBuffer();
    CreateIndexBuffer();

    CompileShaders();

    glutMainLoop();

    return 0;
}