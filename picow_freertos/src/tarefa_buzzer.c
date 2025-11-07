// ===========================================
// tarefa_buzzer.c (versão compatível com SDK 2.1.1)
// ===========================================
#include "FreeRTOS.h"
#include "task.h"
#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include <stdio.h>
#include "modo_global.h"

// ==== Pinos dos buzzers ====
#define BUZZER_A 21
#define BUZZER_B 10

// ==== Frequência base do som (ajuste conforme seu buzzer) ====
#define BUZZER_FREQ_HZ 2000
#define SYS_CLOCK_HZ   125000000 // clock do RP2040

// ============================================================
// Estrutura para armazenar parâmetros PWM de cada buzzer
// ============================================================
typedef struct {
    uint pin;
    uint slice;
    uint32_t top;
} buzzer_t;

static buzzer_t buzzerA, buzzerB;

// ============================================================
// Inicialização e controle PWM
// ============================================================
static void pwm_init_buzzer(buzzer_t *bz, uint buzzer_pin, uint freq_hz) {
    bz->pin = buzzer_pin;
    gpio_set_function(bz->pin, GPIO_FUNC_PWM);
    bz->slice = pwm_gpio_to_slice_num(bz->pin);

    float divider = 1.0f;
    uint32_t top = (uint32_t)(SYS_CLOCK_HZ / (freq_hz * divider)) - 1;
    if (top > 65535) {
        divider = 4.0f;
        top = (uint32_t)(SYS_CLOCK_HZ / (freq_hz * divider)) - 1;
    }

    pwm_set_clkdiv(bz->slice, divider);
    pwm_set_wrap(bz->slice, top);
    pwm_set_chan_level(bz->slice, PWM_CHAN_A, 0);
    pwm_set_enabled(bz->slice, true);

    bz->top = top; // <--- guardamos o wrap aqui
}

static void pwm_set_buzzer(buzzer_t *bz, uint8_t duty_percent) {
    uint16_t level = (duty_percent * bz->top) / 100;
    pwm_set_chan_level(bz->slice, PWM_CHAN_A, level);
}

// ============================================================
// Tarefa principal do buzzer
// ============================================================
void task_buzzer(void *params) {
    pwm_init_buzzer(&buzzerA, BUZZER_A, BUZZER_FREQ_HZ);
    pwm_init_buzzer(&buzzerB, BUZZER_B, BUZZER_FREQ_HZ);
    printf("[BUZZER] Inicializado em %d Hz (GPIO21 / GPIO10)\n", BUZZER_FREQ_HZ);

    bool toggle = false;

    for (;;) {
        uint8_t modo = modo_atual;

        if (modo == 0b100) { // Modo REGEN. FREIO
            toggle = !toggle;

            if (toggle) {
                pwm_set_buzzer(&buzzerA, 70);
                pwm_set_buzzer(&buzzerB, 0);
            } else {
                pwm_set_buzzer(&buzzerA, 0);
                pwm_set_buzzer(&buzzerB, 70);
            }
        } else {
            pwm_set_buzzer(&buzzerA, 0);
            pwm_set_buzzer(&buzzerB, 0);
        }

        vTaskDelay(pdMS_TO_TICKS(150));
    }
}
