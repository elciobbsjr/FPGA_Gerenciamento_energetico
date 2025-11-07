#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>
#include <stdio.h>
#include "tarefa_freio.h"

#define BOTAO_FREIO_PIN 5
#define FPGA_FREIO_PIN  8

void task_freio(void *pvParameters) {
    gpio_init(BOTAO_FREIO_PIN);
    gpio_set_dir(BOTAO_FREIO_PIN, GPIO_IN);
    gpio_pull_up(BOTAO_FREIO_PIN);

    gpio_init(FPGA_FREIO_PIN);
    gpio_set_dir(FPGA_FREIO_PIN, GPIO_OUT);
    gpio_put(FPGA_FREIO_PIN, 0);

    vTaskDelay(pdMS_TO_TICKS(1000)); // Espera inicial
    printf("[FREIO] Tarefa iniciada: pressione o botão A (GPIO5) para enviar sinal via GPIO8.\n");

    bool estado_anterior = false;

    while (true) {
        bool botao_pressionado = !gpio_get(BOTAO_FREIO_PIN);

        if (botao_pressionado != estado_anterior) {
            estado_anterior = botao_pressionado;
            gpio_put(FPGA_FREIO_PIN, botao_pressionado ? 1 : 0);

            if (botao_pressionado)
                printf("[FREIO] Freio acionado -> GPIO8 = HIGH\n");
            else
                printf("[FREIO] Freio solto -> GPIO8 = LOW\n");
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void criar_tarefa_freio(UBaseType_t prio) {
    xTaskCreate(task_freio, "FreioTask", 1024, NULL, prio, NULL);
}
