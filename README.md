# 凸轮控制器
---
  
## 功能简介：
  根据光电编码器测得的角度信息，控制12个通道的高速电磁阀输出；  
  用户通过触摸屏可以设置工作参数（输出开启角、关闭角等）；  
  也可通过六路数字信号输入，对程序运行加以控制；  
  
## 工程结构：
  ../STM32Cube\_FW\_F1\_V1.4.0    从[ST官网](http://www.st.com/content/st_com/en/products/embedded-software/mcus-embedded-software/stm32-embedded-software/stm32cube-embedded-software/stm32cubef1.html)下载的库文件。  
  ../RTT    从[SEGGER官网](https://www.segger.com/jlink-rtt.html)下载的Real Time Transfer调试工具。  
  Controller    本工程文件夹  
  |    .gitignore    git仓库文件更新检测配置脚本  
  |    README.md    本介绍文件  
  |  
  +---Inc    头文件文件夹  
  |   |    main.h    主程序头文件  
  |   |    stm32f1xx\_hal\_conf.h    HAL驱动库的配置头文件  
  |   |    stm32f1xx\_it.h    中断处理函数的头文件  
  |  
  +---Src    源文件文件夹  
  |   |    main.c    主程序源文件  
  |   |    stm32f1xx\_it.c    中断函数源文件  
  |   |  
  |  
  +---toolchain    工具链文件夹  
  |   |    JLinkCommand.jlink    JLink烧录脚本  
  |   |    Makefile    make脚本  
  |   |    STM32F103XC\_FLASH.ld    链接器脚本  
  
