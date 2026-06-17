#include <iostream>
#include <string>
#include <assert.h>
#include <vector>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <cmath>

using namespace std;

// GLAD e GLFW
#include <glad/glad.h>
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace glm;

enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT };

// Encapsula todos os atributos e métodos relacionados à câmera em primeira pessoa.
// Isola a matemática de vetores e matrizes do loop principal da aplicação.
class Pathway {
public:
    // Atributos de posicionamento e orientação
    glm::vec3 Position, Front, Up, Right, WorldUp;
    // Ângulos de Euler e configurações de controle
    float Yaw, Pitch, MovementSpeed, MouseSensitivity, Zoom;

    Pathway(glm::vec3 position = glm::vec3(0.0f, 0.0f, 3.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = -90.0f, float pitch = 0.0f) 
        : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(2.5f), MouseSensitivity(0.05f), Zoom(45.0f) {
        Position = position;
        WorldUp = up;
        Yaw = yaw;
        Pitch = pitch;
        updateCameraVectors(); // Calcula os vetores iniciais com base nos ângulos padrão
    }

    // Constrói e retorna a matriz de View utilizando glm::lookAt
    glm::mat4 GetViewMatrix() {
        return glm::lookAt(Position, Position + Front, Up);
    }

    // Multiplicar pela velocidade (MovementSpeed) e deltaTime garante independência de framerate
    void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
        float velocity = MovementSpeed * deltaTime;
        if (direction == FORWARD) Position += Front * velocity;
        if (direction == BACKWARD) Position -= Front * velocity;
        if (direction == LEFT) Position -= Right * velocity;
        if (direction == RIGHT) Position += Right * velocity;
    }

    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
        Yaw += xoffset * MouseSensitivity;
        Pitch += yoffset * MouseSensitivity;
        if (constrainPitch) {
            if (Pitch > 89.0f) Pitch = 89.0f;
            if (Pitch < -89.0f) Pitch = -89.0f;
        }
        updateCameraVectors();
    }

    // Altera o nível de zoom (Field of View) através do Scroll do mouse
    void ProcessMouseScroll(float yoffset) {
        Zoom -= yoffset;
        if (Zoom < 1.0f) Zoom = 1.0f;
        if (Zoom > 45.0f) Zoom = 45.0f;
    }

private:
    void updateCameraVectors() {
        glm::vec3 front;
        front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        front.y = sin(glm::radians(Pitch));
        front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
        Front = glm::normalize(front);
        Right = glm::normalize(glm::cross(Front, WorldUp));  
        Up    = glm::normalize(glm::cross(Right, Front));
    }
};

const GLuint WIDTH = 800, HEIGHT = 600;

// Instância da Câmera
Pathway camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
bool firstMouse = true;

// Controle de tempo
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Shader sources... (Com integração da matriz View e Projection)
const GLchar *vertexShaderSource = R"(
#version 400
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texc;

uniform mat4 projection;
uniform mat4 view; // <-- Adicionado para a câmera funcionar
uniform mat4 model;

out vec2 texCoord;
void main()
{
    // A ordem de multiplicação importa: projection * view * model
    gl_Position = projection * view * model * vec4(position, 1.0);
    texCoord = texc;
})";

const GLchar *fragmentShaderSource = R"(
#version 400
in vec2 texCoord;
uniform sampler2D texBuff;
out vec4 color;
void main()
{
    color = texture(texBuff,texCoord);
})";

//Setup funções callback
int setupShader();
GLuint loadTexture(string filePath, int &width, int &height);
string getTexturePathFromMTL(string mtlFilePath);
int loadSimpleOBJ(string filePath, int &nVertices, string &textureName);

// Ccalcula o deslocamento do mouse para atualizar a direção da câmera
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; //innvertido: as coordenadas Y vão do topo (0) para baixo
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}

// variáveis globais para controle da trajetória
std::vector<glm::vec3> controlPoints;
int currentPoint = 0;
float trajectoryT = 0.0f;
float trajectorySpeed = 0.5f; // Ajustando a velocidade de movimento

// Processa as teclas pressionadas e repassa para a câmera junto com o deltaTime
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    static bool keyP_wasPressed = false;
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {
        if (!keyP_wasPressed) {
            // Adiciona um novo ponto de controle na posição atual da câmera
            controlPoints.push_back(camera.Position);
            std::cout << "Ponto Salvo (" << camera.Position.x << ","
                      << camera.Position.y << "," << camera.Position.z 
                      << ")! Total: " << controlPoints.size() << std::endl;
            keyP_wasPressed = true;
        }
    } else {
        keyP_wasPressed = false;
    }
}

// Função de interpolação catmull_uniform para trajetórias suaves entre os pontos de controle
// Tradução da função catmull_uniform do Python para C++ (GLM) (Seguindo exemplo compartilhado pelo Professor)
glm::vec3 catmull_uniform(glm::vec3 P0, glm::vec3 P1, glm::vec3 P2, glm::vec3 P3, float t) {
    // Pré-calcula as potências do parâmetro paramétrico 't' (t^2 e t^3) para otimização
    float t2 = t * t;
    float t3 = t2 * t;
    // Equação polinomial de grau 3 que define a coordenada 3D exata na curva.
    // Os vizinhos extremos (P0 e P3) atuam apenas puxando as tangentes para orientar a curvatura.
    return 0.5f * (
        (2.0f * P1) // Posição inicial fixa (quando t = 0.0)
        + (-P0 + P2) * t
        + (2.0f * P0 - 5.0f * P1 + 4.0f * P2 - P3) * t2
        + (-P0 + 3.0f * P1 - 3.0f * P2 + P3) * t3 // Ajuste final da tangente ao chegar em P2
    );
}

//função main 
int main()
{
    glfwInit();

    GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Câmera FPS e Texturas!", nullptr, nullptr);
    glfwMakeContextCurrent(window);

    // Configurando os callbacks de input para a Câmera
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // Oculta o cursor do mouse e captura para a janela
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glViewport(0, 0, WIDTH, HEIGHT);
    glEnable(GL_DEPTH_TEST);

    GLuint shaderID = setupShader();

    int nVertices;
    std::string textureFileName;
    GLuint VAO = loadSimpleOBJ("../assets/Modelos3D/Suzanne.obj", nVertices, textureFileName);

    int imgWidth, imgHeight;
    GLuint texID;
    if (!textureFileName.empty()) {
        std::string texturePath = "../assets/Modelos3D/" + textureFileName;
        texID = loadTexture(texturePath, imgWidth, imgHeight);
    }

    glUseProgram(shaderID);
    glUniform1i(glGetUniformLocation(shaderID, "texBuff"), 0);

    // Game loop
    while (!glfwWindowShouldClose(window))
    {
        // Calcula o deltaTime
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Processa as teclas WASD e ESC
        processInput(window);
        glfwPollEvents();

        // Limpa a tela (Fundo roxo lavanda)
        glClearColor(0.80f, 0.70f, 0.90f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texID);
        glBindVertexArray(VAO);

        // Cálculo da traetória cíclica suave (catmull-rom)
        // Requisito atendido: realiazndo a translação do objeto por pontos salvos de maneira cíclica 
        glm::vec3 objectPos = glm::vec3(0.0f, 0.0f, 0.0f);
        int n = controlPoints.size();

        if (n >= 2) { 
        // A curva de catmull-rom pede 4 pontos simultâneos para desenhar 1 segmento (P0, P1, P2, P3)
        // O operador de módulo (%) é basciamente o coração da trajetória cíclica: ele força os índices 
        // a "darem a volta" para o índice 0 assim que atingem o final do vetor (tamanho 'n')

        // Ponto de controle anterior. E se soma 'n' antes do módulo para evitar índices negativos no C++
            int i0 = (currentPoint - 1 + n) % n;
        // Ponto atual (P1)   
            int i1 = currentPoint;
        // Próximo ponto (P2)
            int i2 = (currentPoint + 1) % n;
        // Próximo próximo ponto (P3)
            int i3 = (currentPoint + 2) % n;
            
        // Calcula a posição do frame atual interpolando os 4 pontos de controle com base no tempo 't'
            objectPos = catmull_uniform(
                controlPoints[i0], controlPoints[i1], 
                controlPoints[i2], controlPoints[i3], trajectoryT
            );
            
            // Incrementa 't' baseado no deltaTime, 
            // garantindo velocidade constante idependente de framerate
            trajectoryT += trajectorySpeed * deltaTime;
            
            // Quando t atinge 1.0f, o objeto chegou perfeitamente no ponto i2.
            // Ao chegar em P2 (t=1.0), avança o foco para o próximo segmento da curva
            if (trajectoryT >= 1.0f) {
                trajectoryT = 0.0f; 
                currentPoint = i2; 
            }
        } else if (n == 1) {
            objectPos = controlPoints[0]; // Permanece estático se apenas um ponto existir
        }

        // 1. Matriz de Projeção (Perspectiva)
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
        glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        // 2. Matriz de Visualização (Câmera)
        glm::mat4 view = camera.GetViewMatrix();
        glUniformMatrix4fv(glGetUniformLocation(shaderID, "view"), 1, GL_FALSE, glm::value_ptr(view));

        // 3. Matriz de Modelo (Posicionamento do OBJ)
        mat4 model = mat4(1.0f);
        model = translate(model, objectPos); 
        model = scale(model, vec3(1.0f, 1.0f, 1.0f)); 
        glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));

        // Desenho :)
        glDrawArrays(GL_TRIANGLES, 0, nVertices);

        glBindVertexArray(0);
        glfwSwapBuffers(window);
    }

    glDeleteVertexArrays(1, &VAO);
    glfwTerminate();
    return 0;
}
//função auxiliares 
int setupShader() {
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    GLint success; GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

GLuint loadTexture(string filePath, int &width, int &height) {
    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

    if (data) {
        if (nrChannels == 3) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        else glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture " << filePath << std::endl;
    }
    stbi_image_free(data);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texID;
}

string getTexturePathFromMTL(string mtlFilePath) {
    std::ifstream file(mtlFilePath);
    if (!file.is_open()) return "";

    std::string line, texPath;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string prefix;
        ss >> prefix;
        if (prefix == "map_Kd") {
            ss >> texPath;
            break;
        }
    }
    return texPath;
}

int loadSimpleOBJ(string filePath, int &nVertices, string &textureName) {
    std::vector<vec3> temp_vertices;
    std::vector<vec2> temp_texCoords;
    std::vector<GLfloat> vBuffer;

    std::ifstream file(filePath);
    if (!file.is_open()) {
        std::cerr << "Erro ao tentar abrir o arquivo " << filePath << std::endl;
        return -1;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "mtllib") {
            std::string mtlFile;
            ss >> mtlFile;
            std::string dir = filePath.substr(0, filePath.find_last_of("/") + 1);
            textureName = getTexturePathFromMTL(dir + mtlFile);     
        }
        else if (prefix == "v") {
            vec3 pos;
            ss >> pos.x >> pos.y >> pos.z;
            temp_vertices.push_back(pos);   
        }
        else if (prefix == "vt") {
            vec2 tex;
            ss >> tex.x >> tex.y;
            temp_texCoords.push_back(tex);
        }
        else if (prefix == "f") {
            std::string vertexData;
            for (int i = 0; i < 3; i++) {
                ss >> vertexData;
                std::replace(vertexData.begin(), vertexData.end(), '/', ' ');
                std::istringstream faceSS(vertexData);
                unsigned int vIdx, tIdx;

                faceSS >> vIdx >> tIdx; 

                vec3 pos = temp_vertices[vIdx - 1];
                vec2 tex = temp_texCoords[tIdx - 1];

                vBuffer.push_back(pos.x);
                vBuffer.push_back(pos.y);
                vBuffer.push_back(pos.z);
                vBuffer.push_back(tex.x);
                vBuffer.push_back(tex.y);
            }
        }
    }

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vBuffer.size() * sizeof(GLfloat), vBuffer.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    nVertices = vBuffer.size() / 5;
    return VAO;
}