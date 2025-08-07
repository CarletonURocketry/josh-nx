/****************************************************************************
 * boards/arm/stm32h7/josh/src/josh.h
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

#ifndef __BOARDS_ARM_STM32H7_JOSH_SRC_JOSH_H
#define __BOARDS_ARM_STM32H7_JOSH_SRC_JOSH_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>
#include <nuttx/compiler.h>

#include <stdint.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************/

#define HAVE_USBDEV          1
#define HAVE_USBHOST         1

/* procfs File System */

#ifdef CONFIG_FS_PROCFS
#  ifdef CONFIG_NSH_PROC_MOUNTPOINT
#    define STM32_PROCFS_MOUNTPOINT CONFIG_NSH_PROC_MOUNTPOINT
#  else
#    define STM32_PROCFS_MOUNTPOINT "/proc"
#  endif
#endif

/* Can't support USB host or device features if USB OTG FS is not enabled */

#ifndef CONFIG_STM32H7_OTGFS
#  undef HAVE_USBDEV
#  undef HAVE_USBHOST
#endif

/* Can't support USB device if USB device is not enabled */

#ifndef CONFIG_USBDEV
#  undef HAVE_USBDEV
#endif

/* Can't support USB host is USB host is not enabled */

#ifndef CONFIG_USBHOST
#  undef HAVE_USBHOST
#endif

/* Check if we should enable the USB monitor before starting NSH */

#ifndef CONFIG_USBMONITOR
#  undef HAVE_USBMONITOR
#endif

#ifndef HAVE_USBDEV
#  undef CONFIG_USBDEV_TRACE
#endif

#ifndef HAVE_USBHOST
#  undef CONFIG_USBHOST_TRACE
#endif

#if !defined(CONFIG_USBDEV_TRACE) && !defined(CONFIG_USBHOST_TRACE)
#  undef HAVE_USBMONITOR
#endif

#if !defined(CONFIG_STM32H7_PROGMEM) || !defined(CONFIG_MTD_PROGMEM)
#  undef HAVE_PROGMEM_CHARDEV
#endif

/* Check if we can support the RTC driver */

#define HAVE_RTC_DRIVER 1
#if !defined(CONFIG_RTC) || !defined(CONFIG_RTC_DRIVER)
#  undef HAVE_RTC_DRIVER
#endif

/* LED
 *
 * Josh has three software controllable LEDs on board:
 *   - Started: Green LED to indicate startup
 *   - Panic: Red LED to indicate panic state
 *   - Eject: Green LED to indicate when SD card can be removed safely
 *   - SD Eject: Green LED to indicate when SD card can be removed safely, controllable from user space
 */

#define GPIO_LED_STARTED (GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_50MHz | \
                          GPIO_OUTPUT_CLEAR | GPIO_PORTA | GPIO_PIN4)
#define GPIO_LED_PANIC   (GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_50MHz | \
                          GPIO_OUTPUT_CLEAR | GPIO_PORTA | GPIO_PIN5)
#define GPIO_LED_EJECT   (GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_50MHz | \
                          GPIO_OUTPUT_CLEAR | GPIO_PORTD | GPIO_PIN3)

#define BOARD_NGPIOOUT    1 /* Amount of GPIO Output pins */
#define GPIO_LED_SD_EJECT (GPIO_OUTPUT | GPIO_PUSHPULL | GPIO_SPEED_50MHz | \
                          GPIO_OUTPUT_SET | GPIO_PORTD | GPIO_PIN3)

/* IMU interrupt pins */

#define GPIO_XL_INT                                                            \
  (GPIO_INPUT | GPIO_FLOAT | GPIO_EXTI | GPIO_SPEED_100MHz | GPIO_PORTE |      \
   GPIO_PIN0)

#define GPIO_GY_INT                                                            \
  (GPIO_INPUT | GPIO_FLOAT | GPIO_EXTI | GPIO_SPEED_100MHz | GPIO_PORTE |      \
   GPIO_PIN1)

/* Magnetometer interrupt pin */

#define GPIO_MAG_INT                                                           \
  (GPIO_INPUT | GPIO_FLOAT | GPIO_EXTI | GPIO_SPEED_100MHz | GPIO_PORTD |      \
   GPIO_PIN15)

/* Buzzer
 * Josh has an arming buzzer to indicate when it is armed and running.
 */

#define JOSH_PWMTIMER 1

/* PWM output at buzzer, which is PE13 */

#define GPIO_TIM1_CH3OUT GPIO_TIM1_CH3OUT_2

#if defined(CONFIG_STM32H7_TIM1_PWM)

#if !defined(CONFIG_STM32H7_TIM1_PWM)
#error "Josh requires CONFIG_STM32H7_TIM1_PWM to have PWM"
#endif

#if CONFIG_STM32H7_TIM1_CHANNEL != 3 || !defined(CONFIG_STM32H7_TIM1_CH3OUT)
#error "Timer 1 Channel 3 must be selected for PWM on PE13"
#endif

#endif

/* ADC pins */

#define GPIO_ADC12_INP5 GPIO_ADC12_INP5_0 /* PB1, channel 5 */
#define GPIO_ADC12_INP4 GPIO_ADC12_INP4_0 /* PC4, channle 4 */

#if defined(CONFIG_DISABLE_MOUNTPOINT) || !defined(CONFIG_MMCSD_SDIO)
#  undef HAVE_SDIO
#endif

#define SDIO_SLOTNO        0
#define SDIO_MINOR         0

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_bringup
 *
 * Description:
 *   Perform architecture-specific initialization
 *
 *   CONFIG_BOARD_LATE_INITIALIZE=y :
 *     Called from board_late_initialize().
 *
 *   CONFIG_BOARD_LATE_INITIALIZE=n && CONFIG_BOARDCTL=y &&
 *   CONFIG_NSH_ARCHINIT:
 *     Called from the NSH library
 *
 ****************************************************************************/

int stm32_bringup(void);

/****************************************************************************
 * Name: stm32_sdio_initialize
 *
 * Description:
 *   Initialize SDMMC slot 1 into SDMMC device driver.
 ****************************************************************************/

#ifdef CONFIG_STM32H7_SDMMC
int stm32_sdio_initialize(void);
#endif

/****************************************************************************
 * Name: stm32_pwm
 *
 * Description:
 *   Initialize PWM driver.
 ****************************************************************************/

#ifdef CONFIG_PWM
int stm32_pwm_setup(void);
#endif

/****************************************************************************
 * Name: stm32_dev_gpio_init
 *
 * Description:
 *   Initialize GPIO driver.
 ****************************************************************************/

#ifdef CONFIG_DEV_GPIO
int stm32_dev_gpio_init(void);
#endif

/****************************************************************************
 * Name: stm32_adc_setup
 *
 * Description:
 *   Initialize ADC and register the ADC driver.
 *
 ****************************************************************************/

#ifdef CONFIG_ADC
int stm32_adc_setup(void);
#endif

#endif /* __BOARDS_ARM_STM32H7_JOSH_SRC_JOSH_H */
