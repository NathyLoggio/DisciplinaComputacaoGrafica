/* 
 *
 * Adaptado por Nathaly Loggiovini
 * para a disciplina de Computação Gráfica - Unisinos
 * Versão inicial: 7/4/2017
 * Nova  Modificaçãa para gerar um Modelo Phong
 * Última atualização em 28/05/2026
 * 
 */

#include <iostream>
#include <string>
#include <assert.h>

#include <fstream>
#include <sstream>
#include <vector>
#include <cstdio>

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
GLuint loadTexture(string filePath, int &width, int &height);

void drawGeometry(GLuint shaderID, GLuint VAO, vec3 position, vec3 dimensions, float angle, int nVertices, vec3 color= vec3(1.0,0.0,0.0), vec3 axis = (vec3(0.0, 0.0, 1.0)));
GLuint generateSphere(float radius, int latSegments, int lonSegments, int &nVertices);
 
// Dimensões da janela (pode ser alterado em tempo de execução)
const GLuint WIDTH = 800, HEIGHT = 800;

// Código fonte do Vertex Shader (em GLSL): ainda hardcoded
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
   	gl_Position = projection * model * vec4(position.x, position.y, position.z, 1.0);
	fragPos = model * vec4(position.x, position.y, position.z, 1.0);
	texCoord = texc;
	vNormal = normal;
	vColor = vec4(color,1.0);
})";

const GLchar *fragmentShaderSource = R"(
#version 400
in vec2 texCoord;
in vec3 vNormal;
in vec4 fragPos;
in vec4 vColor;

uniform sampler2D texBuff;
uniform vec3 lightPos;
uniform vec3 camPos;

// Coeficientes lidos do arquivo .mtl
uniform vec3 Ka; // Ambiente
uniform vec3 Kd; // Difusa
uniform vec3 Ks; // Especular
uniform float Ns; // Expoente de brilho (shininess)

out vec4 color;

void main()
{
    vec3 lightColor = vec3(1.0, 1.0, 1.0);
    //Use texture(texBuff, texCoord) se for texturizado, ou vColor se usar cor sólida do OBJ
    vec4 objectColor = texture(texBuff, texCoord); 

    // 1. Ambiente
    vec3 ambient = Ka * lightColor;

    // 2. Difusa
    vec3 N = normalize(vNormal);
    vec3 L = normalize(lightPos - vec3(fragPos));
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = Kd * diff * lightColor;

    // 3. Especular
    vec3 R = normalize(reflect(-L, N));
    vec3 V = normalize(camPos - vec3(fragPos));
    float spec = max(dot(R, V), 0.0);
    vec3 specular = Ks * pow(spec, Ns) * lightColor;

    // Resultado da iluminação de Phong
    vec3 result = (ambient + diffuse) * vec3(objectColor) + specular;
    color = vec4(result, 1.0);
})";

GLuint loadObj(string filePath, int &nVertices, vec3 &Ka, vec3 &Kd, vec3 &Ks, float &Ns) {
    vector<vec3> temp_v, temp_vn;
    vector<vec2> temp_vt;
    vector<GLfloat> vBuffer;

    ifstream file(filePath);
    if (!file.is_open()) {
        cout << "Erro ao abrir OBJ: " << filePath << endl;
        return 0;
    }

    string line;
    while (getline(file, line)) {
        istringstream iss(line);
        string type;
        iss >> type;

        // Leitura do arquivo MTL referenciado
        if (type == "mtllib") {
            string mtlName;
            iss >> mtlName;
            string mtlPath = filePath.substr(0, filePath.find_last_of('/') + 1) + mtlName;
            ifstream mtlFile(mtlPath);
            if (mtlFile.is_open()) {
                string mtlLine;
                while (getline(mtlFile, mtlLine)) {
                    istringstream mtlIss(mtlLine);
                    string mtlType;
                    mtlIss >> mtlType;
                    if (mtlType == "Ka") mtlIss >> Ka.r >> Ka.g >> Ka.b;
                    else if (mtlType == "Kd") mtlIss >> Kd.r >> Kd.g >> Kd.b;
                    else if (mtlType == "Ks") mtlIss >> Ks.r >> Ks.g >> Ks.b;
                    else if (mtlType == "Ns") mtlIss >> Ns;
                }
            }
        } 
        // Armazenamento temporário dos dados geométricos
        else if (type == "v") {
            vec3 v; iss >> v.x >> v.y >> v.z; temp_v.push_back(v);
        } else if (type == "vt") {
            vec2 vt; iss >> vt.x >> vt.y; temp_vt.push_back(vt);
        } else if (type == "vn") {
            vec3 vn; iss >> vn.x >> vn.y >> vn.z; temp_vn.push_back(vn);
        } 
        // Montagem do VBO lendo as Faces (formato v/vt/vn)
        else if (type == "f") {
            for (int i = 0; i < 3; ++i) {
                string faceData;
                iss >> faceData;
                int vIdx, vtIdx, vnIdx;
                sscanf(faceData.c_str(), "%d/%d/%d", &vIdx, &vtIdx, &vnIdx);

                vBuffer.insert(vBuffer.end(), {
                    temp_v[vIdx - 1].x, temp_v[vIdx - 1].y, temp_v[vIdx - 1].z, // Posição (0)
                    1.0f, 1.0f, 1.0f,                                           // Cor default (1)
                    temp_vn[vnIdx - 1].x, temp_vn[vnIdx - 1].y, temp_vn[vnIdx - 1].z, // Normal (2)
                    temp_vt[vtIdx - 1].x, temp_vt[vtIdx - 1].y                  // UV (3)
                });
            }
        }
    }

    // Geração do VAO e VBO mantendo o seu layout de 11 floats
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vBuffer.size() * sizeof(GLfloat), vBuffer.data(), GL_STATIC_DRAW);

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

	// Essencial para computadores da Apple
	// #ifdef __APPLE__
	//	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	// #endif

	// Criação da janela GLFW
	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Ola esfera iluminada!", nullptr, nullptr);
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

	glEnable(GL_DEPTH_TEST);

	// Compilando e buildando o programa de shader
	GLuint shaderID = setupShader();

	// Declarando as variáveis que serão preenchidas pela função loadObj
    int nVertices;
    vec3 Ka, Kd, Ks;
    float Ns = 32.0f;

    GLuint VAO = loadObj("../assets/Modelos3D/Suzanne.obj", nVertices, Ka, Kd, Ks, Ns);

    // Carregando a textura
    int imgWidth, imgHeight;
    GLuint texID = loadTexture("../assets/tex/pixelWall.png", imgWidth, imgHeight);

    vec3 lightPos = vec3(0.6, 1.2, -0.5);
    vec3 camPos = vec3(0.0, 0.0, -3.0);

    glUseProgram(shaderID);

    // Enviando as variáveis lidas do arquivo para o Shader
    glUniform3fv(glGetUniformLocation(shaderID, "Ka"), 1, value_ptr(Ka));
    glUniform3fv(glGetUniformLocation(shaderID, "Kd"), 1, value_ptr(Kd));
    glUniform3fv(glGetUniformLocation(shaderID, "Ks"), 1, value_ptr(Ks));
    glUniform1f(glGetUniformLocation(shaderID, "Ns"), Ns);

    glUniform3fv(glGetUniformLocation(shaderID, "lightPos"), 1, value_ptr(lightPos));
    glUniform3fv(glGetUniformLocation(shaderID, "camPos"), 1, value_ptr(camPos));

	//Ativando o primeiro buffer de textura da OpenGL
	glActiveTexture(GL_TEXTURE0);

	// Matriz de projeção paralela ortográfica
	// mat4 projection = ortho(-10.0, 10.0, -10.0, 10.0, -1.0, 1.0);
	mat4 projection = ortho(-1.0, 1.0, -1.0, 1.0, -3.0, 3.0);
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "projection"), 1, GL_FALSE, value_ptr(projection));

	// Matriz de modelo: transformações na geometria (objeto)
	mat4 model = mat4(1); // matriz identidade
	glUniformMatrix4fv(glGetUniformLocation(shaderID, "model"), 1, GL_FALSE, value_ptr(model));

	float angle = 0.0f;


	// Loop da aplicação - "game loop"
	while (!glfwWindowShouldClose(window))
	{
		// Checa se houveram eventos de input (key pressed, mouse moved etc.) e chama as funções de callback correspondentes
		glfwPollEvents();

		// Limpa o buffer de cor
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // cor de fundo
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glBindVertexArray(VAO); // Conectando ao buffer de geometria
		glBindTexture(GL_TEXTURE_2D, texID); //conectando com o buffer de textura que será usado no draw

		// Faz o ângulo crescer continuamente
		angle += 0.01f;

		// Eixo Y (0, 1, 0) para rotacionar o objeto lateralmente
		drawGeometry(shaderID, VAO, vec3(0, 0, 0), vec3(1, 1, 1), angle, nVertices, vec3(1,0,0), vec3(0, 1, 0));

	
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

void drawGeometry(GLuint shaderID, GLuint VAO, vec3 position, vec3 dimensions, float angle, int nVertices, vec3 color, vec3 axis)
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

	//glUniform4f(glGetUniformLocation(shaderID, "inputColor"), color.r, color.g, color.b, 1.0f); // enviando cor para variável uniform inputColor
																								//  Chamada de desenho - drawcall
																								//  Poligono Preenchido - GL_TRIANGLES
	glDrawArrays(GL_TRIANGLES, 0, nVertices);
}