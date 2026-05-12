/* Hello Triangle - código adaptado de https://learnopengl.com/#!Getting-started/Hello-Triangle
 * Usando como referência o código do exercício anterior (Hello Triangle), este exemplo tem como objetivo mostrar como carregar e usar texturas em OpenGL. 
 * O código inicial foi adaptado pela Professora: Rossana Baptista Queiroz
 * 
 * Código modificado por: Nathaly Loggiovini
 * Última atualização do código: 11/05/2025 
 */

#include <iostream>
#include <string>
#include <assert.h>

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

#include <vector>
#include <algorithm>

#include <fstream>
#include <sstream>

// Protótipo da função de callback de teclado
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode);

// Protótipos das funções
int setupShader();
GLuint loadTexture(string filePath, int &width, int &height);

void drawTriangle(GLuint shaderID, GLuint VAO, vec3 position, vec3 dimensions, float angle, vec3 color, vec3 axis = (vec3(0.0, 0.0, 1.0)));

string getTexturePathFromMTL(string mtlFilePath);
int loadSimpleOBJ(string filePath, int &nVertices, string &textureName); // Protótipo da função principal de leitura 

// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 800, HEIGHT = 600;

// Variáveis globais para controlar a rotação via teclado
float angleX = 0.0f;
float angleY = 0.0f;
float angleZ = 0.0f;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
const GLchar *vertexShaderSource = R"(
#version 400
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texc;
uniform mat4 projection;
uniform mat4 model;
out vec2 texCoord;
void main()
{
   	gl_Position = projection * model * vec4(position.x, position.y, position.z, 1.0);
	texCoord = texc;
})";

// Código fonte do Fragment Shader (em GLSL): ainda hardcoded
const GLchar *fragmentShaderSource = R"(
#version 400
in vec2 texCoord;
uniform sampler2D texBuff;
out vec4 color;
void main()
{
	color = texture(texBuff,texCoord);
})";

// Função MAIN
int main()
{
	// Inicialização da GLFW
	glfwInit();

	// Muita atenção aqui: alguns ambientes não aceitam essas configurações
	// Você deve adaptar para a versão do OpenGL suportada por sua placa
	// Sugestão: comente essas linhas de código para desobrir a versão e
	// depois atualize (por exemplo: 4.5 com 4 e 5)
	/*glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);*/

	// Criação da janela GLFW
	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Ola Imagem Texturizado!", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	// Fazendo o registro da função de callback para a janela GLFW
	glfwSetKeyCallback(window, key_callback);

	// GLAD: carrega todos os ponteiros d funções da OpenGL
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
	}

	// Obtendo as informações de versão
	const GLubyte *renderer = glGetString(GL_RENDERER); /* get renderer string */
	const GLubyte *version = glGetString(GL_VERSION);	/* version as a string */
	cout << "Renderer: " << renderer << endl;
	cout << "OpenGL version supported " << version << endl;

	// Definindo as dimensões da viewport com as mesmas dimensões da janela da aplicação
	int width, height;
	glfwGetFramebufferSize(window, &width, &height);
	glViewport(0, 0, width, height);

	// Compilando e buildando o programa de shader
	GLuint shaderID = setupShader();

	// Lendo modelo .obj e criando o VAO 
	int nVertices;
	std::string textureFileName;

	// Caminho atualizado para o arquivo .obj real
	GLuint VAO = loadSimpleOBJ("../assets/Modelos3D/Suzanne.obj", nVertices, textureFileName);

	// Carregando a textura a partir do arquivo .mtl
	int imgWidth, imgHeight;
	GLuint texID;
	if (!textureFileName.empty()) {
		std::string texturePath = "../assets/Modelos3D/" + textureFileName;
		texID = loadTexture(texturePath, imgWidth, imgHeight);
	} else {
		std::cout << "Nenhuma textura encontrada no arquivo MTL. Exit." << std::endl;
	}

	glUseProgram(shaderID);

	// Enviar a informação de qual variável armazenará o buffer da textura
	glUniform1i(glGetUniformLocation(shaderID, "texBuff"), 0);

	//Ativando o primeiro buffer de textura da OpenGL
	glActiveTexture(GL_TEXTURE0);
	

	// Matriz de projeção paralela ortográfica
	// Decidi trocar -1.0, 1.0 por valores grandes para englobar o modelo escalado
	mat4 projection = ortho(0.0f, 800.0f, 0.0f, 600.0f, -1000.0f, 1000.0f);
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, value_ptr(projection));

	// Matriz de modelo: transformações na geometria (objeto)
	mat4 model = mat4(1); // matriz identidade
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));

	glEnable(GL_DEPTH_TEST);
	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(0.80f, 0.70f, 0.90f, 1.0f); // Fundo roxo lavanda <3
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  
		// Usei o  o operador OR bit a bit (|) para indicar que múltiplos buffers devem ser limpos ao mesmo tempo

		glBindVertexArray(VAO); // Conectando ao buffer de geometria
		glBindTexture(GL_TEXTURE_2D, texID); // conectando com o buffer de textura que será usado no draw

		// Define as transformações para o objeto carregado (centralizado e com tamanho ajustado)
        mat4 model = mat4(1.0f);
        // Exemplo: Centralizando na tela (dependendo das coordenadas do seu OBJ)
        model = translate(model, vec3(400.0f, 300.0f, 0.0f)); 

		// Aplicando as rotações controladas pelo teclado (x, y, z)
        model = rotate(model, angleX, vec3(1.0f, 0.0f, 0.0f));
        model = rotate(model, angleY, vec3(0.0f, 1.0f, 0.0f));
        model = rotate(model, angleZ, vec3(0.0f, 0.0f, 1.0f));

        model = scale(model, vec3(100.0f, 100.0f, 100.0f)); // Ajuste a escala conforme o tamanho do seu modelo
		
        glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));

        // Desenha o objeto completo (todos os triângulos lidos do OBJ)
        glDrawArrays(GL_TRIANGLES, 0, nVertices);

		glBindVertexArray(0); // Desconectando o buffer de geometria

		// Troca os buffers da tela
		glfwSwapBuffers(window);
	}
	// Pede pra OpenGL desalocar os buffers
	glDeleteVertexArrays(1, &VAO);
	// Finaliza a execução da GLFW, limpando os recursos alocados por ela
	glfwTerminate();
	return 0;
}

// Função de callback de teclado - só pode ter uma instância (deve ser estática se
// estiver dentro de uma classe) - É chamada sempre que uma tecla for pressionada
// ou solta via GLFW
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mode)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
	
	// Controle de Rotação (Aciona segurando a tecla)
    if (key == GLFW_KEY_X && (action == GLFW_PRESS || action == GLFW_REPEAT))
        angleX += 5.0f;
    if (key == GLFW_KEY_Y && (action == GLFW_PRESS || action == GLFW_REPEAT))
        angleY += 5.0f;
    if (key == GLFW_KEY_Z && (action == GLFW_PRESS || action == GLFW_REPEAT))
        angleZ += 5.0f;
}

// Esta função está basntante hardcoded - objetivo é compilar e "buildar" um programa de
//  shader simples e único neste exemplo de código
//  O código fonte do vertex e fragment shader está nos arrays vertexShaderSource e
//  fragmentShader source no iniçio deste arquivo
//  A função retorna o identificador do programa de shader
int setupShader()
{
	// Vertex shader
	GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// Checando erros de compilação (exibição via log no terminal)
	GLint success;
	GLchar infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}
	// Fragment shader
	GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// Checando erros de compilação (exibição via log no terminal)
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
				  << infoLog << std::endl;
	}
	// Linkando os shaders e criando o identificador do programa de shader
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// Checando por erros de linkagem
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success)
	{
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
				  << infoLog << std::endl;
	}
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	return shaderProgram;
}

GLuint loadTexture(string filePath, int &width, int &height)
{
	GLuint texID; // id da textura a ser carregada

	// Gera o identificador da textura na memória
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);

	// Ajuste dos parâmetros de wrapping e filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Carregamento da imagem usando a função stbi_load da biblioteca stb_image
	int nrChannels;
	stbi_set_flip_vertically_on_load(true);

	unsigned char *data = stbi_load(filePath.c_str(), &width, &height, &nrChannels, 0);

	if (data)
	{
		if (nrChannels == 3) // jpg, bmp
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
		}
		else // assume que é 4 canais png
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		}
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

void drawTriangle(GLuint shaderID, GLuint VAO, vec3 position, vec3 dimensions, float angle, vec3 color, vec3 axis)
{
	// Matriz de modelo: transformações na geometria (objeto)
	mat4 model = mat4(1); // matriz identidade
	// Translação
	model = translate(model, position);
	// Rotação
	model = rotate(model, radians(angle), axis);
	// Escala
	model = scale(model, dimensions);
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));

	glUniform4f(glGetUniformLocation(shaderID, "inputColor"), color.r, color.g, color.b, 1.0f); // enviando cor para variável uniform inputColor
																								//  Chamada de desenho - drawcall
																								//  Poligono Preenchido - GL_TRIANGLES
	glDrawArrays(GL_TRIANGLES, 0, 3);
}

string getTexturePathFromMTL(string mtlFilePath) {
    std::ifstream file(mtlFilePath);
    if (!file.is_open()) return "";

    std::string line, texPath;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "map_Kd") { // Se a linha lida começar com "map_Kd" (textura difusa)
            ss >> texPath; // Lê o proximo texto no fluxo (caminho da imagem) e jogaa na variável texPath
            break; // PARA a execução do loop (sai do switch ou do for)	- já que encontrou o caminho da textura 
        }
    }
    return texPath;
}

int loadSimpleOBJ(string filePath, int &nVertices, string &textureName) {
    std::vector<vec3> temp_vertices; // vetor para armazenar as coordenadas dos vértices
    std::vector<vec2> temp_texCoords; // vetor para armazenar as coordenadas de textura
    std::vector<GLfloat> vBuffer; // vetor para armazenar os dados do VBO

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
            // CORREÇÃO: A chave do for agora engloba toda a extração de dados
            for (int i = 0; i < 3; i++) {
                ss >> vertexData;
                std::replace(vertexData.begin(), vertexData.end(), '/', ' ');
                std::istringstream faceSS(vertexData);
                unsigned int vIdx, tIdx;

                // CORREÇÃO: Linha descomentada para realizar a leitura
                faceSS >> vIdx >> tIdx; 

                vec3 pos = temp_vertices[vIdx - 1];
                vec2 tex = temp_texCoords[tIdx - 1];

                vBuffer.push_back(pos.x);
                vBuffer.push_back(pos.y);
                vBuffer.push_back(pos.z);
                vBuffer.push_back(tex.x); // x corresponde a s
                vBuffer.push_back(tex.y); // y corresponde a t
            }
        }
    }

    // Adicionando a geração dos buffers OpenGL que estava faltando
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vBuffer.size() * sizeof(GLfloat), vBuffer.data(), GL_STATIC_DRAW);

    // Atributo 0: Posição (x, y, z)
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);

    // Atributo 1: Coordenada de Textura (s, t) -> offset de 3 floats
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    nVertices = vBuffer.size() / 5;

    return VAO; // Retorna o identificador gerado
}