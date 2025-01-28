/****************************************************************************
 * boards/arm/stm32h7/josh/src/stm32_bringup.c
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

#include <debug.h>
#include <errno.h>
#include <sys/types.h>
#include <syslog.h>

#include <arch/board/board.h>

#include <nuttx/fs/fs.h>
#include <nuttx/fs/partition.h>

#include "josh.h"

#include "stm32.h"
#include "stm32_gpio.h"
#include "stm32_sdmmc.h"

#if defined(CONFIG_SENSORS_MS56XX)
#include "stm32_i2c.h"
#include <nuttx/sensors/ms56xx.h>
#endif

/****************************************************************************
 * Private Functions
 ****************************************************************************/

typedef struct {
  int partition_num;
  uint8_t err;
} partition_state_t;

static void partition_handler(struct partition_s *part, void *arg) {
  partition_state_t *partition_handler_state = (partition_state_t *)arg;

  char devname[] = "/dev/mmcsd0p0";

  if (partition_handler_state->partition_num < 10 &&
      part->index == partition_handler_state->partition_num) {
    finfo("Num of sectors: %d \n", part->nblocks);
    devname[sizeof(devname) - 2] = partition_handler_state->partition_num + 48;
    register_blockpartition(devname, 0, "/dev/mmcsd0", part->firstblock,
                            part->nblocks);
    partition_handler_state->err = 0;
  }
}

/****************************************************************************
 * Name: stm32_i2c_register
 *
 * Description:
 *   Register one I2C drivers for the I2C tool.
 *
 ****************************************************************************/

#if defined(CONFIG_I2C) && defined(CONFIG_SYSTEM_I2CTOOL)
static void stm32_i2c_register(int bus) {
  struct i2c_master_s *i2c;
  int ret;

  i2c = stm32_i2cbus_initialize(bus);
  if (i2c == NULL) {
    syslog(LOG_ERR, "ERROR: Failed to get I2C%d interface\n", bus);
  } else {
    i2cinfo("I2C bus %d initialized\n", bus);
    ret = i2c_register(i2c, bus);
    if (ret < 0) {
      syslog(LOG_ERR, "ERROR: Failed to register I2C%d driver: %d\n", bus, ret);
      stm32_i2cbus_uninitialize(i2c);
    }
  }
}
#endif

/****************************************************************************
 * Name: stm32_i2ctool
 *
 * Description:
 *   Register I2C drivers for the I2C tool.
 *
 ****************************************************************************/

#if defined(CONFIG_I2C) && defined(CONFIG_SYSTEM_I2CTOOL)
static void stm32_i2ctool(void) {
  i2cinfo("Registering I2CTOOL busses.");
#ifdef CONFIG_STM32H7_I2C1
  stm32_i2c_register(1);
#endif
#ifdef CONFIG_STM32H7_I2C2
  stm32_i2c_register(2);
#endif
#ifdef CONFIG_STM32H7_I2C3
  stm32_i2c_register(3);
#endif
#ifdef CONFIG_STM32H7_I2C4
  stm32_i2c_register(4);
#endif
}
#endif

/****************************************************************************
 * Public Functions
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

int stm32_bringup(void) {
  int ret = OK;

  /* I2C device drivers */

#if defined(CONFIG_I2C) && defined(CONFIG_SYSTEM_I2CTOOL)
  stm32_i2ctool();
#endif

  /* Sensor drivers */

#if defined(CONFIG_SENSORS_MS56XX)
  /* MS56XX at 0x76 on I2C bus 1 */

  ret = ms56xx_register(stm32_i2cbus_initialize(1), 0, MS56XX_ADDR1,
                        MS56XX_MODEL_MS5607);
  if (ret < 0) {
    syslog(LOG_ERR, "Failed to register MS5607: %d\n", ret);
  }
#endif /* defined(CONFIG_SENSORS_MS56XX) */

#ifdef CONFIG_FS_PROCFS
  /* Mount the procfs file system */

  ret = nx_mount(NULL, STM32_PROCFS_MOUNTPOINT, "procfs", 0, NULL);
  if (ret < 0) {
    syslog(LOG_ERR, "ERROR: Failed to mount the PROC filesystem: %d\n", ret);
  }
#endif /* CONFIG_FS_PROCFS */

#ifdef CONFIG_STM32H7_SDMMC
  ret = stm32_sdio_initialize();
  if (ret < 0) {
    syslog(LOG_ERR, "ERROR: Failed to register SD card device: %d\n.", ret);
  }

  /* Look for both partitions */

  static partition_state_t partitions[] = {
      {.partition_num = 0, .err = ENOENT},
      {.partition_num = 1, .err = ENOENT},
  };

  for (int i = 0; i < 2; i++) {
    parse_block_partition("/dev/mmcsd0", partition_handler, &partitions[i]);
    if (partitions[i].err == ENOENT) {
      fwarn("Partition %d did not register \n", partitions[i].partition_num);
    } else {
      finfo("Partition %d registered! \n", partitions[i].partition_num);
    }
  }

  /* Mount first partitions as FAT file system (user friendly) */

  ret = nx_mount("/dev/mmcsd0p0", "/mnt/usrfs", "vfat", 0, NULL);

  if (ret) {
    ferr("ERROR: Could not mount fat partition %d: \n", ret);
    return ret;
  }

  /* Mount second partition as littlefs file system (power safe)
   * Auto-format because a user cannot feasibly create littlefs system ahead of
   * time, so we auto format to power-safe.
   */

  /*ret = nx_mount("/dev/mmcsd0p1", "/mnt/pwrfs", "littlefs", 0, "autoformat");*/

  /* if (ret) { */
  /*   ferr("ERROR: Could not mount littlefs partition %d: \n", ret); */
  /*   return ret; */
  /* } */

#endif

  return OK;
}
