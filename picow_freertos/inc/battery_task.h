#ifndef BATTERY_TASK_H
#define BATTERY_TASK_H

#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

// ------------------------------------------------------------
// DEFINIÇÕES DE PINOS
// ------------------------------------------------------------
#define PIN_BOTAO_B       6   // Botão B - entrada (INPUT_PULLUP)
#define PIN_FPGA_BATTERY  9   // Saída para o FPGA

// ------------------------------------------------------------
// Protótipo de criação da tarefa
// ------------------------------------------------------------
void battery_task_init(UBaseType_t prioridade);

#endif // BATTERY_TASK_H
