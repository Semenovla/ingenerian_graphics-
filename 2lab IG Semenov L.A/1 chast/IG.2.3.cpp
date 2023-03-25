
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "math_3d.h"

GLuint VBO;
GLuint gWorldLocation;


static const char* pVS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
layout (location = 0) in vec3 Position;                                             \n\
                                                                                    \n\
uniform mat4 gWorld;                                                                \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    gl_Position = gWorld * vec4(Position, 1.0);                                     \n\
}";

static const char* pFS = "                                                          \n\
#version 330                                                                        \n\
                                                                                    \n\
out vec4 FragColor;                                                                 \n\
                                                                                    \n\
void main()                                                                         \n\
{                                                                                   \n\
    FragColor = vec4(1.0, 0.0, 0.0, 1.0);                                           \n\
}";

static void RenderSceneCB() //объявление функции RenderSceneCB() с модификатором static.
{
    glClear(GL_COLOR_BUFFER_BIT); //очистка экрана текущим цветом

    static float Scale = 0.0f; //объявление и инициализация статической переменной Scale типа float.

    Scale += 0.005f;

    Matrix4f World;

    World.m[0][0] = sinf(Scale);    World.m[0][1] = sinf(Scale);  World.m[0][2] = 0.0f;        World.m[0][3] = -sinf(Scale); //1 u 2) povorot u mashtab 4) peremeshenie L u R po osi X
    World.m[1][0] = sinf(Scale);   World.m[1][1] = cosf(Scale);   World.m[1][2] = 0.0f;        World.m[1][3] = 0.0f;         // 1 u 2) povorot u mashtab osi Y
    World.m[2][0] = 0.0f;           World.m[2][1] = 0.0f;          World.m[2][2] = sinf(Scale); World.m[2][3] = 0.0f;        // Os Z 
    World.m[3][0] = 0.0f;           World.m[3][1] = 0.0f;          World.m[3][2] = 0.0f;        World.m[3][3] = 1.0f;       

    glUniformMatrix4fv(gWorldLocation, 1, GL_TRUE, &World.m[0][0]); //передача значения матрицы World в uniform-переменную gWorldLocation на графическую карту.

    glEnableVertexAttribArray(0); //ключение массива атрибутов вершин с индексом 0.
    glBindBuffer(GL_ARRAY_BUFFER, VBO); //связывание буфера вершин VBO с текущим контекстом OpenGL.
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0); //установка указателя на данные массива атрибутов вершин и их параметров.

    glDrawArrays(GL_TRIANGLES, 0, 3); //отрисовка треугольников на основе массива вершин

    glDisableVertexAttribArray(0);

    glutSwapBuffers(); //смена буферов и отображение результата.
}


static void InitializeGlutCallbacks() //устанавливает обратные вызовы для отображения и прорисовки сцены.
{
    glutDisplayFunc(RenderSceneCB);
    glutIdleFunc(RenderSceneCB);
}

static void CreateVertexBuffer() //оздает буфер вершин и заполняет его данными для треугольника.
{
    Vector3f Vertices[3];
    Vertices[0] = Vector3f(-1.0f, -1.0f, 0.0f);
    Vertices[1] = Vector3f(1.0f, -1.0f, 0.0f);
    Vertices[2] = Vector3f(0.0f, 1.0f, 0.0f);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
}

static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType) // компилирует и прикрепляет шейдер к программе шейдеров.
{
    GLuint ShaderObj = glCreateShader(ShaderType);

    if (ShaderObj == 0) {
        fprintf(stderr, "Ошибка создания шейдера %d\n", ShaderType);
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
        fprintf(stderr, "Ошибка завершения шейдера %d: '%s'\n", ShaderType, InfoLog);
        exit(1);
    }

    glAttachShader(ShaderProgram, ShaderObj);
}

static void CompileShaders()//Создаем новую программу шейдеров.
{
    GLuint ShaderProgram = glCreateProgram();

    if (ShaderProgram == 0)//Проверяем, была ли создана программа шейдеров. 
    {
        fprintf(stderr, "Ошибка создания шейдера программы \n");
        exit(1);
    }
    //Добавляем вершинный шейдер в программу. Добавляем фрагментный шейдер в программу
    AddShader(ShaderProgram, pVS, GL_VERTEX_SHADER);
    AddShader(ShaderProgram, pFS, GL_FRAGMENT_SHADER);
    //Объявляем переменные Success и ErrorLog, для проверки на ошибки при связывании шейдеров.
    GLint Success = 0;
    GLchar ErrorLog[1024] = { 0 };
    //Связываем шейдеры программы.
    glLinkProgram(ShaderProgram);
    glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);//Получаем информацию о состоянии связывания шейдеров.
    if (Success == 0) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);//Если связывание шейдеров не было успешно, выводим сообщение об ошибке
        fprintf(stderr, "Ошибка подключения шейдера программ: '%s'\n", ErrorLog);
        exit(1);
    }

    glValidateProgram(ShaderProgram); // Проверяем корректность программы шейдеров.
    glGetProgramiv(ShaderProgram, GL_VALIDATE_STATUS, &Success); //Получаем информацию о состоянии проверки корректности программы шейдеров.
    if (!Success) {
        glGetProgramInfoLog(ShaderProgram, sizeof(ErrorLog), NULL, ErrorLog);
        fprintf(stderr, "неверный  шейдер программ: '%s'\n", ErrorLog);// Если программа шейдеров некорректна, выводим сообщение об ошибке.
        exit(1);
    }

    glUseProgram(ShaderProgram); //Устанавливаем данную программу шейдеров в качестве текущей.
    //Получаем местоположение uniform-переменной gWorld в программе шейдеров.
    gWorldLocation = glGetUniformLocation(ShaderProgram, "gWorld");
    assert(gWorldLocation != 0xFFFFFFFF);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv); //Инициализация GLUT, передача аргументов командной строки для инициализации.
    //Установка режима отображения окна GLUT с использованием двойной буферизации и RGBA цветовой модели.
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(1024, 768);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("mashtab");
    //Вызов функции InitializeGlutCallbacks() для установки обратных вызовов GLUT.
    InitializeGlutCallbacks();


    GLenum res = glewInit();
    if (res != GLEW_OK) {
        fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
        return 1;
    }

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    CreateVertexBuffer();

    CompileShaders();

    glutMainLoop();

    return 0;
}
