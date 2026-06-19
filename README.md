# ESP32-S3 OV5640 Camera Example

PlatformIO project demonstrating the OV5640 camera module on an ESP32-S3 development board. The hardware is available on Tokopedia (see "Where to Buy" below). This project initializes the camera, captures frames, and streams them over serial for debugging. This project is the answer for overheating issues when using the ESP32-S3 with OV5640 camera module. It is designed to be simple and easy to use for testing and development.

**Contents**
- **Hardware:** ESP32-S3 DevKit + OV5640 camera module
- **Source:** [src/main.cpp](src/main.cpp#L1)
- **Build system:** PlatformIO (see `platformio.ini`)

## Features
- Connect to wi-fi and serve a simple web page with still image capture from the OV5640 camera.
- Initialize and sleep OV5640 camera.

## Build & Upload
Build and flash using PlatformIO. It is recommended to install the PlatformIO IDE extension for your code editor.
From the project root run:

```powershell
pio run --environment esp32-s3-devkitc-1 --target upload
```

Or using the shorthand:

```powershell
pio run -e esp32-s3-devkitc-1 -t upload
```

## Serial Monitor
To view debug output and camera logs:

```powershell
pio device monitor -e esp32-s3-devkitc-1
```

Adjust baud rate / device settings in `platformio.ini` if needed.

## Configuration
- Edit `src/main.cpp` to change camera pins, resolution, or frame format.
- If you use a different ESP32-S3 board or OV5640 breakout, update `platformio.ini` and pin definitions accordingly.

## References
- Camera code & example: [src/main.cpp](src/main.cpp#L1)
- PlatformIO build: `platformio.ini`

## License
This project is licensed under [CC BY-NC-ND](https://creativecommons.org/licenses/by-nc-nd/4.0/). You may use this code for personal or research purposes only. Redistribution is not allowed.

## Disclaimer
The hardware and software in this project are provided "as-is" without any express or implied warranty. The authors, contributors, and sellers (including the Tokopedia store linked below) do not accept responsibility for any damage to devices, data loss, personal injury, or other loss arising from the use, testing, or modification of this product. Use and operation of the device are at your own risk; no guarantee of functionality, longevity, or safety is provided.

## Where to Buy
This hardware is available on Tokopedia (ESP32-S3 camera boards and OV5640/OV2640 modules) Smart Electronics Store:

- https://www.tokopedia.com/smartestore/esp32-s3-cam-ov2640-ov5640-n16r8-1734625360084108941-1735445635402270349

Include this link in your product pages or documentation for customers who want to purchase the exact hardware used in this project.

