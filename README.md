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

## Aluna: Nathaly Loggiovini.
## Professor: Guilherme Chagas Kurtz.