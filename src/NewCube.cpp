/* Hello Triangle - código adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Código modificado por: Nathaly Loggiovini
 * para as disciplinas de Computação Gráfica - Unisinos
 * Versão inicial:  25/03/2026
 * Versão final: 07/03/2025
 */
 
#include <iostream>
#include <string>
#include <assert.h>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

//GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader();
int setupGeometry();

// Dimensões da janela
const GLuint WIDTH = 1000, HEIGHT = 1000;

// Código fonte do Vertex Shader
const GLchar* vertexShaderSource = "#version 450\n"
"layout (location = 0) in vec3 position;\n"
"layout (location = 1) in vec3 color;\n"
"uniform mat4 model;\n"
"out vec4 finalColor;\n"
"void main()\n"
"{\n"
"gl_Position = model * vec4(position, 1.0);\n"
"finalColor = vec4(color, 1.0);\n"
"}\0";

//Código fonte do Fragment Shader
const GLchar* fragmentShaderSource = "#version 450\n"
"in vec4 finalColor;\n"
"out vec4 color;\n"
"void main()\n"
"{\n"
"color = finalColor;\n"
"}\n\0";

bool rotateX=false, rotateY=false, rotateZ=false;
//Novas Variáveis para posição
float translateX=0.0f, translateY=0.0f, translateZ=0.0f;

int main()
{
    glfwInit();
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Ola 3D --Nathaly!", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    glfwSetKeyCallback(window, key_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }

    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);

    GLuint shaderID = setupShader();
    GLuint VAO = setupGeometry();

    glUseProgram(shaderID);

    glm::mat4 model = glm::mat4(1);
    GLint modelLoc = glGetUniformLocation(shaderID, "model");
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    glEnable(GL_DEPTH_TEST);

    // Posições para 5 cubos na cena
    glm::vec3 cubePositions[] = {
        glm::vec3( 0.0f,  0.0f,  0.0f),  // Cubo central
        glm::vec3( 2.0f,  1.0f, -3.0f),  // Cubo na direita, cima, fundo
        glm::vec3(-1.5f, -1.0f, -1.5f),  // Cubo na esquerda, baixo
        glm::vec3(-2.0f,  2.0f, -2.0f),  // Cubo na esquerda, cima
        glm::vec3( 1.5f, -1.5f, -1.0f)   // Cubo na direita, baixo
    };

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glLineWidth(10);
        glPointSize(5); // Reduzido de 20 para 5 para não encobrir as faces coloridas

        float angle = (GLfloat)glfwGetTime();

        glBindVertexArray(VAO);
        
        // Loop iterando sobre os 5 cubos
        for(unsigned int i = 0; i < 5; i++)
        {
            model = glm::mat4(1.0f); 
            
            // 1. Escala: Reduz aproox. 30% para que os cubos das bordas não sejam cortados da tela
            model = glm::scale(model, glm::vec3(0.3f, 0.3f, 0.3f));

            // 2. Translação global (Move todos juntos com as teclas WASD + IO (Up/Down))
            model = glm::translate(model, glm::vec3(translateX, translateY, translateZ));

            // 3. Posicionamento individual (lê a posição do cubo 'i' no array)
            model = glm::translate(model, cubePositions[i]);

            // 4. Rotação estática inicial (Inclina um pouco para evidenciar o 3D)
            model = glm::rotate(model, glm::radians(35.0f), glm::vec3(1.0f, 1.0f, 0.0f));

            // 5. Rotação dinâmica via teclado (X, Y, Z)
            if (rotateX)
                model = glm::rotate(model, angle, glm::vec3(1.0f, 0.0f, 0.0f));
            else if (rotateY)
                model = glm::rotate(model, angle, glm::vec3(0.0f, 1.0f, 0.0f));
            else if (rotateZ)
                model = glm::rotate(model, angle, glm::vec3(0.0f, 0.0f, 1.0f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
            
            // Desenha os 36 vértices do cubo atual (Polígonos preenchidos)
            glDrawArrays(GL_TRIANGLES, 0, 36);

            // Desenha os 36 vértices do cubo atual como pontos (Contorno)
            glDrawArrays(GL_POINTS, 0, 36);
        }
        
        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }
    
    glDeleteVertexArrays(1, &VAO);
    glfwTerminate();
    return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // Velocidade do movimento a cada clique/frame
    float moveSpeed = 0.05f;

    // Translação no eixo Z (Profundidade)
    if (key == GLFW_KEY_W && (action == GLFW_PRESS || action == GLFW_REPEAT))
        translateZ -= moveSpeed;
    if (key == GLFW_KEY_S && (action == GLFW_PRESS || action == GLFW_REPEAT))
        translateZ += moveSpeed;

    // Translação no eixo X (Horizontal)
    if (key == GLFW_KEY_A && (action == GLFW_PRESS || action == GLFW_REPEAT))
        translateX -= moveSpeed;
    if (key == GLFW_KEY_D && (action == GLFW_PRESS || action == GLFW_REPEAT))
        translateX += moveSpeed;

    // Translação no eixo Y (Vertical)
    if (key == GLFW_KEY_I && (action == GLFW_PRESS || action == GLFW_REPEAT))
        translateY += moveSpeed;
    if (key == GLFW_KEY_O && (action == GLFW_PRESS || action == GLFW_REPEAT))
        translateY -= moveSpeed;
        //Movimentação com as setas do teclado
        
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    if (key == GLFW_KEY_X && action == GLFW_PRESS)
    { rotateX = true; rotateY = false; rotateZ = false; }
    if (key == GLFW_KEY_Y && action == GLFW_PRESS)
    { rotateX = false; rotateY = true; rotateZ = false; }
    if (key == GLFW_KEY_Z && action == GLFW_PRESS)
    { rotateX = false; rotateY = false; rotateZ = true; }
}

int setupShader()
{
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

int setupGeometry()
{
    GLfloat vertices[] = {
        // Face Frontal (Roxo Lavanda)
        -0.5f, -0.5f,  0.5f,  0.7f,  0.5f,  0.9f,
         0.5f, -0.5f,  0.5f,  0.7f,  0.5f,  0.9f,
         0.5f,  0.5f,  0.5f,  0.7f,  0.5f,  0.9f,
         0.5f,  0.5f,  0.5f,  0.7f,  0.5f,  0.9f,
        -0.5f,  0.5f,  0.5f,  0.7f,  0.5f,  0.9f,
        -0.5f, -0.5f,  0.5f,  0.7f,  0.5f,  0.9f,

        // Face Traseira (Verde Água)
        -0.5f, -0.5f, -0.5f,  0.2f,  0.8f,  0.8f,
        -0.5f,  0.5f, -0.5f,  0.2f,  0.8f,  0.8f,
         0.5f,  0.5f, -0.5f,  0.2f,  0.8f,  0.8f,
         0.5f,  0.5f, -0.5f,  0.2f,  0.8f,  0.8f,
         0.5f, -0.5f, -0.5f,  0.2f,  0.8f,  0.8f,
        -0.5f, -0.5f, -0.5f,  0.2f,  0.8f,  0.8f,

        // Face Esquerda (Azul Escuro)
        -0.5f,  0.5f,  0.5f,  0.1f,  0.1f,  0.6f,
        -0.5f,  0.5f, -0.5f,  0.1f,  0.1f,  0.6f,
        -0.5f, -0.5f, -0.5f,  0.1f,  0.1f,  0.6f,
        -0.5f, -0.5f, -0.5f,  0.1f,  0.1f,  0.6f,
        -0.5f, -0.5f,  0.5f,  0.1f,  0.1f,  0.6f,
        -0.5f,  0.5f,  0.5f,  0.1f,  0.1f,  0.6f,

        // Face Direita (Amarelo)
         0.5f,  0.5f,  0.5f,  1.0f,  1.0f,  0.0f,
         0.5f, -0.5f,  0.5f,  1.0f,  1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  1.0f,  0.0f,
         0.5f, -0.5f, -0.5f,  1.0f,  1.0f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  1.0f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  1.0f,  0.0f,

        // Face Superior (Laranja)
        -0.5f,  0.5f, -0.5f,  1.0f,  0.5f,  0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f,  0.5f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.5f,  0.0f,
         0.5f,  0.5f,  0.5f,  1.0f,  0.5f,  0.0f,
         0.5f,  0.5f, -0.5f,  1.0f,  0.5f,  0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f,  0.5f,  0.0f,

        // Face Inferior (Vermelho)
        -0.5f, -0.5f, -0.5f,  0.9f,  0.1f,  0.1f,
         0.5f, -0.5f, -0.5f,  0.9f,  0.1f,  0.1f,
         0.5f, -0.5f,  0.5f,  0.9f,  0.1f,  0.1f,
         0.5f, -0.5f,  0.5f,  0.9f,  0.1f,  0.1f,
        -0.5f, -0.5f,  0.5f,  0.9f,  0.1f,  0.1f,
        -0.5f, -0.5f, -0.5f,  0.9f,  0.1f,  0.1f
    };

    GLuint VBO, VAO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3*sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    return VAO;
}