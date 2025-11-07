// ===========================================
// tarefa_display.c
// ===========================================
#include "tarefa_display.h"
#include "FreeRTOS.h"
#include "task.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "ssd1306.h"
#include "ssd1306_i2c.h"
#include <string.h>
#include <stdio.h>
#include "modo_global.h"   // <-- Importa a variável global

// ==== Configurações do display ====
#define I2C_PORT i2c1
#define SDA_PIN 14
#define SCL_PIN 15
#define OLED_WIDTH 128
#define OLED_HEIGHT 64

// ==== Pinos de comunicação FPGA ====
#define FPGA_SIGNAL_R 28   // bit0
#define FPGA_SIGNAL_G 16   // bit1
#define FPGA_SIGNAL_B 17   // bit2

// ============================================================
// Inicialização do barramento I²C
// ============================================================
static void i2c_init_display(void) {
    i2c_init(I2C_PORT, 400 * 1000); // 400kHz
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
}

// ============================================================
// Função auxiliar: traduz código do modo
// ============================================================
static const char *nome_modo(uint8_t code) {
    switch (code) {
        case 0b000: return "IDLE / PARADO";
        case 0b001: return "ELETRICO";
        case 0b010: return "DIESEL CHARGE";
        case 0b011: return "HIBRIDO ASSIST";
        case 0b100: return "REGEN. FREIO";
        default:    return "DESCONHECIDO";
    }
}

// ============================================================
// Tarefa principal do display OLED
// ============================================================
void task_display(void *params) {
    // Delay inicial para estabilizar I2C
    vTaskDelay(pdMS_TO_TICKS(500));

    printf("[DISPLAY] Inicializando OLED SSD1306...\n");
    i2c_init_display();

    ssd1306_t oled;
    ssd1306_init_bm(&oled, OLED_WIDTH, OLED_HEIGHT, false, ssd1306_i2c_address, I2C_PORT);
    ssd1306_config(&oled);
    ssd1306_init();

    // Configura pinos de entrada (sinais do FPGA)
    gpio_init(FPGA_SIGNAL_R);
    gpio_set_dir(FPGA_SIGNAL_R, GPIO_IN);
    gpio_pull_down(FPGA_SIGNAL_R);

    gpio_init(FPGA_SIGNAL_G);
    gpio_set_dir(FPGA_SIGNAL_G, GPIO_IN);
    gpio_pull_down(FPGA_SIGNAL_G);

    gpio_init(FPGA_SIGNAL_B);
    gpio_set_dir(FPGA_SIGNAL_B, GPIO_IN);
    gpio_pull_down(FPGA_SIGNAL_B);

    printf("[DISPLAY] Aguardando sinais do FPGA (GPIO28/16/17)...\n");

    uint8_t last_code = 0xFF;
    char line1[32];
    char line2[32];

    while (true) {
        // Lê os três bits do FPGA
        bool sig_r = gpio_get(FPGA_SIGNAL_R);
        bool sig_g = gpio_get(FPGA_SIGNAL_G);
        bool sig_b = gpio_get(FPGA_SIGNAL_B);

        // Combina bits em um código de 3 bits
        uint8_t code = (sig_b << 2) | (sig_g << 1) | sig_r;

        // Atualiza apenas se o código mudar
        if (code != last_code) {
            last_code = code;
            modo_atual = code; // <-- Atualiza o modo global
            printf("[DISPLAY] FPGA -> Novo modo: %03b (%s)\n", code, nome_modo(code));
        }

        // Limpa tela
        memset(oled.ram_buffer + 1, 0, oled.bufsize - 1);

        // Exibe título e modo atual
        snprintf(line1, sizeof(line1), " MODO ATUAL:");
        snprintf(line2, sizeof(line2), "%s", nome_modo(code));

        ssd1306_draw_string(oled.ram_buffer + 1, 10, 20, line1);
        ssd1306_draw_string(oled.ram_buffer + 1, 10, 40, line2);

        ssd1306_send_data(&oled);

        vTaskDelay(pdMS_TO_TICKS(200)); // Atualiza ~5x/s
    }
}
