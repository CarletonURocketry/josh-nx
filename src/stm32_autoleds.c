/****************************************************************************
 * boards/arm/stm32h7/josh/src/stm32_autoleds.c
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
 * TODO: change for number of LEDs on Josh
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdbool.h>
#include <debug.h>

#include <sys/param.h>

#include <nuttx/board.h>
#include <arch/board/board.h>

#include "stm32_gpio.h"
#include "josh.h"

#ifdef CONFIG_ARCH_LEDS

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: board_autoled_initialize
 ****************************************************************************/

void board_autoled_initialize(void)
{
  /* Configure LED GPIO for output */

  stm32_configgpio(GPIO_LED_STARTED);
  stm32_configgpio(GPIO_LED_PANIC);
  stm32_configgpio(GPIO_LED_EJECT);

  stm32_configgpio(GPIO_BUZZER);
  stm32_gpiowrite(GPIO_BUZZER, true);
}

/****************************************************************************
 * Name: board_autoled_on
 ****************************************************************************/

void board_autoled_on(int led)
{
  ledinfo("board_autoled_on(%d)\n", led);

  switch (led)
    {
    case LED_STARTED:

      /* As the board provides only one soft controllable LED, we simply
       * turn it on when the board boots.
       */

      stm32_gpiowrite(GPIO_LED_STARTED, true);
      break;

    case LED_PANIC:
    case LED_ASSERTION:

      /* For panic state, the LED is blinking */

      stm32_gpiowrite(GPIO_LED_PANIC, true);
      break;
    }
}

/****************************************************************************
 * Name: board_autoled_off
 ****************************************************************************/

void board_autoled_off(int led)
{
  switch (led)
    {

    case LED_PANIC:
      stm32_gpiowrite(GPIO_LED_PANIC, false);
      break;

    case LED_STARTED:
      stm32_gpiowrite(GPIO_LED_STARTED, false);
      break;
    }
}

#endif /* CONFIG_ARCH_LEDS */
