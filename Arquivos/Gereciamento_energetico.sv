// ============================================================
// PROJETO COMPLETO - Gerenciamento_energetico.sv
// ============================================================
// Sistema de gerenciamento energético híbrido (FPGA + BitdogLab)
// ------------------------------------------------------------ 
// Data: 01/11/2025
// FPGA: Lattice ECP5 LFE5U-45F
// ============================================================
// FUNCIONALIDADE GERAL:
// - Joystick controla demanda de potência (LOW/HIGH/IDLE).
// - Botão A → freio regenerativo.
// - Botão B → alterna estado da bateria (cheia/baixa).
// - FPGA devolve código de 3 bits (modo operacional) lido pelo Pico.
// ============================================================

module energy_system_all_in_one (
    input  wire clk,
    input  wire reset_n,

    // Entradas - controle de aceleração / freio (Pico → FPGA)
    input  wire p_demand_low,     // GP18 → A18
    input  wire p_demand_high,    // GP19 → C1
    input  wire p_idle,           // GP20 → B1
    input  wire is_braking,       // GP8  → D2

    // Entrada - botão de bateria (Pico → FPGA)
    input  wire battery_button,   // GP9  → K3 (ativo em LOW)

    // Saídas - modo operacional (FPGA → Pico)
    output wire operating_mode0,  // G20 → GP28 (vermelho)
    output wire operating_mode1,  // L18 → GP16 (verde)
    output wire operating_mode2   // L20 → GP17 (azul)
);

    // ============================================================
    // LÓGICA DE BATERIA CONTROLADA PELO BOTÃO B
    // ============================================================
    // Botão ativo em nível BAIXO:
    // - Pressionado → simula bateria BAIXA → entra em DIESEL_CHARGE (se houver demanda).
    // - Solto → simula bateria CHEIA → opera normalmente no modo elétrico.
    // ============================================================

    wire battery_low  = ~battery_button;  // botão pressionado = bateria fraca
    wire battery_high =  battery_button;  // botão solto = bateria cheia
    wire battery_full =  battery_button;  // mesmo sinal (FSM compatível)

    // ============================================================
    // INSTÂNCIA DO GERENCIADOR DE ENERGIA PRINCIPAL
    // ============================================================
    energy_manager_fixed u_energy (
        .clk(clk),
        .reset_n(reset_n),
        .p_demand_low(p_demand_low),
        .p_demand_high(p_demand_high),
        .p_idle(p_idle),
        .is_braking(is_braking),
        .battery_low(battery_low),
        .battery_high(battery_high),
        .battery_full(battery_full),
        .operating_mode0(operating_mode0),
        .operating_mode1(operating_mode1),
        .operating_mode2(operating_mode2)
    );

endmodule


// ============================================================
// MÓDULO INTERNO: energy_manager_fixed
// ============================================================
// Máquina de estados finita (FSM) que decide o modo de operação
// de um sistema híbrido elétrico/diesel, com recuperação de energia.
// ============================================================

module energy_manager_fixed (
    input  wire clk,
    input  wire reset_n,

    // Entradas
    input  wire p_demand_low,
    input  wire p_demand_high,
    input  wire p_idle,
    input  wire is_braking,
    input  wire battery_low,
    input  wire battery_high,
    input  wire battery_full,

    // Saídas (código de 3 bits do modo operacional)
    output reg operating_mode0,
    output reg operating_mode1,
    output reg operating_mode2
);

    // ============================================================
    // DEFINIÇÃO DOS MODOS
    // ============================================================
    parameter IDLE          = 3'b000;
    parameter ELECTRIC      = 3'b001;
    parameter DIESEL_CHARGE = 3'b010;
    parameter HYBRID_ASSIST = 3'b011;
    parameter REGEN_BRAKING = 3'b100;

    reg [2:0] current_mode, next_mode;

    // ============================================================
    // REGISTRADOR DE ESTADO
    // ============================================================
    always @(posedge clk or negedge reset_n) begin
        if (!reset_n)
            current_mode <= IDLE;
        else
            current_mode <= next_mode;
    end

    // ============================================================
    // LÓGICA DE TRANSIÇÃO DE ESTADOS
    // ============================================================
    always @(*) begin
        next_mode = current_mode;

        case (current_mode)
            //-----------------------------------------------------
            IDLE: begin
                // Bateria cheia → modo elétrico / híbrido normal
                if (p_demand_low && !is_braking && !battery_low)
                    next_mode = ELECTRIC;
                else if (p_demand_high && !is_braking && !battery_low)
                    next_mode = HYBRID_ASSIST;
                // Bateria baixa → só entra em diesel se houver demanda (não parado)
                else if ((p_demand_low || p_demand_high) && battery_low && !is_braking)
                    next_mode = DIESEL_CHARGE;
                else
                    next_mode = IDLE;
            end

            //-----------------------------------------------------
            ELECTRIC: begin
                if (p_demand_high && battery_high)
                    next_mode = HYBRID_ASSIST;
                else if (battery_low)
                    next_mode = DIESEL_CHARGE;
                else if (is_braking)
                    next_mode = REGEN_BRAKING;
                else if (p_idle)
                    next_mode = IDLE;
            end

            //-----------------------------------------------------
            DIESEL_CHARGE: begin
                if (battery_full)
                    next_mode = ELECTRIC;
                else if (is_braking)
                    next_mode = REGEN_BRAKING;
                else if (p_idle)
                    next_mode = IDLE;   // sai do diesel se joystick parado
            end

            //-----------------------------------------------------
            HYBRID_ASSIST: begin
                if (is_braking)
                    next_mode = REGEN_BRAKING;
                else if (battery_low)
                    next_mode = DIESEL_CHARGE;
                else if (p_idle)
                    next_mode = IDLE;
            end

            //-----------------------------------------------------
            REGEN_BRAKING: begin
                if (!is_braking && p_idle)
                    next_mode = IDLE;
                else if (!is_braking && p_demand_low)
                    next_mode = ELECTRIC;
            end

            //-----------------------------------------------------
            default: next_mode = IDLE;
        endcase
    end

    // ============================================================
    // SAÍDAS — codificação direta do estado atual
    // ============================================================
    always @(*) begin
        {operating_mode2, operating_mode1, operating_mode0} = current_mode;
    end

endmodule
