#include "battery_task.h"
#include <stdio.h>

// ------------------------------------------------------------
// Protótipo da função de tarefa
// ------------------------------------------------------------
static void vTaskBatteryButton(void *pvParameters);

// ------------------------------------------------------------
// Inicializa a tarefa e os pinos
// ------------------------------------------------------------
void battery_task_init(UBaseType_t prioridade) {
    gpio_init(PIN_BOTAO_B);
    gpio_set_dir(PIN_BOTAO_B, GPIO_IN);
    gpio_pull_up(PIN_BOTAO_B);

    gpio_init(PIN_FPGA_BATTERY);
    gpio_set_dir(PIN_FPGA_BATTERY, GPIO_OUT);
    gpio_put(PIN_FPGA_BATTERY, 0);   // inicia em nível baixo

    xTaskCreate(
        vTaskBatteryButton,
        "BatteryTask",
        1024,
        NULL,
        prioridade,
        NULL
    );

    printf("🟢 Battery task iniciada (GPIO6 → GPIO9)\n");
}

// ------------------------------------------------------------
// Implementação da tarefa
// ------------------------------------------------------------
static void vTaskBatteryButton(void *pvParameters) {
    bool estado_anterior = false;

    for (;;) {
        bool botao_pressionado = !gpio_get(PIN_BOTAO_B); // botão ativo em LOW

        if (botao_pressionado != estado_anterior) {
            estado_anterior = botao_pressionado;
            gpio_put(PIN_FPGA_BATTERY, botao_pressionado ? 1 : 0);

            if (botao_pressionado)
                printf("[BATERIA] Botão B pressionado -> GPIO9 = HIGH\n");
            else
                printf("[BATERIA] Botão B solto -> GPIO9 = LOW\n");
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}