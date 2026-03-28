# ESP32 Light Switch

A BLE-enabled smart light switch built on the **ESP32**, developed as part of a custom hardware product lineup. The firmware is written in C using the **ESP-IDF** framework and integrates natively with **Home Assistant** via a dedicated [BLE HAOS integration](https://github.com/ikok07/kok-products-ble-haos-integration).

---

## Features

- **BLE Control** — Toggle the light on/off wirelessly from any BLE-capable client (preferably the HAOS integration)
- **Physical Toggle** — Interrupt-driven physical button for manual control (GPIO 21)
- **Home Assistant Integration** — Works out of the box with my custom [HAOS BLE integration](https://github.com/ikok07/kok-products-ble-haos-integration)
- **Status LED** — RGB LED (GPIO 48) that reflects the current device state at a glance
- **Light Sleep** — CPU frequency scaling and light sleep enabled at runtime to reduce power consumption
- **Dual-core task scheduling** — BLE stack runs on Core 0; light control, status LED, and notification tasks run on Core 1

---

## Hardware

The PCB design files are located in the [`pcb/`](pcb/) directory (KiCad project). Key datasheets and reference material can be found in [`docs/`](docs/).

![PCB 3D-model](https://github.com/ikok07/esp32-light-switch/blob/main/pcb/3d_model.png?raw=true)

| Component         | Purpose             |
|-------------------|---------------------|
| ESP32             | Main MCU            |
| BTB16 - TRIAC     | Mains switching     |
| HLK-PM01          | AC→5 V power supply |
| MIC5205 LDO       | 3.3 V regulation    |
| PC817 Optocoupler | TRIAC isolation     |

---

## Firmware Overview

The project is structured as a standard ESP-IDF project with reusable components:

```
main/           – Application entry point & board-specific configuration
components/
  ├── app_state/        – Global application state (tasks, handles, shared values)
  ├── ble/              – BLE driver (GAP / GATT abstraction over NimBLE)
  ├── logger/           – Logging utilities
  ├── power/            – CPU frequency control & light sleep management
  ├── shared_values/    – Thread-safe shared value store (FreeRTOS-based)
  ├── tasks_scheduler/  – FreeRTOS task lifecycle helpers
  └── timer/            – Timer utilities
```

### Startup sequence

1. Application state initialised
2. Logger configured
3. Power management configured & CPU frequency scaling enabled
4. Status LED initialised → **orange** (configuring)
5. Light control GPIO and ISR configured
6. BLE stack initialised → status LED turns **white** (ready to connect)
7. Light sleep enabled

### Status LED states

| Color     | State |
|-----------|-------|
| 🟠 Orange | Configuring / booting |
| 🔴 Red    | Hardware configuration error |
| 🔵 White  | Ready to connect |
| 🟢 Green  | Connected |
| Other     | BLE configuration error |

---

## BLE Profile

| Property             | Value                                           |
|----------------------|-------------------------------------------------|
| Device name          | `LSwitch`                                       |
| GAP appearance       | `0x04C1` (Generic Switch)                       |
| Advertising interval | 50 ms                                           |
| Security mode        | Disabled / Just Works                           |
| Max connections      | Configured via CONFIG_BT_NIMBLE_MAX_CONNECTIONS |
| Serial number        | `LS001`                                         |

### GATT Services

| Service        | UUID                                             | Description                                                  |
|----------------|--------------------------------------------------|--------------------------------------------------------------|
| Identifier     | `651535-36ae-49aa-354c-ecfec15bc888` (128-bit)   | Used by the HAOS integration to identify the device family   |
| Automation I/O | `0x1815`                                         | Exposes the light state characteristic                       |

**Light State Characteristic** (`0x2A56` – Digital):
- **Read** — returns current relay state (`0` or `1`)
- **Write** — sets relay state (`0` = off, `1` = on)
- **Notify** — pushes state updates to subscribed clients

---

## Home Assistant Integration

Install the [kok-products BLE HAOS integration](https://github.com/ikok07/kok-products-ble-haos-integration) in your Home Assistant instance. The device advertises the identifier service UUID and manufacturer data in its BLE advertisement so that the integration can automatically recognise and add it.

See [`docs/haos_compatibility.md`](docs/haos_compatibility.md) for the full BLE advertisement requirements.

---

## Building & Flashing

Prerequisites: [ESP-IDF v5.x](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/get-started/) installed and sourced.

```bash
# Configure
idf.py set-target esp32

# Build
idf.py build

# Flash & monitor
idf.py -p /dev/tty.usbserial-* flash monitor
```

---

## License

This project is part of a custom hardware product lineup. All rights reserved © Kaloyan Stefanov.
