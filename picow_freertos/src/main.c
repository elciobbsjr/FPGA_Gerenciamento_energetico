#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

#include "tarefa_joystick.h"
#include "tarefa_freio.h"
#include "battery_task.h"
#include "tarefa_fpga_monitor.h"
#include "tarefa_display.h"
#include "tarefa_buzzer.h"

// ==== Header da variável global compartilhada ====
#include "modo_global.h"

// ============================================================
// VARIÁVEL GLOBAL (compartilhada entre display e buzzer)
// ============================================================
volatile uint8_t modo_atual = 0;

// ============================================================
// FUNÇÃO PRINCIPAL
// ============================================================
int main() {
    stdio_init_all();

    // ==== Criação das tarefas principais ====
    criar_tarefa_joystick(1);       // Leitura do joystick e envio de sinais ao FPGA
    criar_tarefa_freio(1);          // Botão A -> freio (GPIO5 → GPIO8)
    battery_task_init(1);           // Botão B -> simulação de bateria (GPIO6 → GPIO9)
    criar_tarefa_fpga_monitor(1);   // LEDs RGB + feedback serial (GPIO28/16/17)
    xTaskCreate(task_display, "DisplayTask", 2048, NULL, 1, NULL);   // OLED SSD1306
    xTaskCreate(task_buzzer,  "BuzzerTask",  1024, NULL, 1, NULL);   // Buzzers PWM

    // ==== Mensagens informativas ====
    printf("\n=========================================\n");
    printf("Sistema RTOS - FPGA Energy Manager\n");
    printf("=========================================\n");
    printf("Display OLED ativo, aguardando sinais do FPGA...\n");
    printf("Buzzer ativo, aguardando modo REGEN. FREIO...\n");
    printf("Tarefas criadas: joystick, freio, bateria, monitor, display e buzzer.\n");

    // ==== Inicia o escalonador do FreeRTOS ====
    vTaskStartScheduler();

    // Loop de segurança (caso o scheduler retorne)
    while (true) {
        tight_loop_contents();
    }
}
