#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdbool.h>
#include <stdio.h>

// ================================================================
// CONFIGURAÇÕES DO SISTEMA
// ================================================================
#define PERIOD_MS   100u   // intervalo entre leituras (em ms)
#define ADC_VRY_CH  0      // GPIO26 → ADC0
#define ADC_VRX_CH  1      // GPIO27 → ADC1

// === Pinos ===
#define JOY_SW_PIN        22
#define FPGA_P_LOW_PIN    18
#define FPGA_P_HIGH_PIN   19
#define FPGA_IDLE_PIN     20

TaskHandle_t handle_joy = NULL;

// ================================================================
// Conversão do valor ADC em porcentagem (0–100%)
// ================================================================
static inline int adc_to_percent(uint16_t raw) {
    if (raw > 4095) raw = 4095;
    return (raw * 100) / 4095;
}

// ================================================================
// Tarefa principal do joystick
// ================================================================
void tarefa_joystick(void *params) {
    (void)params;

    // Inicializa pinos de saída para FPGA
    uint8_t fpga_pins[] = { FPGA_P_LOW_PIN, FPGA_P_HIGH_PIN, FPGA_IDLE_PIN };
    for (int i = 0; i < 3; i++) {
        gpio_init(fpga_pins[i]);
        gpio_set_dir(fpga_pins[i], GPIO_OUT);
        gpio_put(fpga_pins[i], 0);
    }

    // Inicializa botão SW
    gpio_init(JOY_SW_PIN);
    gpio_set_dir(JOY_SW_PIN, GPIO_IN);
    gpio_pull_up(JOY_SW_PIN);

    // Inicializa ADCs (eixos X e Y)
    adc_init();
    adc_gpio_init(26);  // VRy → GPIO26 → ADC0
    adc_gpio_init(27);  // VRx → GPIO27 → ADC1
    adc_select_input(ADC_VRY_CH);

    printf("\n[JOYSTICK] Calibrando... mantenha o joystick parado.\n");
    vTaskDelay(pdMS_TO_TICKS(1000));

    uint16_t calib_center_y = 0;
    uint16_t calib_center_x = 0;

    // Captura ponto central de calibração
    adc_select_input(ADC_VRY_CH);
    calib_center_y = adc_read();

    adc_select_input(ADC_VRX_CH);
    calib_center_x = adc_read();

    printf("[JOYSTICK] Calibração concluída.\n");
    printf(" - Centro Y = %u | Centro X = %u\n", calib_center_y, calib_center_x);
    printf("[JOYSTICK] Monitorando aceleração...\n");

    bool last_low = false, last_high = false, last_idle = true;

    for (;;) {
        // Lê eixo Y (controle de potência)
        adc_select_input(ADC_VRY_CH);
        uint16_t raw_y = adc_read();

        // Lê botão (apenas para debug ou ações futuras)
        bool sw_pressed = !gpio_get(JOY_SW_PIN);

        // Corrige escala em torno do ponto central
        int delta = (int)raw_y - (int)calib_center_y;
        int power_demand = (delta * 100 / 2048) + 50;  // 50% = neutro
        if (power_demand < 0)   power_demand = 0;
        if (power_demand > 100) power_demand = 100;

        // Define faixas de operação
        bool p_demand_low  = (power_demand >= 10 && power_demand < 70);
        bool p_demand_high = (power_demand >= 70);
        bool p_idle        = (power_demand < 10);

        // Envia sinais digitais
        gpio_put(FPGA_P_LOW_PIN,  p_demand_low);
        gpio_put(FPGA_P_HIGH_PIN, p_demand_high);
        gpio_put(FPGA_IDLE_PIN,   p_idle);

        // Log apenas quando houver mudança de estado
        if (p_demand_low != last_low || p_demand_high != last_high || p_idle != last_idle) {
            printf("[JOY] Potência = %3d%% | LOW=%d HIGH=%d IDLE=%d | SW=%d\n",
                   power_demand,
                   p_demand_low, p_demand_high, p_idle,
                   sw_pressed);

            last_low  = p_demand_low;
            last_high = p_demand_high;
            last_idle = p_idle;
        }

        vTaskDelay(pdMS_TO_TICKS(PERIOD_MS));
    }
}

// ================================================================
// Criação da tarefa no FreeRTOS
// ================================================================
void criar_tarefa_joystick(UBaseType_t prio) {
    BaseType_t ok = xTaskCreate(
        tarefa_joystick,
        "JoystickTask",
        2048,
        NULL,
        prio,
        &handle_joy
    );
    configASSERT(ok == pdPASS);
}
