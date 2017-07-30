STM32F7_MEMORY_MAPPED_SDRAM
===


This example is based on the following configuration:

* [__STM32F746G-DISCO__](http://www.st.com/en/evaluation-tools/32f746gdiscovery.html)
* [__Ac6 SystemWorkbench__](http://www.openstm32.org)
* [__STM32CubeMX__](http://www.st.com/en/development-tools/stm32cubemx.html) (used to generate the project).

The project demonstrates how to use a memory-mapped SDRAM through the Flexible Memory Controller.

And supports the following features:

| Feature | Functions |
| ------- | --------- |
| Base memory helpers | sdram_ex_memset/sdram_ex_memcpy/sdram_ex_memcmp/sdram_ex_get_sp |
| MPU helpers | sdram_ex_mpu_config_add_region/sdram_ex_mpu_config |
| A simplistic implementation of a memory heap | sdram_ex_malloc/sdram_ex_free/sdram_ex_is_in_heap |
| A section to map code data directly into the SDRAM | \_\_attribute\_\_((section (".sdram_data"))) |


Build
---

You can build the project with [Ac6 SystemWorkbench](http://www.openstm32.org).

Make sure that `HAL_SDRAM_MODULE_ENABLED` is defined (normally done with the project configuration).

__Note:__ If you wants to re-generate the project with [STM32CubeMX](http://www.st.com/en/development-tools/stm32cubemx.html) make sure that the FMC is disabled (it is managed by the BSP part).


Run
---
A tool script is available to read logs from the STM32 ST-Link port.

> python serial_log.py --device (device name or path for linux) --bauds 115200


Files
---

The architecture of the files is not the best, but it has the merit of extracting as much as possible the code of example of the generated code.

| Files | Changes |
| ----- | ------- |
| drv/stm32746g_discovery_sdram.[ch] | Copied from STM32Cube/Repository/STM32Cube_FW_F7_V1.7.0/Drivers/BSP/STM32746G-Discovery (Configured for 8 bits data and 12 bits address).  |
| drv/stm327xx_hal_sdram.c + drv/stm327xx_ll_fmc.c | Copied from STM32Cube/Repository/STM32Cube_FW_F7_V1.7.0/Drivers/STM32F7xx_HAL_Driver/Src |
| drv/stm327xx_hal_sdram.h + drv/stm327xx_ll_fmc.h | Copied from STM32Cube/Repository/STM32Cube_FW_F7_V1.7.0/Drivers/STM32F7xx_HAL_Driver/Inc |
| app.c | Examples of using memory-mapped SDRAM. |
| freertos.c | Generated file I added the call of these functions app_init and app_task_loop. |
| gpio.c | Generated file, no changes. |
| main.c | Generated file, no changes. |
| sdram_ex.c | This file contain a set of helpers function and it define the management of the SDRAM heap implementation (malloc+free). |
| stm32f7xx_hal_msp.c | Generated file, no changes. |
| stm32f7xx_hal_timebase_TIM.c | Generated file, no changes. |
| stm32f7xx_it.c | Generated file, no changes. |
| system_stm32f7xx.c | Generated file, no changes. |
| usart.c | Generated file, I added the the _write function (to redirect printf on the usart). |
| STM32F746NGx_FLASH.ld | Generated file, I added the SDRAM memory section and a new section named sdram_data to easily store the data in the SDRAM from the C code. |


License (like LGPL)
==================

[LGPLv3.txt](./LICENSE)