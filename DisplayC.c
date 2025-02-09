#include <stdlib.h>
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/irq.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include "ws2812.pio.h"

// Definições
#define I2C_PORT i2c1 // Porta I2C utilizada
#define I2C_SDA 14 // Pino SDA (dados) do I2C
#define I2C_SCL 15 // Pino SCL (clock) do I2C
#define endereco 0x3C // Endereço do display OLED

#define led_pin_g 11 // LED verde
#define led_pin_b 12 // LED azul
#define led_pin_r 13 // LED verelho

#define button_a_pin 5 // botão A
#define button_b_pin 6 // botão B

#define WS2812_PIN 7 // Pino do LED WS2812
#define NUM_PIXELS 25 // Número de LEDs na matriz
#define IS_RGBW false // Define se o LED é RGBW (falso para RGB)

// Atualiza o display
void update_display();

// Variáveis globais
volatile bool led_green_state = false; // Estado do LED verde
volatile bool led_blue_state = false; // Estado do LED azul
volatile int current_number = -1; // Número atual exibido (-1 significa nenhum número)

volatile uint32_t last_time_a = 0;
volatile uint32_t last_time_b = 0;
volatile bool last_led_changed = false; // false = LED verde, true = LED azul

ssd1306_t ssd; // Variável para controlar o display SSD1306

// Matriz de números dos LEDs
bool numbers[10][NUM_PIXELS] = {
    // Número 00
    { 
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0
    },
    // Número 01
    { 
        0, 1, 1, 1, 0,
        0, 0, 1, 0, 0,
        0, 0, 1, 0, 0,
        0, 1, 1, 0, 0,
        0, 0, 1, 0, 0
    },
    // Número 02
    { 
        0, 1, 1, 1, 0,
        0, 1, 0, 0, 0,
        0, 1, 1, 1, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0
    },
    // Número 03
     {
        0, 1, 1, 1, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0
    },
    // Número 04
    { 
        0, 1, 0, 0, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 0, 1, 0
    },
    // Número 05
    { 
        0, 1, 1, 1, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 0, 0,
        0, 1, 1, 1, 0
    },
    // Número 06
    { 
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 0, 0,
        0, 1, 1, 1, 0
    },
    // Número 07
    { 
        0, 1, 0, 0, 0,
        0, 0, 0, 1, 0,
        0, 1, 0, 0, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0
    },
    // Número 08
    { 
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0
    },
    // Número 09
    { 
        0, 1, 1, 1, 0,
        0, 0, 0, 1, 0,
        0, 1, 1, 1, 0,
        0, 1, 0, 1, 0,
        0, 1, 1, 1, 0
    } 
 
};

// Funções auxiliares
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

static inline void put_pixel(uint32_t pixel_grb) {
    // Envia um valor de pixel para o LED WS2812
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

uint32_t ajustarBrilho(uint8_t r, uint8_t g, uint8_t b, float fator) { // Ajusta o brilho de uma cor RGB
    return urgb_u32((uint8_t)(r * fator), (uint8_t)(g * fator), (uint8_t)(b * fator));
}

void display_number(int number) {
    // Exibe o número na matriz de LEDs
    float brilho = 0.1;
    uint32_t color = ajustarBrilho(0,255,255, brilho); // Cor ciano
    for (int i = 0; i < NUM_PIXELS; i++) {
        put_pixel(numbers[number][i] ? color : 0); // Liga ou desliga o LED
    }
}

void gpio_irq_handler(uint gpio, uint32_t events) {
    // Manipula as interrupções dos botões
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    if (gpio == button_a_pin && current_time - last_time_a > 400000) {
        last_time_a = current_time;
        led_green_state = !led_green_state; // Alterna o estado do LED verde
        gpio_put(led_pin_g, led_green_state);
        last_led_changed = false; // LED verde foi alterado
        current_number = -1; // Reseta o número atual
        printf("Botão A pressionado. LED Verde: %s\n", led_green_state ? "Ligado" : "Desligado");
        update_display(); // Atualiza o display
    }

    if (gpio == button_b_pin && current_time - last_time_b > 400000) {
        last_time_b = current_time;
        led_blue_state = !led_blue_state; // Alterna o estado do LED azul
        gpio_put(led_pin_b, led_blue_state);
        last_led_changed = true; // LED azul foi alterado
        current_number = -1; // Reseta o número atual
        printf("Botão B pressionado. LED Azul: %s\n", led_blue_state ? "Ligado" : "Desligado");
        update_display(); // Atualiza o display após pressionar o botão B
    }
}

// Função para atualizar o display com os estados dos LEDs e o número atual
void update_display() {
    char message[20]; // Buffer para armazenar a mensagem

    // Limpa o display antes de atualizar
    ssd1306_fill(&ssd, false);

    // Se houver um número atual, exibe e ignora o status dos LEDs
    if (current_number != -1) {
        char num_str[3];
        sprintf(num_str, "%d", current_number);
        ssd1306_draw_string(&ssd, num_str, 60, 30); // Centralizado
    } else {
        // Caso contrário, exibe o status do último LED alterado
        if (!last_led_changed) { // LED verde foi alterado por último
            sprintf(message, "Led Verde: %s", led_green_state ? ">>>>>>Ligado" : ">>>>Desligado");
        } else { // LED azul foi alterado por último
            sprintf(message, "Led Azul: %s", led_blue_state ? ">>>>>>Ligado" : ">>>>\nDesligado");
        }
        ssd1306_draw_string(&ssd, message, 35, 30); // Centralizado
    }

    ssd1306_send_data(&ssd); // Envia os dados para o display
}

    // Função principal
int main() {
    stdio_init_all(); // Inicializa a comunicação serial

    // Inicializa os LEDs
    gpio_init(led_pin_r);
    gpio_set_dir(led_pin_r, GPIO_OUT);
    gpio_put(led_pin_r, 0);

    gpio_init(led_pin_g);
    gpio_set_dir(led_pin_g, GPIO_OUT);
    gpio_put(led_pin_g, 0);

    gpio_init(led_pin_b);
    gpio_set_dir(led_pin_b, GPIO_OUT);
    gpio_put(led_pin_b, 0);

    // Inicializa os botões
    gpio_init(button_a_pin);
    gpio_set_dir(button_a_pin, GPIO_IN);
    gpio_pull_up(button_a_pin);

    gpio_init(button_b_pin);
    gpio_set_dir(button_b_pin, GPIO_IN);
    gpio_pull_up(button_b_pin);

    // Configuração das interrupções
    gpio_set_irq_enabled(button_a_pin, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(button_b_pin, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_callback(&gpio_irq_handler);
    irq_set_enabled(IO_IRQ_BANK0, true);

    // Inicialização do WS2812
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    // Inicialização do I2C e display SSD1306
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT);
    ssd1306_config(&ssd);
    ssd1306_send_data(&ssd);
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    printf("RP2040 inicializado.\n");

    update_display(); // Exibe mensagem inicial no display

    while (true) {
        if (stdio_usb_connected()) {
            char c;
            if (scanf("%c", &c) == 1) {
                printf("Caractere recebido: '%c'\n", c);
    
                if (c >= '0' && c <= '9') {
                    current_number = c - '0'; // Converte o caractere para número
                    display_number(current_number); // Atualiza a matriz de LED com o número
                    update_display(); // Atualiza o display OLED com o número
                } else if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) {
                    // Exibe o caractere no display OLED
                    current_number = -1; // Reseta o número atual
                    ssd1306_fill(&ssd, false); // Limpa o display
                    char str[2] = {c, '\0'}; // Converte o caractere para uma string
                    ssd1306_draw_string(&ssd, str, 60, 30); // Exibe o caractere no centro do display
                    ssd1306_send_data(&ssd); // Envia os dados para o display
                }
            }
        }
    
        sleep_ms(40); // Delay para evitar uso excessivo da CPU
    }
}
