# Computação Gráfica - Híbrido

Repositório de exemplos de códigos em C++ utilizando OpenGL moderna (3.3+) criado para a Atividade Acadêmica Computação Gráfica do curso de graduação em Ciência da Computação - modalidade híbrida - da Unisinos. Ele é estruturado para facilitar a organização dos arquivos e a compilação dos projetos utilizando CMake.

## Atividade de Configuração do Ambiente (Hello3D) 

Este diretório contém a entrega da atividade inicial da disciplina de Computação Gráfica. O ambiente de desenvolvimento foi configurado com sucesso e as seguintes etapas foram concluídas:

- [x] **Criação do Repositório:** Configuração deste repositório para concentrar as entregas das atividades da disciplina.
- [x] **Importação do Projeto:** Projeto base `Hello3D` adicionado e adaptado ao ambiente local.
- [x] **Alteração de Código:** O título da janela de execução foi modificado para `"Ola 3D --Nathaly!"`, conforme exigido nas instruções.
- [x] **Comprovação de Execução:** Criação do arquivo `RESULT.md` contendo a captura de tela que demonstra a compilação e execução corretas do programa.

## Atividade 1: Geometria de um Cubo 3D Interativo (05/05/2026)

### 📂 Arquivo Principal (Correção)
Todo o código-fonte referente à geometria, instanciação e controles desta atividade foi implementado e está localizado no seguinte caminho:

> 📁 `DisciplinaComputacaoGrafica` / 📁 `src` / 📄 **`NewCube.cpp`**

---

Nesta atividade, a geometria base de uma pirâmide foi substituída por um Cubo 3D. O objeto foi modelado utilizando malhas poligonais (triângulos), onde cada face recebeu uma cor distinta para facilitar a visualização espacial na ausência de texturas e iluminação. Múltiplos cubos foram instanciados na cena.

### 🎮 Controles para Teste

A cena possui interação via teclado para testar as transformações geométricas. Utilize as seguintes teclas:

**Movimentação (Translação)**
* `A` / `D` - Move a cena para a Esquerda / Direita (Eixo X)
* `I` / `O` - Move a cena para Cima / Baixo (Eixo Y)

**Escala**
* `W` - Aumenta o tamanho (escala uniforme maior)
* `S` - Diminui o tamanho (escala uniforme menor)

**Rotação**
* `X` - Gira os cubos no próprio Eixo X
* `Y` - Gira os cubos no próprio Eixo Y
* `Z` - Gira os cubos no próprio Eixo Z

## Atividade 2: Leitura e Aplicação de Texturas em Modelos 3D (11/05/2026)

### 📂 Arquivo Principal
O código-fonte contendo a lógica de extração de dados e renderização texturizada está localizado em:

> 📁 `DisciplinaComputacaoGrafica` / 📁 `src` / 📄 **`Texture.cpp`**

---

O objetivo desta etapa foi implementar o carregamento dinâmico de texturas (Mapeamento UV) a partir de modelos 3D exportados. O pipeline gráfico foi atualizado para interpretar corretamente os arquivos de geometria e materiais, substituindo as cores sólidas da atividade anterior por imagens 2D projetadas sobre a malha.

### ⚙️ Funcionalidades Implementadas

* **Leitura de Arquivos `.OBJ`:** Processamento das linhas de coordenadas de vértices (`v`) e de texturas (`vt`), unificando-as de forma intercalada no VBO (Vertex Buffer Object).
* **Leitura de Arquivos `.MTL`:** Extração automatizada do caminho da imagem de textura difusa (`map_Kd`).
* **Mapeamento UV e Correção de Eixos:** Carregamento da imagem via `stb_image` com o ajuste de inversão vertical (`stbi_set_flip_vertically_on_load`) para garantir que a textura corresponda perfeitamente à orientação do OpenGL.
* **Teste de Profundidade (Z-Buffer):** Ativação e limpeza do buffer de profundidade (`GL_DEPTH_TEST`) para corrigir o descarte de fragmentos e garantir a oclusão correta dos triângulos (evitando que o modelo fique "pelo avesso").

### 🎮 Controles para Teste (Atividade 2)

A cena permite interação em tempo real para validar o mapeamento das texturas em diferentes ângulos e comprovar a volumetria do modelo 3D. Utilize as seguintes teclas:

**Rotação Interativa**
* `X` - Rotaciona o modelo no Eixo X
* `Y` - Rotaciona o modelo no Eixo Y
* `Z` - Rotaciona o modelo no Eixo Z

**Geral**
* `ESC` - Encerra a execução e fecha a janela

* # Sistema de Iluminação de 3 Pontos (Three-Point Lighting)

Atividade desenvolvida para a disciplina de *Computação Gráfica*.

*Alunas:* * Nathaly Loggiovini  
* Isadora Albano  

---
## 📌 M4 | Tarefa - Atividade vivencial 2
## Alunas: Nathaly Loggiovini e Isadora Albano.

O objetivo deste projeto é implementar o conceito clássico de *Sistema de Iluminação de 3 Pontos* utilizando a linguagem C++ e a API gráfica OpenGL. A cena renderiza uma esfera tridimensional sombreada, controlada e iluminada por três fontes de luz distintas que trabalham juntas para dar volume, separação e destaque ao objeto:

1. *Key Light (Luz Principal):* A fonte mais brilhante, posicionada à frente e lateralmente ao objeto, responsável por definir as principais luzes e sombras.
2. *Fill Light (Luz de Preenchimento):* Posicionada no lado oposto à Key Light com menor intensidade, usada para suavizar as sombras densas criadas pela luz principal.
3. *Backlight (Luz de Fundo):* Posicionada atrás do objeto para criar uma borda iluminada (silhueta), separando visualmente o elemento do fundo da cena.

O cálculo de iluminação utiliza o modelo de reflexão local associado a um fator de *atenuação da luz pela distância*, garantindo um decaimento realista da intensidade luminosa conforme a posição dos pontos na cena.

---

## 📂 Localização do Arquivo Principal

Para fins de correção, o código-fonte principal que contém toda a lógica de Shaders, cálculo de iluminação e a malha da esfera está localizado em:
* src/Light.cpp

---

## 🎮 Controles Interativos

É possível ligar e desligar cada uma das fontes de luz em tempo de execução através do teclado para observar o impacto individual de cada componente no sombreamento da esfera:

| Tecla | Função | Feedback no Terminal |
| :---: | :--- | :--- |
| *1* | Alterna o estado da *Key Light* (Luz Principal) | Key Light: LIGADA / DESLIGADA |
| *2* | Alterna o estado da *Fill Light* (Luz de Preenchimento) | Fill Light: LIGADA / DESLIGADA |
| *3* | Alterna o estado da *Backlight* (Luz de Fundo) | Backlight: LIGADA / DESLIGADA |
| *ESC*| Encerra a aplicação com segurança | — |

---

### Atividade 3: Implementação do Modelo de Iluminação Phong (28/05/2026)

📂 **Arquivo Principal**

O código-fonte contendo a implementação do modelo de iluminação e a renderização do modelo 3D (Suzanne) está localizado em:
📁 `DisciplinaComputacaoGrafica` / 📁 `src` / 📄 `PhongModel.cpp`

Nesta atividade, o foco foi a implementação do **Modelo de Reflexão de Phong** para criar uma iluminação realista em superfícies curvas. 
O pipeline gráfico foi expandido para calcular a iluminação por fragmento, integrando os componentes ambiente, difuso e especular, 
utilizando dados extraídos diretamente de arquivos `.mtl`.

⚙️ **Funcionalidades Implementadas**

* **Extração de Materiais (MTL):** O parser do arquivo `.obj` foi atualizado para ler os coeficientes de iluminação (`Ka`, `Kd`, `Ks`) e o expoente de brilho (`Ns`) do arquivo de material associado.
* **Modelo de Reflexão de Phong:** Implementação matemática no *Fragment Shader* dos três componentes principais:
    * **Ambiente (`Ka`):** Iluminação constante para evitar áreas totalmente pretas.
    * **Difusa (`Kd`):** Cálculo baseado no produto escalar entre a normal da superfície e a direção da luz.
    * **Especular (`Ks`):** Cálculo de realces brilhantes (brilho especular) baseado na reflexão do vetor de visão.
* **Pipeline de Dados:** Inclusão das normais (`vn`) no VBO para permitir o cálculo correto da iluminação sobre a geometria complexa do modelo "Suzanne".
* **Renderização:** Integração de texturas com o modelo de iluminação, permitindo que a textura `pixelWall.png` receba o sombreamento e o brilho calculado pelo shader.

🎮 **Controles para Teste**

A cena mantém a visualização dinâmica do modelo com rotação automática para análise do comportamento da luz especular sobre a superfície:
* **Rotação:** O objeto rotaciona automaticamente no eixo Y.
* **ESC:** Encerra a execução e fecha a janela.

---

## Aluna: Nathaly Loggiovini.
## Professor: Guilherme Chagas Kurtz.