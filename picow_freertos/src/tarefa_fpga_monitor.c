#include "tarefa_fpga_monitor.h"
#include <stdio.h>

// ============================================================
// DEFINIÇÕES DE PINOS (sinais do FPGA e LEDs RGB)
// ============================================================
#define FPGA_SIGNAL_R 28   // Entrada - bit0 do modo (vermelho)
#define FPGA_SIGNAL_G 16   // Entrada - bit1 do modo (verde)
#define FPGA_SIGNAL_B 17   // Entrada - bit2 do modo (azul)

#define LED_R_PIN     13   // LED RGB Vermelho
#define LED_G_PIN     11   // LED RGB Verde
#define LED_B_PIN     12   // LED RGB Azul

// ------------------------------------------------------------
// Protótipo interno da tarefa
// ------------------------------------------------------------
static void task_fpga_monitor(void *pvParameters);

// ------------------------------------------------------------
// Função auxiliar: traduz código do modo para texto
// ------------------------------------------------------------
static const char* nome_modo(uint8_t code) {
    switch (code) {
        case 0b000: return "IDLE";
        case 0b001: return "ELECTRIC";
        case 0b010: return "DIESEL_CHARGE";
        case 0b011: return "HYBRID_ASSIST";
        case 0b100: return "REGEN_BRAKING";
        default:    return "DESCONHECIDO";
    }
}

// ------------------------------------------------------------
// Criação da tarefa
// ------------------------------------------------------------
void criar_tarefa_fpga_monitor(UBaseType_t prioridade) {
    // Configuração inicial dos pinos FPGA
    gpio_init(FPGA_SIGNAL_R);
    gpio_set_dir(FPGA_SIGNAL_R, GPIO_IN);
    gpio_pull_down(FPGA_SIGNAL_R);

    gpio_init(FPGA_SIGNAL_G);
    gpio_set_dir(FPGA_SIGNAL_G, GPIO_IN);
    gpio_pull_down(FPGA_SIGNAL_G);

    gpio_init(FPGA_SIGNAL_B);
    gpio_set_dir(FPGA_SIGNAL_B, GPIO_IN);
    gpio_pull_down(FPGA_SIGNAL_B);

    // Configuração inicial dos LEDs
    gpio_init(LED_R_PIN);
    gpio_set_dir(LED_R_PIN, GPIO_OUT);
    gpio_init(LED_G_PIN);
    gpio_set_dir(LED_G_PIN, GPIO_OUT);
    gpio_init(LED_B_PIN);
    gpio_set_dir(LED_B_PIN, GPIO_OUT);

    gpio_put(LED_R_PIN, 0);
    gpio_put(LED_G_PIN, 0);
    gpio_put(LED_B_PIN, 0);

    // Cria a tarefa RTOS
    xTaskCreate(
        task_fpga_monitor,
        "FPGA_Monitor",
        1024,
        NULL,
        prioridade,
        NULL
    );

    printf("Tarefa FPGA Monitor iniciada (GPIO 28/16/17)\n");
}

// ------------------------------------------------------------
// Implementação da tarefa
// ------------------------------------------------------------
static void task_fpga_monitor(void *pvParameters) {
    uint8_t last_code = 0xFF; // valor impossível inicial

    for (;;) {
        bool sig_r = gpio_get(FPGA_SIGNAL_R);
        bool sig_g = gpio_get(FPGA_SIGNAL_G);
        bool sig_b = gpio_get(FPGA_SIGNAL_B);

        // Atualiza LEDs conforme sinais
        gpio_put(LED_R_PIN, sig_r);
        gpio_put(LED_G_PIN, sig_g);
        gpio_put(LED_B_PIN, sig_b);

        // Combina sinais num código de 3 bits (bit2: B, bit1: G, bit0: R)
        uint8_t code = (sig_b << 2) | (sig_g << 1) | sig_r;

        // Se o código mudou, imprime feedback
        if (code != last_code) {
            printf("📶 FPGA → Novo código recebido: %03b (%s)\n", code, nome_modo(code));
            last_code = code;
        }

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
