# ⚡ Sistema de Gerenciamento Energético Híbrido (FPGA + BitDogLab)

Este projeto implementa um **sistema de gerenciamento energético híbrido** totalmente processado em **FPGA (Lattice ECP5)**, responsável por decidir o modo operacional de um sistema híbrido (elétrico/diesel) com **frenagem regenerativa**.
A placa **BitDogLab** é utilizada apenas como **interface de simulação e teste**, enviando sinais de entrada ao FPGA e recebendo de volta os sinais de saída que representam os modos de operação.

---

## 🧠 Visão Geral

O **FPGA atua como o cérebro do sistema**, executando a lógica principal de decisão via **máquina de estados finita (FSM)**.
A **BitDogLab (RP2040)** apenas **simula as condições externas** — joystick, freio e estado da bateria — e **recebe feedback** através dos sinais retornados pelo FPGA.

---

## ⚙️ Funcionalidades Principais

* **Simulação de Entradas (BitDogLab → FPGA):**

  * `p_demand_low`, `p_demand_high`, `p_idle` → simulam o controle de potência.
  * `is_braking` → simula frenagem regenerativa.
  * `battery_button` → simula o estado da bateria (cheia/baixa).

* **Processamento (FPGA):**

  * Interpreta as entradas e define o modo de operação com base na FSM interna.

* **Feedback (FPGA → BitDogLab):**

  * Envia o código de 3 bits (`operating_mode[2:0]`) para LEDs e display, representando o modo atual do sistema.

---

## 🔋 Modos de Operação (FSM)

| Modo            | Código Binário | Descrição                                |
| --------------- | -------------- | ---------------------------------------- |
| `IDLE`          | `000`          | Sistema parado                           |
| `ELECTRIC`      | `001`          | Tração elétrica pura                     |
| `DIESEL_CHARGE` | `010`          | Carregamento da bateria via motor diesel |
| `HYBRID_ASSIST` | `011`          | Modo híbrido (diesel + elétrico)         |
| `REGEN_BRAKING` | `100`          | Frenagem regenerativa ativa              |

---

## 🧩 Estrutura do Projeto

```
FPGA_GERENCIAMENTO_ENERGETICO/
│
├── Gereciamento_energetico.sv       # Módulo principal e FSM
├── tb_energy_system_all_in_one.sv    # Testbench completo
├── Gereciamento_energetico.lpf       # Arquivo de restrições (pinos FPGA)
└── README.md
```

---

## 🧰 Simulação com Icarus Verilog + GTKWave

### 1️⃣ Compilar

```bash
iverilog -o energy_test Gereciamento_energetico.sv tb_energy_system_all_in_one.sv
```

### 2️⃣ Executar simulação

```bash
vvp energy_test
```

### 3️⃣ Abrir no GTKWave

```bash
gtkwave energy_system_all_in_one.vcd
```

---

## 📊 Resultados da Simulação

A FSM alterna corretamente entre os modos de operação conforme as entradas simuladas.
O arquivo `.vcd` gerado exibe as transições esperadas entre os estados:

* **IDLE → ELECTRIC → HYBRID_ASSIST → REGEN_BRAKING → IDLE**

<img src="A_flowchart_diagram_of_a_Hybrid_Energy_Management_.png" width="500">

---

## 🧩 Lógica da FSM

O módulo interno `energy_manager_fixed` é responsável por toda a decisão de estado, baseada nas entradas simuladas:

```verilog
if (p_demand_low && !is_braking && !battery_low)
    next_mode = ELECTRIC;
else if (p_demand_high && !is_braking && !battery_low)
    next_mode = HYBRID_ASSIST;
else if ((p_demand_low || p_demand_high) && battery_low)
    next_mode = DIESEL_CHARGE;
else if (is_braking)
    next_mode = REGEN_BRAKING;
```

---

## 🔌 Mapa de Conexões (BitDogLab ↔ FPGA)

| Função                           | Pico GPIO | FPGA Pino | Direção | Descrição                   |
| -------------------------------- | --------- | --------- | ------- | --------------------------- |
| `p_demand_low`                   | GP18      | A18       | Entrada | Demanda de baixa potência   |
| `p_demand_high`                  | GP19      | C1        | Entrada | Demanda de alta potência    |
| `p_idle`                         | GP20      | B1        | Entrada | Sistema em repouso          |
| `is_braking`                     | GP8       | D2        | Entrada | Freio regenerativo          |
| `battery_button`                 | GP9       | K3        | Entrada | Alterna bateria cheia/baixa |
| `operating_mode0` (LED Vermelho) | GP28      | G20       | Saída   | Bit 0 do modo operacional   |
| `operating_mode1` (LED Verde)    | GP16      | L18       | Saída   | Bit 1 do modo operacional   |
| `operating_mode2` (LED Azul)     | GP17      | L20       | Saída   | Bit 2 do modo operacional   |

> ⚠️ **Observação:**
>
> * O **FPGA processa toda a lógica FSM e define os modos de operação.**
> * A **BitDogLab apenas fornece os sinais de entrada e exibe o feedback visual dos modos**, funcionando como uma interface homem-máquina (HMI) de teste e diagnóstico.

---

## 🚀 Requisitos

* [Icarus Verilog](http://iverilog.icarus.com/)
* [GTKWave](http://gtkwave.sourceforge.net/)
* Lattice Diamond (para síntese FPGA)
* BitDogLab (para simulação e interface de feedback)
