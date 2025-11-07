add_executable(picow_freertos
    main.c
    tarefa_display.c
    tarefa_joystick.c
    tarefa_freio.c
    battery_task.c
    tarefa_fpga_monitor.c
    tarefa_buzzer.c
    ../inc/ssd1306_i2c.c
)

set_target_properties(picow_freertos PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

target_sources(picow_freertos
    PRIVATE
    ${FREERTOS_KERNEL_PATH}/portable/MemMang/heap_4.c
)

target_include_directories(picow_freertos PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../inc
    ${CMAKE_CURRENT_LIST_DIR}/../config
)

target_link_libraries(picow_freertos 
    pico_stdlib 
    hardware_i2c
    hardware_adc
    hardware_dma
    hardware_pwm
    FreeRTOS-Kernel
)

pico_add_extra_outputs(picow_freertos)
pico_enable_stdio_usb(picow_freertos 1)
pico_enable_stdio_uart(picow_freertos 1)
