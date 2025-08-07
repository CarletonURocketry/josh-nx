/****************************************************************************
 * boards/arm/stm32h7/josh/src/stm32_adc.c
 *
 * SPDX-License-Identifier: Apache-2.0
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

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <errno.h>
#include <debug.h>

#include <nuttx/board.h>
#include <nuttx/analog/adc.h>
#include <arch/board/board.h>

#include "chip.h"
#include "stm32_gpio.h"
#include "stm32_adc.h"
#include "josh.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************/

/* Up to 3 ADC interfaces are supported, Josh only uses ADC2 */

#if defined(CONFIG_STM32H7_ADC2)

/* The number of ADC channels in the conversion list */

#define ADC2_NCHANNELS 2

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * ADC2
 ****************************************************************************/

#ifdef CONFIG_STM32H7_ADC2

static const uint8_t g_adc2_chanlist[ADC2_NCHANNELS] =
{
  4, 5,
};

static const uint32_t g_adc2_pinlist[ADC2_NCHANNELS] =
{
  GPIO_ADC12_INP4,
  GPIO_ADC12_INP5,
};
#endif /* CONFIG_STM32H7_ADC2 */

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: stm32_adc_setup
 *
 * Description:
 *   Initialize ADC and register the ADC driver.
 *
 ****************************************************************************/

int stm32_adc_setup(void)
{
#if defined(CONFIG_STM32H7_ADC2)
  static bool initialized = false;
  struct adc_dev_s *adc;
  int ret;
  int i;
  char devname[] = "/dev/adc0";

  /* Check if we have already initialized */

  if (!initialized)
    {
      /* Configure the pins as analog inputs for the selected channels */

      for (i = 0; i < ADC2_NCHANNELS; i++)
        {
          if (g_adc2_pinlist[i] != 0)
            {
              stm32_configgpio(g_adc2_pinlist[i]);
            }
        }

      /* Call stm32_adcinitialize() to get an instance of the ADC interface */

      adc = stm32h7_adc_initialize(2, g_adc2_chanlist, ADC2_NCHANNELS);
      if (adc == NULL)
        {
          aerr("ERROR: Failed to get ADC2 interface\n");
          return -ENODEV;
        }

      /* Register the ADC driver at "/dev/adc[0-1]" */

      ret = adc_register(devname, adc);
      if (ret < 0)
        {
          aerr("ERROR: adc_register(%s) failed: %d\n", devname, ret);
          return ret;
        }

      devname[8]++;

      /* Now we are initialized */

      initialized = true;
    }

  return OK;
#else
  return -ENOSYS;
#endif
}

#endif /* CONFIG_STM32H7_ADC2 */
