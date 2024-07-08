## GD32470VI WiFi-Host-Driver

### 简介
该仓库是对 GD32F470ZIT6 开发板的 WiFi 开源驱动支持包，也可作为物联网开发使用的软件SDK，可以更简单方便的开发物联网程序。

### 开发板介绍

<img src="documents/figures/board.jpg" alt="image-20201009181905422" style="zoom:80%;" />

该开发板常用 **板载资源** 如下：

- GD32F470ZIT6
- On-board USB To UART(CH340)
- USB OTG with Type-C connector
- SDIO TF Card slot
- RGB565 FPC connector
- I8080 FPC connector
- SPI LCD FPC connector
- 32Mbytes SDRAM
- 16Mbytes SPI FLASH
- 256bytes EEPROM
- Power LED(blue) for 3.3 v power-on
- Two user LED1(RED), LED2 (Green)
- Three push-buttons (user and wake and reset)

### 使用说明

使用说明分为如下两个章节：

- 快速上手
  
  本章节是为刚接触 RT-Thread 的新手准备的使用说明，遵循简单的步骤即可将 RT-Thread 操作系统运行在该开发板上，看到实验效果 。

- 进阶使用
  
  本章节是为需要在 RT-Thread 操作系统上使用更多开发板资源的开发者准备的。通过使用 ENV 工具对 BSP 进行配置，可以开启更多板载资源，实现更多高级功能。

### 快速上手

本 BSP 为开发者提供 MDK5工程，支持 GCC 开发环境，也可使用RT-Thread Studio开发。下面以 MDK5 开发环境为例，介绍如何将系统运行起来。

#### 硬件连接

使用调试器连接开发板到 PC，使用USB2TTL连接UART3(PA0&PA1)，并给开发板供电。<br>
不建议使用板载的USB2TTL，因为板载的CH340连接了RTS到MCU的NRST引脚，使用引脚流控会复位芯片。

#### 编译下载

一定要在打开MDK5工程之前先通过`scons --target=mdk5`命令重新生成工程文件。<br>
双击 project.uvprojx 文件，打开 MDK5 工程，编译并下载程序到开发板。

#### 运行结果

下载程序成功之后，系统会自动运行，LED 闪烁。

连接开发板对应串口到 PC , 在终端工具里打开相应的串口（115200-8-1-N），复位设备后，可以看到 RT-Thread 的输出信息:

```bash
 \ | /
- RT -     Thread Operating System
 / | \     5.0.2 build Jul  8 2024 21:57:48
 2006 - 2022 Copyright by RT-Thread team
lwIP-2.0.3 initialized!
[I/FAL] RT-Thread Flash Abstraction Layer initialize success.
[Flash] EasyFlash V4.1.0 is initialize success.
[Flash] You can get the latest version on https://github.com/armink/EasyFlash .
[I/sal.skt] Socket Abstraction Layer initialize success.
[I/FAL] The FAL block device (filesystem) created successfully
msh />RT-Thread WiFi Host Drivers (WHD)
You can get the latest version on https://github.com/Evlers/rt-thread_wifi-host-driver
WLAN MAC Address : C0:AE:FD:00:10:4B
WLAN Firmware    : wl0: Jul 31 2023 06:07:24 version 13.10.271.305 (f2b5c53 CY) FWID 01-e6b954e
WLAN CLM         : API: 18.2 Data: 9.10.0 Compiler: 1.36.1 ClmImport: 1.34.1 Creation: 2022-08-16 03:35:21
WHD VERSION      : 3.1.0.23284 : v3.1.0 : ARM CLANG 5060960 : 2024-03-21 22:57:11 +0800
[I/WLAN.dev] wlan init success
[I/WLAN.lwip] eth device init ok name:w0
[I/WLAN.dev] wlan init success
[I/WLAN.lwip] eth device init ok name:w1
```

### 进阶使用

此 BSP 默认只开启了 GPIO 和 串口1的功能，如果需使用高级功能，需要利用 ENV 工具对BSP 进行配置，步骤如下：

1. 在 bsp 下打开 env 工具。

2. 输入`menuconfig`命令配置工程，配置好之后保存退出。

3. 输入`pkgs --update`命令更新软件包。

4. 输入`scons --target=mdk5/vsc` 命令重新生成工程。

