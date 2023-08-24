#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include "glad/glad.h"
#include "GLFW/glfw3.h"

struct ShaderProgramSource
{
    std::string VertexSource;
    std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filePath)
{
    std::ifstream stream(filePath); 
    enum class ShaderType {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };

    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;
    while (getline(stream, line)) {
        if (line.find("#shader") != std::string::npos) { 
            if (line.find("vertex") != std::string::npos) { 
                type = ShaderType::VERTEX;
            }
            else if (line.find("fragment") != std::string::npos) {
                type = ShaderType::FRAGMENT;
            }
        }
        else {
            ss[(int)type] << line << '\n';
        }
    }
    return { ss[0].str(),ss[1].str() };
}

static unsigned int CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type); 
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id); 

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result); // 获取当前着色器编译状态
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length); // 获取日志长度
        char* msg = (char*)_malloca(length * sizeof(char)); /* Cherno这里采用的alloca, 根据IDE提示, 我这里改成了_malloca函数 */
        glGetShaderInfoLog(id, length, &length, msg); // 获取日志信息
        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << " shader!" << std::endl;
        std::cout << msg << std::endl;
        glDeleteShader(id); // 删除着色器
        return 0;
    }

    return id;
}

static int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int program = glCreateProgram(); /* 创建程序 */
    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    /* 将着色器附加到程序上 */
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program); /* 链接程序 */
    glValidateProgram(program); /* 验证 */

    /* 删除着色器 */
    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

int main(void)
{
    GLFWwindow* window;

    if (!glfwInit()) return -1;

    /// 创建窗口模式窗口及其 OpenGL 上下文 
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

     // 使窗口的上下文内容为当前内容
    glfwMakeContextCurrent(window);

    // 初始化glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    /* 顶点位置浮点型数组 */
    float positions[] = {
        -0.5f, -0.5f, // 0
        0.5f, -0.5f,  // 1
        0.5f, 0.5f,   // 2
        -0.5f, 0.5f,  // 3
    };

    /* 索引缓冲区所需索引数组 */
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    unsigned int buffer;
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer); 
    glBufferData(GL_ARRAY_BUFFER, 4 * 2 * sizeof(float), positions, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0); 
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0); 

    unsigned int ibo;
    glGenBuffers(1, &ibo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW);

    /// 从文件中解析着色器源码 
    ShaderProgramSource source = ParseShader("res/shaders/Basic.shader");
    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);

    glUseProgram(shader); 

    
    while (!glfwWindowShouldClose(window)) {
        

        glClear(GL_COLOR_BUFFER_BIT);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        
        glfwSwapBuffers(window);

        glfwPollEvents();
    }
    glDeleteProgram(shader); /* 删除着色器程序 */

    glfwTerminate();
    return 0;
}