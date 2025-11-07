#ifndef TAREFA_FPGA_MONITOR_H
#define TAREFA_FPGA_MONITOR_H

#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

// ------------------------------------------------------------
// Função para criar a tarefa de monitoramento do FPGA
// ------------------------------------------------------------
void criar_tarefa_fpga_monitor(UBaseType_t prioridade);

#endif // TAREFA_FPGA_MONITOR_H
