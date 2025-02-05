#include <stdio.h> 
#include "pico/stdlib.h" 
#include "hardware/pio.h" 
#include "hardware/timer.h" 
#include "hardware/clocks.h" 
#include "ws2812.pio.h" 
 
// Configurações dos pinos
 
#define LED_RGB_G 11
#define LED_RGB_B 12
#define LED_RGB_R 13
#define BUTTON_A 5
#define BUTTON_B 6
#define WS2812_PIN 7 
#define NUM_PIXELS 25 
#define IS_RGBW false 
 
// Variáveis globais
volatile int current_number = 0; // Número atual exibido
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
 
// Funções auxiliares
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b); 
} 

static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u); 
}

// Função para ajustar brilho
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

void gpio_irq_handler(uint gpio, uint32_t events) {
    uint32_t current_time = to_us_since_boot(get_absolute_time());

    if (gpio == BUTTON_A && current_time - last_time_a > 400000) {
        last_time_a = current_time;
        current_number = (current_number + 1) % 10; // Incrementa número
        display_number(current_number);
    } 

    if (gpio == BUTTON_B && current_time - last_time_b > 400000) {
        last_time_b = current_time;
        current_number = (current_number - 1 + 10) % 10; // Decrementa número
        display_number(current_number);
    } 
}

 
int main() {
    stdio_init_all();
    // Inicialização dos LEDs RGB
    gpio_init(LED_RGB_R);
    gpio_set_dir(LED_RGB_R, GPIO_OUT);
    gpio_init(LED_RGB_G);
    gpio_set_dir(LED_RGB_G, GPIO_OUT);
    gpio_init(LED_RGB_B);
    gpio_set_dir(LED_RGB_B, GPIO_OUT);
 
    // Inicialização dos botões
    gpio_init(BUTTON_A);
    gpio_set_dir(BUTTON_A, GPIO_IN);
    gpio_pull_up(BUTTON_A);
    gpio_set_irq_enabled_with_callback(BUTTON_A, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
 
    gpio_init(BUTTON_B);
    gpio_set_dir(BUTTON_B, GPIO_IN);
    gpio_pull_up(BUTTON_B);
    gpio_set_irq_enabled_with_callback(BUTTON_B, GPIO_IRQ_EDGE_FALL, true, &gpio_irq_handler);
 
    // Inicialização da matriz WS2812
    PIO pio = pio0; 
    int sm = 0; 
    uint offset = pio_add_program(pio, &ws2812_program); 
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW); 
 
    // Loop principal, pisca o LED RGB vermelho 5 vezes por segundo.
    while (true) {
        gpio_put(LED_RGB_R, 1);
        sleep_ms(100);
        gpio_put(LED_RGB_R, 0);
        sleep_ms(100);
     }
 
    return 0; 
}; 

