/* Hello Triangle - código adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 *
 * Adaptado para a disciplina de Computação Gráfica - Unisinos
 * Configurado para Sistema de Iluminação de 3 Pontos
 * Alunas: Nathaly Loggiovini, Isadora Albano
 */

#include <iostream>
#include <string>
#include <assert.h>
#include <vector>

using namespace std;

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace glm;

#include <cmath>

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader();
int setupGeometry();
GLuint loadTexture(string filePath, int &width, int &height);
void drawGeometry(GLuint shaderID, GLuint VAO, vec3 position, vec3 dimensions, float angle, int nVertices, vec3 color = vec3(1.0,0.0,0.0), vec3 axis = (vec3(0.0, 0.0, 1.0)));
GLuint generateSphere(float radius, int latSegments, int lonSegments, int &nVertices);
 
// Dimensões da janela
const GLuint WIDTH = 800, HEIGHT = 800;

// Variáveis globais para controle das luzes (1 = Ligado, 0 = Desligado)
vec3 lightsOn = vec3(1.0f, 1.0f, 1.0f); // x: Key, y: Fill, z: Back

// Código fonte do Vertex Shader
const GLchar *vertexShaderSource = R"(
#version 400
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 color;
layout (location = 2) in vec3 normal;
layout (location = 3) in vec2 texc;

uniform mat4 projection;
uniform mat4 model;

out vec2 texCoord;
out vec3 vNormal;
out vec4 fragPos; 
out vec4 vColor;
void main()
{
   	gl_Position = projection * model * vec4(position, 1.0);
	fragPos = model * vec4(position, 1.0);
	texCoord = texc;
	vNormal = mat3(transpose(inverse(model))) * normal;
	vColor = vec4(color,1.0);
})";

// Código fonte do Fragment Shader com 3 fontes de luz e atenuação
const GLchar *fragmentShaderSource = R"(
#version 400
in vec2 texCoord;
uniform sampler2D texBuff;

uniform vec3 keyLightPos;
uniform vec3 fillLightPos;
uniform vec3 backLightPos;

uniform vec3 keyLightColor;
uniform vec3 fillLightColor;
uniform vec3 backLightColor;

uniform vec3 lightsOn;

uniform vec3 camPos;
uniform float ka;
uniform float kd;
uniform float ks;
uniform float q;

out vec4 color;
in vec4 fragPos;
in vec3 vNormal;
in vec4 vColor;

vec3 calculateLight(vec3 lightPos, vec3 lightColor, vec3 N, vec3 V, float isLightOn)
{
	if (isLightOn < 0.5) return vec3(0.0);

	vec3 L = lightPos - vec3(fragPos);
	float distance = length(L);
	L = normalize(L);

	float attenuation = 1.0 / (1.0 + 0.5 * distance + 0.25 * (distance * distance));

	float diff = max(dot(N, L), 0.0);
	vec3 diffuse = (kd * diff * lightColor) * attenuation;

	vec3 R = normalize(reflect(-L, N));
	float spec = max(dot(R, V), 0.0);
	spec = pow(spec, q);
	vec3 specular = ks * spec * lightColor;

	return diffuse + specular;
}

void main()
{
	vec4 objectColor = vColor;
	vec3 N = normalize(vNormal);
	vec3 V = normalize(camPos - vec3(fragPos));

	vec3 ambient = ka * vec3(1.0, 1.0, 1.0) * vec3(objectColor);

	vec3 lightingResult = vec3(0.0);
	lightingResult += calculateLight(keyLightPos, keyLightColor, N, V, lightsOn.x);
	lightingResult += calculateLight(fillLightPos, fillLightColor, N, V, lightsOn.y);
	lightingResult += calculateLight(backLightPos, backLightColor, N, V, lightsOn.z);

	vec3 finalColor = ambient + (lightingResult * vec3(objectColor));
	color = vec4(finalColor, 1.0);
})";

int main()
{
	glfwInit();

	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Iluminacao de Tres Pontos", nullptr, nullptr);
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

	int nVertices;
	GLuint VAO = generateSphere(0.5, 32, 32, nVertices);

	int imgWidth, imgHeight;
	GLuint texID = loadTexture("../assets/tex/pixelWall.png", imgWidth, imgHeight);

	float ka = 0.05, kd = 0.6, ks = 0.4, q = 20.0;
	vec3 camPos = vec3(0.0, 0.0, 2.0);

	vec3 objectPos = vec3(0.0f, 0.0f, 0.0f);
	vec3 objectScale = vec3(1.0f, 1.0f, 1.0f);

	float offset = objectScale.x * 1.5f; 
	
	vec3 keyLightPos  = objectPos + vec3(-offset,  offset,  offset);
	vec3 fillLightPos = objectPos + vec3( offset,  offset * 0.5f,  offset);
	vec3 backLightPos = objectPos + vec3( 0.0f,    offset, -offset);

	vec3 keyLightColor  = vec3(1.0f, 0.95f, 0.9f);
	vec3 fillLightColor = vec3(0.4f, 0.4f, 0.45f);
	vec3 backLightColor = vec3(0.8f, 0.8f, 0.8f);

	glUseProgram(shaderID);

	glUniform1i(glGetUniformLocation(shaderID, "texBuff"), 0);
	glUniform1f(glGetUniformLocation(shaderID, "ka"), ka);
	glUniform1f(glGetUniformLocation(shaderID, "kd"), kd);
	glUniform1f(glGetUniformLocation(shaderID, "ks"), ks);
	glUniform1f(glGetUniformLocation(shaderID, "q"), q);
	glUniform3f(glGetUniformLocation(shaderID, "camPos"), camPos.x, camPos.y, camPos.z);

	glUniform3f(glGetUniformLocation(shaderID, "keyLightPos"), keyLightPos.x, keyLightPos.y, keyLightPos.z);
	glUniform3f(glGetUniformLocation(shaderID, "fillLightPos"), fillLightPos.x, fillLightPos.y, fillLightPos.z);
	glUniform3f(glGetUniformLocation(shaderID, "backLightPos"), backLightPos.x, backLightPos.y, backLightPos.z);

	glUniform3f(glGetUniformLocation(shaderID, "keyLightColor"), keyLightColor.x, keyLightColor.y, keyLightColor.z);
	glUniform3f(glGetUniformLocation(shaderID, "fillLightColor"), fillLightColor.x, fillLightColor.y, fillLightColor.z);
	glUniform3f(glGetUniformLocation(shaderID, "backLightColor"), backLightColor.x, backLightColor.y, backLightColor.z);

	glActiveTexture(GL_TEXTURE0);
	
	mat4 projection = perspective(radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
	mat4 view = lookAt(camPos, objectPos, vec3(0.0f, 1.0f, 0.0f));
	mat4 pv = projection * view;
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, value_ptr(pv));

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		glUniform3f(glGetUniformLocation(shaderID, "lightsOn"), lightsOn.x, lightsOn.y, lightsOn.z);

		glClearColor(0.05f, 0.05f, 0.05f, 1.0f); 
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);

		glBindVertexArray(VAO);
		glBindTexture(GL_TEXTURE_2D, texID);

		drawGeometry(shaderID, VAO, objectPos, objectScale, 0.0, nVertices);
	
		glBindVertexArray(0);
		glfwSwapBuffers(window);
	}

	glDeleteVertexArrays(1, &VAO);
	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
		lightsOn.x = (lightsOn.x == 1.0f) ? 0.0f : 1.0f;
		cout << "Key Light: " << (lightsOn.x ? "LIGADA" : "DESLIGADA") << endl;
	}
	if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
		lightsOn.y = (lightsOn.y == 1.0f) ? 0.0f : 1.0f;
		cout << "Fill Light: " << (lightsOn.y ? "LIGADA" : "DESLIGADA") << endl;
	}
	if (key == GLFW_KEY_3 && action == GLFW_PRESS) {
		lightsOn.z = (lightsOn.z == 1.0f) ? 0.0f : 1.0f;
		cout << "Back Light: " << (lightsOn.z ? "LIGADA" : "DESLIGADA") << endl;
	}
}

int setupShader()
{
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

GLuint loadTexture(string filePath, int &width, int &height)
{
	GLuint texID;
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int nrChannels;
	unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

	if (data)
	{
		if (nrChannels == 3)
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		else
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture " << filePath << std::endl;
	}

	stbi_image_free(data);
	glBindTexture(GL_TEXTURE_2D, 0);
	return texID;
}

void drawGeometry(GLuint shaderID, GLuint VAO, vec3 position, vec3 dimensions, float angle, int nVertices, vec3 color, vec3 axis)
{
	mat4 model = mat4(1);
	model = translate(model, position);
	model = rotate(model, radians(angle), axis);
	model = scale(model, dimensions);
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));

	glDrawArrays(GL_TRIANGLES, 0, nVertices);
}

GLuint generateSphere(float radius, int latSegments, int lonSegments, int &nVertices) {
    vector<GLfloat> vBuffer;
    vec3 color = vec3(1.0f, 0.5f, 0.0f); // Laranja para destacar o sombreamento

    // Definição manual e direta do PI para evitar erros de include de constantes do GLM
    const float PI_VAL = 3.14159265359f;

    auto calcPosUVNormal = [&](int lat, int lon, vec3& pos, vec2& uv, vec3& normal) {
        float theta = lat * PI_VAL / latSegments;
        float phi = lon * 2.0f * PI_VAL / lonSegments;

        pos = vec3(
            radius * cos(phi) * sin(theta),
            radius * cos(theta),
            radius * sin(phi) * sin(theta)
        );

        uv = vec2(phi / (2.0f * PI_VAL), theta / PI_VAL);
        normal = normalize(pos);
    };

    // Monta a malha de triângulos da esfera
    for (int i = 0; i < latSegments; ++i) {
        for (int j = 0; j < lonSegments; ++j) {
            vec3 v0, v1, v2, v3;
            vec2 uv0, uv1, uv2, uv3;
            vec3 n0, n1, n2, n3;

            calcPosUVNormal(i, j, v0, uv0, n0);
            calcPosUVNormal(i + 1, j, v1, uv1, n1);
            calcPosUVNormal(i, j + 1, v2, uv2, n2);
            calcPosUVNormal(i + 1, j + 1, v3, uv3, n3);

            // Primeiro triângulo
            vBuffer.insert(vBuffer.end(), { v0.x, v0.y, v0.z, color.r, color.g, color.b, n0.x, n0.y, n0.z, uv0.x, uv0.y });
            vBuffer.insert(vBuffer.end(), { v1.x, v1.y, v1.z, color.r, color.g, color.b, n1.x, n1.y, n1.z, uv1.x, uv1.y });
            vBuffer.insert(vBuffer.end(), { v2.x, v2.y, v2.z, color.r, color.g, color.b, n2.x, n2.y, n2.z, uv2.x, uv2.y });

            // Segundo triângulo
            vBuffer.insert(vBuffer.end(), { v1.x, v1.y, v1.z, color.r, color.g, color.b, n1.x, n1.y, n1.z, uv1.x, uv1.y });
            vBuffer.insert(vBuffer.end(), { v3.x, v3.y, v3.z, color.r, color.g, color.b, n3.x, n3.y, n3.z, uv3.x, uv3.y });
            vBuffer.insert(vBuffer.end(), { v2.x, v2.y, v2.z, color.r, color.g, color.b, n2.x, n2.y, n2.z, uv2.x, uv2.y });
        }
    }

    // Configuração dos buffers na GPU
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vBuffer.size() * sizeof(GLfloat), vBuffer.data(), GL_STATIC_DRAW);

    // Atributos: Posição(0), Cor(1), Normal(2), UV(3)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(0));
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(GLfloat), (GLvoid*)(9 * sizeof(GLfloat)));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);
    nVertices = vBuffer.size() / 11;

    return VAO;
}