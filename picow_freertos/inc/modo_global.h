// ===========================================
// modo_global.h
// ===========================================
#ifndef MODO_GLOBAL_H
#define MODO_GLOBAL_H

#include <stdint.h>

// Variável global compartilhada entre tarefas
extern volatile uint8_t modo_atual;

#endif
