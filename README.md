# ROBOT TỰ HÀNH TRÁNH VẬT CẢN – STM32F103C8T6 + FreeRTOS

**Công cụ phát triển:** Keil MDK / ARM Compiler 6 (armclang)  
**Vi điều khiển:** STM32F103C8T6 (Cortex-M3, Flash 64 KB / SRAM 20 KB)  
**Thư viện điều khiển ngoại vi:** STM32F10x Standard Peripheral Library (SPL)  
**Hệ điều hành thời gian thực:** FreeRTOS Kernel, sử dụng trực tiếp các API như `xTaskCreate`, Queue, Semaphore, Task Notification...

## 1. Phần cứng và sơ đồ chân mặc định

| Chức năng | Chân STM32 | Ghi chú |
|---|---|---|
| HC-SR04 TRIG | PB10 | GPIO đầu ra |
| HC-SR04 ECHO | PB6 | Đọc mức bằng phương pháp polling, sử dụng TIM4 làm bộ đếm 1 MHz |
| Encoder trái A/B | PA0 / PA1 | Chế độ Encoder của TIM2 |
| Encoder phải A/B | PA6 / PA7 | Chế độ Encoder của TIM3 |
| L298N PWMA / PWMB | PA8 / PA9 | TIM1 CH1 / CH2 |
| L298N AIN1 / AIN2 | PB12 / PB13 | GPIO điều khiển chiều quay |
| L298N BIN1 / BIN2 | PB14 / PB15 | GPIO điều khiển chiều quay |
| L298N STBY | PB5 | Mức HIGH = cho phép hoạt động |
| Bluetooth HC-08 TX/RX | PA3 / PA2 | USART2, tốc độ 9600 baud |
| SWD | PA13 / PA14 | Không sử dụng làm GPIO |



## 2. Cấu trúc thư mục dự án

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
├── CMSIS/                 # Sao chép từ gói thư viện SPL
├── SPL/Inc + SPL/Src      # Sao chép từ gói thư viện SPL
├── FreeRTOS/              # Sao chép từ FreeRTOS-Kernel
├── Keil/                  # Chứa project Keil
├── main.c
├── stm32f10x_conf.h
└── .gitignore
```

## 3. Các thư viện bên thứ ba cần sử dụng

### 3.1. Thư viện SPL và CMSIS

Sử dụng gói thư viện **STM32F10x Standard Peripheral Library phiên bản V3.6.0** của ST.

Trang tải thư viện:

https://www.st.com/en/embedded-software/stsw-stm32054.html

Thực hiện sao chép các thư mục sau:

- `Libraries/CMSIS/Include` vào `CMSIS/Include`.
- `Libraries/CMSIS/Device/ST/STM32F10x/Include` vào `CMSIS/Device/ST/STM32F10x/Include`.
- `Libraries/STM32F10x_StdPeriph_Driver/inc` vào `SPL/Inc`.
- `Libraries/STM32F10x_StdPeriph_Driver/src` vào `SPL/Src`.
- Sao chép file `system_stm32f10x.c` tương ứng từ thư viện CMSIS vào `CMSIS/Device/ST/STM32F10x/Source/Templates`.
- Sao chép file khởi động `startup_stm32f10x_md.s` từ thư mục startup tương ứng trong gói SPL vào `Keil/`.

Sử dụng cấu hình vi điều khiển loại **Medium-density** với macro `STM32F10X_MD` cho STM32F103C8T6.

Lưu ý: Thư viện SPL cũ không phải thư viện HAL hiện đại. Chỉ sử dụng một bộ CMSIS, thiết bị và file khởi động phù hợp, tránh đưa các file trùng lặp vào project.

### 3.2. FreeRTOS Kernel

Tải mã nguồn FreeRTOS Kernel chính thức tại:

https://github.com/FreeRTOS/FreeRTOS-Kernel

Các file cần sao chép:

- `tasks.c`
- `queue.c`
- `list.c`
- `timers.c`
- `event_groups.c`
- `stream_buffer.c`

Có thể chỉ đưa vào project những file cần thiết tương ứng với các tính năng được bật.

Sao chép các thư mục và file:

- `include/*` vào `FreeRTOS/include`.
- `portable/GCC/ARM_CM3/port.c` và `portmacro.h` vào `FreeRTOS/portable/GCC/ARM_CM3`.
- `portable/MemMang/heap_4.c` vào `FreeRTOS/portable/MemMang`.

**Lưu ý khi sử dụng ARM Compiler 6:**

Sử dụng port FreeRTOS `GCC/ARM_CM3` dành cho Cortex-M3 với armclang, không sử dụng `RVDS/ARM_CM3` vốn dành cho ARM Compiler 5/armcc cũ.

Cần kiểm tra lại port tương ứng với phiên bản FreeRTOS Kernel được lựa chọn và kết quả biên dịch thực tế.

## 4. Cấu hình Keil µVision

### Bước 1: Tạo project

Tạo project có tên:

`Keil/Robot_FreeRTOS.uvprojx`

Chọn vi điều khiển:

`STMicroelectronics > STM32F103C8`

### Bước 2: Cấu hình file khởi động

Sử dụng file startup dành cho vi điều khiển Medium-density trong gói SPL.

Chỉ thêm một file startup vào project, không thêm trùng lặp.

### Bước 3: Cấu hình bộ nhớ

Trong `Options for Target > Target`, thiết lập:

| Thông số | Giá trị |
|---|---|
| IROM1 Address | `0x08000000` |
| IROM1 Size | `0x00010000` |
| IRAM1 Address | `0x20000000` |
| IRAM1 Size | `0x00005000` |
| Xtal | `8.0 MHz` |

Lưu ý: Blue Pill thường sử dụng thạch anh HSE 8 MHz, cần kiểm tra đúng phiên bản bo mạch thực tế.

### Bước 4: Khai báo macro

Trong `Options for Target > C/C++ > Define`, thêm:

```text
USE_STDPERIPH_DRIVER
STM32F10X_MD
```

### Bước 5: Thêm đường dẫn thư viện

Trong mục Include Paths, thêm:

```text
Config
CMSIS/Include
CMSIS/Device/ST/STM32F10x/Include
SPL/Inc
FreeRTOS/include
FreeRTOS/portable/GCC/ARM_CM3
BSP/Inc
Drivers/Inc
App/Inc
```

### Bước 6: Thêm các file mã nguồn

Các file cần thêm vào project gồm:

- File chính: `main.c`.
- File hệ thống: `system_stm32f10x.c`.
- Các file SPL cần thiết như `misc.c`, `stm32f10x_gpio.c`, `stm32f10x_rcc.c`, `stm32f10x_usart.c`, `stm32f10x_tim.c`, `stm32f10x_flash.c`.
- Các file FreeRTOS: `tasks.c`, `queue.c`, `list.c`, `timers.c` nếu được bật, `port.c`, `heap_4.c`.
- Toàn bộ file mã nguồn cần thiết trong `BSP/Src`, `Drivers/Src`, `App/Src`.

Không thêm các file bị trùng lặp.

### Bước 7: Cấu hình trình biên dịch

Trong `Options for Target > C/C++`:

- Sử dụng ngôn ngữ lập trình C.
- Chọn chuẩn C99 nếu phiên bản trình biên dịch hỗ trợ.
- Chỉ bật MicroLIB nếu cấu hình project và các hàm thư viện đang sử dụng tương thích. Project này không bắt buộc phải sử dụng `printf`.

### Bước 8: Cấu hình Linker và Debug

- Trong `Options for Target > Linker`, sử dụng file scatter do Keil tạo cho vi điều khiển hoặc cấu hình vùng nhớ phù hợp.
- Không thêm file startup thứ hai.
- Chọn `ST-Link Debugger`.
- Sử dụng giao tiếp SWD.
- Tiến hành Build để kiểm tra project.

Lưu ý: Không sử dụng file `.uvprojx` hoặc startup tự tạo không phù hợp với phiên bản Keil và Device Pack đang cài đặt. Cần tạo project trực tiếp trong µVision và đưa file `.uvprojx` cùng file startup vào Git.

## 5. Giao thức điều khiển Bluetooth

Sử dụng giao tiếp USART2 với tốc độ truyền 9600 baud.

Robot nhận một ký tự từ Bluetooth để thực hiện lệnh:

| Ký tự | Chức năng |
|---|---|
| `F` | Tiến về phía trước |
| `B` | Lùi về phía sau |
| `L` | Quay trái |
| `R` | Quay phải |
| `S` | Dừng động cơ |

Ngoài việc nhận lệnh điều khiển từ Bluetooth, chương trình còn có chức năng tự động dừng robot khi cảm biến siêu âm phát hiện vật cản ở khoảng cách nhỏ hơn giá trị `APP_OBSTACLE_STOP_CM`.

Giao thức điều khiển được thiết kế đơn giản, chỉ nhận từng ký tự, chưa triển khai ứng dụng điện thoại hoặc cơ chế phân tích chuỗi lệnh.






