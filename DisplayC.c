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

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define endereco 0x3C

#define led_pin_g 11  // LED Verde
#define led_pin_b 12  // LED Azul
#define led_pin_r 13  // LED Vermelho (não utilizado)

#define button_a_pin 5  // Botão A
#define button_b_pin 6  // Botão B

#define WS2812_PIN 7
#define NUM_PIXELS 25
#define IS_RGBW false

volatile bool led_green_state = false; // Estado do LED Verde
volatile bool led_blue_state = false;  // Estado do LED Azul
volatile int current_number = -1;      // Número atual exibido na matriz WS2812

// Variáveis para debouncing
volatile uint32_t last_time_a = 0;
volatile uint32_t last_time_b = 0;

// Buffer para números na matriz (exemplo: 0 a 9)
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

// Funções auxiliares (mesmo do código anterior)
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

uint32_t ajustarBrilho(uint8_t r, uint8_t g, uint8_t b, float fator) {
    return urgb_u32((uint8_t)(r * fator), (uint8_t)(g * fator), (uint8_t)(b * fator));
}

void display_number(int number) {
    float brilho = 0.1; // Reduz brilho para 10%
    uint32_t color = ajustarBrilho(148, 0, 211, brilho); // roxo com brilho reduzido
    
    for (int i = 0; i < NUM_PIXELS; i++) {
        put_pixel(numbers[number][i] ? color : 0);
    }
}

// Função de interrupção única para os botões A e B
void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    if (gpio == button_a_pin && current_time - last_time_a > 400000) { // Debouncing
        last_time_a = current_time;
        led_green_state = !led_green_state; // Alterna o estado do LED Verde
        gpio_put(led_pin_g, led_green_state);
        printf("Botão A pressionado. LED Verde: %s\n", led_green_state ? "Ligado" : "Desligado");
    }

    if (gpio == button_b_pin && current_time - last_time_b > 400000) { // Debouncing
        last_time_b = current_time;
        led_blue_state = !led_blue_state; // Alterna o estado do LED Azul
        gpio_put(led_pin_b, led_blue_state);
        printf("Botão B pressionado. LED Azul: %s\n", led_blue_state ? "Ligado" : "Desligado");
    }
}

void display_message(ssd1306_t *ssd, const char *message) {
    ssd1306_fill(ssd, false); // Limpa o display
    ssd1306_draw_string(ssd, message, 10, 30); // Desenha a mensagem
    ssd1306_send_data(ssd); // Atualiza o display
}

int main() {
    stdio_init_all(); // Inicializa comunicação USB CDC para monitor serial

    // Configura os pinos dos LEDs como saída
    gpio_init(led_pin_r);
    gpio_set_dir(led_pin_r, GPIO_OUT);
    gpio_put(led_pin_r, 0); // Inicialmente desligado

    gpio_init(led_pin_g);
    gpio_set_dir(led_pin_g, GPIO_OUT);
    gpio_put(led_pin_g, 0); // Inicialmente desligado

    gpio_init(led_pin_b);
    gpio_set_dir(led_pin_b, GPIO_OUT);
    gpio_put(led_pin_b, 0); // Inicialmente desligado

    // Configura os pinos dos botões como entrada
    gpio_init(button_a_pin);
    gpio_set_dir(button_a_pin, GPIO_IN);
    gpio_pull_up(button_a_pin);

    gpio_init(button_b_pin);
    gpio_set_dir(button_b_pin, GPIO_IN);
    gpio_pull_up(button_b_pin);

    // Configura interrupções para os botões
    gpio_set_irq_enabled(button_a_pin, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(button_b_pin, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_callback(&gpio_irq_handler);
    irq_set_enabled(IO_IRQ_BANK0, true);

    // Inicialização da matriz WS2812
    PIO pio = pio0;
    int sm = 0;
    uint offset = pio_add_program(pio, &ws2812_program);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);

    // I2C Initialisation. Using it at 400Khz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Set the GPIO pin function to I2C
    gpio_pull_up(I2C_SDA); // Pull up the data line
    gpio_pull_up(I2C_SCL); // Pull up the clock line

    ssd1306_t ssd; // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, endereco, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display

    // Limpa o display. O display inicia com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    printf("RP2040 inicializado. Envie caracteres ou números (0-9) para interagir.\n");

    while (true) {
        if (stdio_usb_connected()) { // Certifica-se de que o USB está conectado
            char c;
            if (scanf("%c", &c) == 1) { // Lê caractere da entrada padrão
                printf("Número selecionado: '%c'\n", c);

                // Exibe o caractere no display
                char message[2] = {c, '\0'};
                display_message(&ssd, message);

                // Verifica se o caractere é um número entre 0 e 9
                if (c >= '0' && c <= '9') {
                    current_number = c - '0';
                    display_number(current_number);
                }
            }
        }

        // Atualiza o display com o estado dos LEDs
        if (led_green_state) {
            display_message(&ssd, "TUDO CERTO1");
        } else {
            display_message(&ssd, "TUDO CERTO2");
        }

        if (led_blue_state) {
            display_message(&ssd, "TUDO CERTO3");
        } else {
            display_message(&ssd, "TUDO CERTO4");
        }

        sleep_ms(40);
    }

    return 0;
}