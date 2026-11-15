# Robot tự hành tránh vật cản – STM32F103C8T6 + FreeRTOS

**Toolchain:** Keil MDK / ARM Compiler 6 (armclang)  
**MCU:** STM32F103C8T6 (Cortex-M3, 64 KB Flash / 20 KB SRAM)  
**Driver ngoại vi:** STM32F10x Standard Peripheral Library (SPL)  
**RTOS:** FreeRTOS Kernel, API trực tiếp (`xTaskCreate`, Queue, Semaphore, task notification...)

> Đây là bộ mã nguồn ứng dụng và cấu hình nền tảng do nhóm dự án quản lý. Các thư viện bên thứ ba (SPL/CMSIS/FreeRTOS Kernel) được lấy từ nguồn upstream và phải được đặt vào đúng thư mục theo hướng dẫn bên dưới. Không sao chép HAL vào dự án.

## 1. Phần cứng và pin mặc định

| Chức năng | Chân STM32 | Ghi chú |
|---|---|---|
| HC-SR04 TRIG | PB10 | GPIO output |
| HC-SR04 ECHO | PB6 | Đọc mức bằng polling, TIM4 làm bộ đếm 1 MHz |
| Encoder trái A/B | PA0 / PA1 | TIM2 encoder mode |
| Encoder phải A/B | PA6 / PA7 | TIM3 encoder mode |
| TB6612 PWMA / PWMB | PA8 / PA9 | TIM1 CH1 / CH2 |
| TB6612 AIN1 / AIN2 | PB12 / PB13 | GPIO direction |
| TB6612 BIN1 / BIN2 | PB14 / PB15 | GPIO direction |
| TB6612 STBY | PB5 | HIGH = enable |
| Bluetooth HC-05 TX/RX | PA3 / PA2 | USART2, 9600 baud |
| SWD | PA13 / PA14 | Không dùng làm GPIO |

**An toàn điện:** HC-SR04 ECHO thường là 5 V; dùng cầu chia áp/level shifter trước PB6. Nối chung GND giữa STM32, driver và nguồn động cơ. Cấp nguồn động cơ riêng đúng định mức. Không cấp motor từ chân 3V3. Kiểm tra mức logic, pinout module và dây thực tế trước khi cấp nguồn.

## 2. Cây thư mục

```text
Robot_FreeRTOS/
├── App/
│   ├── Inc/app_config.h
│   ├── Inc/app_tasks.h
│   └── Src/app_tasks.c
├── BSP/
│   ├── Inc/board.h
│   └── Src/board.c
├── Drivers/
│   ├── Inc/motor.h
│   ├── Inc/ultrasonic.h
│   ├── Inc/encoder.h
│   ├── Inc/uart.h
│   └── Src/motor.c
│       Src/ultrasonic.c
│       Src/encoder.c
│       Src/uart.c
├── Config/FreeRTOSConfig.h
├── CMSIS/                 # copy from SPL package
├── SPL/Inc + SPL/Src      # copy from SPL package
├── FreeRTOS/              # copy from FreeRTOS-Kernel
├── Keil/                  # create/open µVision project here
├── main.c
├── stm32f10x_conf.h
└── .gitignore
```

## 3. Third-party sources (required)

### SPL / CMSIS
Use ST's **STM32F10x Standard Peripheral Library V3.6.0** package:
- ST product page: https://www.st.com/en/embedded-software/stsw-stm32054.html
- Copy `Libraries/CMSIS/Include` to `CMSIS/Include`
- Copy `Libraries/CMSIS/Device/ST/STM32F10x/Include` to `CMSIS/Device/ST/STM32F10x/Include`
- Copy `Libraries/STM32F10x_StdPeriph_Driver/inc` to `SPL/Inc`
- Copy `Libraries/STM32F10x_StdPeriph_Driver/src` to `SPL/Src`
- Copy the matching `system_stm32f10x.c` from the package's CMSIS device source into `CMSIS/Device/ST/STM32F10x/Source/Templates`
- Copy the matching startup file `startup_stm32f10x_md.s` from the package's `Project/STM32F10x_StdPeriph_Examples/.../startup` or `Project/STM32F10x_StdPeriph_Template` into `Keil/`.
- Use **medium-density** device (`STM32F10X_MD`) for STM32F103C8T6.

ST legacy SPL is not the modern STM32 HAL. Keep one CMSIS/device/startup set only.

### FreeRTOS Kernel
Download the official kernel: https://github.com/FreeRTOS/FreeRTOS-Kernel
Copy:
- `tasks.c`, `queue.c`, `list.c`, `timers.c`, `event_groups.c`, `stream_buffer.c` (or include only files required by enabled features)
- `include/*` to `FreeRTOS/include`
- `portable/GCC/ARM_CM3/port.c` and `portmacro.h` to `FreeRTOS/portable/GCC/ARM_CM3`
- `portable/MemMang/heap_4.c` to `FreeRTOS/portable/MemMang`

**Important ARM Compiler 6 note:** use the FreeRTOS **GCC/ARM_CM3** Cortex-M3 port with armclang, not `RVDS/ARM_CM3` (the latter is the legacy ARM Compiler 5/armcc port). The official FreeRTOS port list identifies RVDS as armcc; ARM Compiler 6 projects commonly use the GCC ARM_CM3 port. Verify against the selected FreeRTOS kernel release and build output.

## 4. Keil µVision setup

1. Create project `Keil/Robot_FreeRTOS.uvprojx`, select `STMicroelectronics > STM32F103C8`.
2. Device startup: use the SPL package's medium-density startup file exactly once.
3. Set **Options for Target > Target**:
   - IROM1: `0x08000000`, Size `0x00010000`
   - IRAM1: `0x20000000`, Size `0x00005000`
   - Xtal: `8.0 MHz` (Blue Pill commonly has 8 MHz HSE; verify board crystal)
4. Set **Options for Target > C/C++ > Define**:
   - `USE_STDPERIPH_DRIVER`
   - `STM32F10X_MD`
5. Add include paths:
   - `Config`
   - `CMSIS/Include`
   - `CMSIS/Device/ST/STM32F10x/Include`
   - `SPL/Inc`
   - `FreeRTOS/include`
   - `FreeRTOS/portable/GCC/ARM_CM3`
   - `BSP/Inc`, `Drivers/Inc`, `App/Inc`
6. Add project C files:
   - `main.c`, `CMSIS/.../system_stm32f10x.c`
   - SPL source files needed: `misc.c`, `stm32f10x_gpio.c`, `rcc.c`, `usart.c`, `tim.c`, `flash.c`, `gpio.c` as required by the exact package (do not add duplicate files).
   - FreeRTOS: `tasks.c`, `queue.c`, `list.c`, `timers.c` if enabled, `port.c`, `heap_4.c`.
   - All `BSP/Src`, `Drivers/Src`, `App/Src` files.
7. In **Options for Target > C/C++**, use C language, C99 if available. Enable MicroLIB only if your project settings and library calls are compatible; this project does not require printf.
8. In **Options for Target > Linker**, use the scatter file generated for the target or set memory regions to match the device. Do not add a second startup file.
9. Set debugger to ST-Link Debugger, SWD, then build.

The package deliberately does not include a hand-invented `.uvprojx` or startup assembly: those are device-pack/version-sensitive and must match the actual Keil device pack installed. Create the project once in µVision and commit the `.uvprojx` plus startup file to Git.

## 5. Bluetooth command protocol

USART2 receives one character:
- `F`: forward
- `B`: backward
- `L`: rotate left
- `R`: rotate right
- `S`: stop

The current code also autonomously stops if measured distance is below `APP_OBSTACLE_STOP_CM`. The simple command interface is intentionally minimal; it does not implement a phone app or line parser.

## 6. First bring-up sequence

1. Build with motors disconnected.
2. Confirm 1 Hz heartbeat LED on PC13 (Blue Pill LED is commonly active-low).
3. Test UART2 at 9600 baud with a USB-UART adapter (3.3 V logic).
4. Test HC-SR04 with ECHO level-shifted to 3.3 V.
5. Test motor driver with wheels lifted and a current-limited motor supply.
6. Verify motor direction and encoder polarity; swap motor wires or invert signs in `motor.c` if required.
7. Only then test autonomous obstacle stop on the floor.

## 7. Known limitations / values to calibrate

- Motor PWM frequency defaults to approximately 1 kHz at 72 MHz (TIM1 prescaler 71, ARR 999).
- PWM duty is currently fixed at `APP_DEFAULT_DUTY_PERCENT`; speed closed-loop PID is not yet calibrated.
- Encoder counts/revolution and wheel geometry are not assumed. The encoder driver exposes raw signed counts.
- HC-SR04 polling is timeout-bounded but occupies the SensorTask briefly. Do not call it from ISR.
- No claim of physical validation is made. Verify board revision, wiring, timer mapping and motor polarity on the real robot.

## 8. Git workflow

- `main`: tested releases only.
- `develop`: integration branch.
- `feature/<task>`: feature branches from develop.
- Use Pull Requests into develop; never commit directly to main.
- Commit messages: `feat:`, `fix:`, `docs:`, `chore:`, `refactor:`.
