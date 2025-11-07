#ifndef TAREFA_JOYSTICK_H
#define TAREFA_JOYSTICK_H

#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"

void tarefa_joystick(void *params);
void criar_tarefa_joystick(UBaseType_t prio);

#endif
