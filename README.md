# Controle de Matriz de LEDs, LED RGB e Display OLED com Botões

Este projeto foi desenvolvido como parte do curso **EmbarcaTech**, promovido pelo **CEPEDI**, **SOFTEX** e **HwIT - Hardware Innovation Technologies**, para consolidar conhecimentos em comunicação serial, interrupções, debouncing, controle de LEDs e uso de displays no microcontrolador **RP2040**, utilizando a placa de desenvolvimento **BitDogLab**.

---

## 📝 Enunciado do Projeto

O objetivo é criar um sistema que combine hardware e software para:

- Demonstrar o uso de interrupções e tratamento de bouncing em botões.
- Manipular LEDs comuns e endereçáveis (WS2812).
- Exibir informações em um display OLED SSD1306 via comunicação I2C.
- Fixar o uso de resistores de pull-up internos em botões.
- Implementar comunicação serial via UART para interação com o usuário.

---

## 🔧 Componentes Utilizados

- **Matriz 5x5 de LEDs WS2812**: Conectada à GPIO 7.
- **LED RGB**: Conectado às GPIOs 11 (verde), 12 (azul) e 13 (vermelho).
- **Botão A**: Conectado à GPIO 5.
- **Botão B**: Conectado à GPIO 6.
- **Display OLED SSD1306**: Conectado via I2C (GPIO 14 - SDA, GPIO 15 - SCL).

---

## 🎯 Funcionalidades do Projeto

### 1. **Modificação da Biblioteca `font.h`**
- Adição de caracteres minúsculos à biblioteca `font.h` para exibição no display OLED.

### 2. **Entrada de Caracteres via Serial**
- Utilização do **Serial Monitor** do VS Code para digitar caracteres.
- Cada caractere digitado é exibido no display OLED.
- Quando um número entre 0 e 9 é digitado, o símbolo correspondente é exibido na matriz de LEDs WS2812.

### 3. **Interação com o Botão A**
- Pressionar o botão A alterna o estado do LED RGB **Verde** (ligado/desligado).
- O estado do LED é exibido no display OLED e enviado ao Serial Monitor.

### 4. **Interação com o Botão B**
- Pressionar o botão B alterna o estado do LED RGB **Azul** (ligado/desligado).
- O estado do LED é exibido no display OLED e enviado ao Serial Monitor.

### 5. **Uso de Interrupções e Debouncing**
- As ações dos botões são tratadas por interrupções (IRQs).
- Implementação de debouncing via software para evitar múltiplos acionamentos.

### 6. **Controle de LEDs**
- LED RGB conectado diretamente às GPIOs.
- Matriz de LEDs WS2812 controlada via PIO.

---

## 📂 Organização do Código

### Arquivos Incluídos
- **main.c**: Código principal com implementação de todas as funcionalidades.
- **ws2812.pio**: Código PIO para controlar os LEDs WS2812.
- **ssd1306.h**: Biblioteca para controle do display OLED SSD1306.
- **font.h**: Biblioteca de fontes para exibição no display OLED.

### Estrutura do Código
1. **Inicialização dos Componentes**:
   - LEDs RGB, botões, matriz de LEDs WS2812 e display OLED.
2. **Definição dos Números**:
   - Cada número de 0 a 9 é representado por uma matriz 5x5 de LEDs.
3. **Funções Auxiliares**:
   - Funções para manipulação de cores, exibição de números na matriz e atualização do display.
4. **Rotina de Interrupção**:
   - Trata os eventos dos botões A e B, realizando o debouncing e atualizando o estado dos LEDs e do display.
5. **Loop Principal**:
   - Verifica a entrada de caracteres via Serial e atualiza o display e a matriz de LEDs conforme necessário.

---

## 🚀 Como Executar o Projeto

### Pré-requisitos
- Placa **BitDogLab** configurada e conectada.
- Ambiente de desenvolvimento configurado com SDK do RP2040.
- **VS Code** com extensão **Pico SDK** instalada.

### Passos
1. Clone este repositório:
   ```bash
   git clone <https://github.com/queirozPatrick/comunica-o_serial>
   ```
2. Abra o projeto no VS Code.
3. Compile e carregue o código na placa BitDogLab.
4. Abra o **Serial Monitor** no VS Code para interagir com o projeto.

---

## 📋 Requisitos do Projeto

1. **Interrupções**:
   - As funcionalidades dos botões utilizam rotinas de interrupção para garantir responsividade.
2. **Debouncing**:
   - O bouncing dos botões é tratado via software, utilizando controle de tempo entre os acionamentos.
3. **Controle de LEDs**:
   - LED RGB conectado diretamente às GPIOs.
   - Matriz de LEDs WS2812 controlada via PIO.
4. **Display OLED**:
   - Utilização de fontes maiúsculas e minúsculas para exibição de informações.
5. **Comunicação Serial**:
   - Envio de informações via UART para interação com o usuário.

---

## 🎥 Vídeo de Demonstração

[Link para o vídeo no YouTube ou Google Drive](À ser add)

---

## 📄 Entrega

- **Código**: O código-fonte do projeto está disponível neste repositório, contendo todos os arquivos necessários para execução.
- **Vídeo de Demonstração**: O vídeo mostra o funcionamento do projeto na placa BitDogLab, com explicação das funcionalidades implementadas.

---

## 👨‍💻 Autor

**Patrick Queiroz**  
- GitHub: [queirozPatrick](https://github.com/queirozPatrick)  
- LinkedIn: [Patrick Queiroz](https://www.linkedin.com/in/patricksq/)
