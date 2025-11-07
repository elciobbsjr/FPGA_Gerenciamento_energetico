`timescale 1ns/1ps

module tb_energy_system_all_in_one;

    // Entradas
    reg clk;
    reg reset_n;
    reg p_demand_low;
    reg p_demand_high;
    reg p_idle;
    reg is_braking;
    reg battery_button;

    // Saídas
    wire operating_mode0;
    wire operating_mode1;
    wire operating_mode2;

    // Instância do DUT
    energy_system_all_in_one uut (
        .clk(clk),
        .reset_n(reset_n),
        .p_demand_low(p_demand_low),
        .p_demand_high(p_demand_high),
        .p_idle(p_idle),
        .is_braking(is_braking),
        .battery_button(battery_button),
        .operating_mode0(operating_mode0),
        .operating_mode1(operating_mode1),
        .operating_mode2(operating_mode2)
    );

    // Clock
    initial clk = 0;
    always #5 clk = ~clk;   // 100 MHz

    // Estímulos
    initial begin
        $dumpfile("energy_system_all_in_one.vcd");
        $dumpvars(0, tb_energy_system_all_in_one);

        // Inicialização
        reset_n = 0;
        p_demand_low = 0;
        p_demand_high = 0;
        p_idle = 0;
        is_braking = 0;
        battery_button = 1; // 1 = bateria cheia
        #20;
        reset_n = 1;

        // IDLE
        p_idle = 1; #50;

        // Demanda baixa → elétrico
        p_idle = 0;
        p_demand_low = 1; #80;

        // Alta demanda → híbrido
        p_demand_low = 0;
        p_demand_high = 1; #80;

        // Freio regenerativo
        is_braking = 1; #60;
        is_braking = 0; #40;

        // Bateria fraca
        battery_button = 0;
        p_demand_high = 1; #80;

        // Retorna ao IDLE
        p_demand_high = 0;
        p_idle = 1; #60;

        // Bateria cheia novamente
        battery_button = 1;
        p_idle = 0; p_demand_low = 1; #80;

        // Frenagem novamente
        is_braking = 1; #50;
        is_braking = 0; #50;

        #100;
        $finish;
    end

endmodule
