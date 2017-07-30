/*
 * @file sdram_ex_ex.h
 * @author Keidan
 * @copyright GNU LESSER GENERAL PUBLIC LICENSE Version 3
 */

#ifndef __SDRAM_EX_H__
#define __SDRAM_EX_H__

#include <stddef.h>
#include <stdint.h>

#ifndef SDRAM_EX_BASE_ADDRESS
#define SDRAM_EX_BASE_ADDRESS 0xC0000000
#endif /* SDRAM_EX_BASE_ADDRESS */

#ifndef SDRAM_EX_MPU_REGION_SIZE
#define SDRAM_EX_MPU_REGION_SIZE MPU_REGION_SIZE_8MB
#endif /* SDRAM_EX_MPU_REGION_SIZE */


#ifndef SDRAM_EX_HEAP_SIZE
#define SDRAM_EX_HEAP_SIZE ((1024*1024)*2)
#endif /* SDRAM_EX_HEAP_SIZE */

/**
 * @brief Returns the current stack pointer address.
 * @return The SP register address.
 */
int sdram_ex_get_sp(void);


/**
 * @brief Adds the address of the SDRAMM region to the MPU configuration to manage memory alignment.
 * @param[in] addr The SDRAM address (see SDRAM_EX_BASE_ADDRESS).
 * @param[in] size The SDRAM region size (see SDRAM_EX_MPU_REGION_SIZE).
 */
void sdram_ex_mpu_config_add_region(uint32_t addr, uint8_t size);

/**
 * @brief Restarts the MPU (with default privileges) and adds the address of the SDRAMM region to the MPU configuration to manage memory alignment.
 * @param[in] addr The SDRAM address (see SDRAM_EX_BASE_ADDRESS).
 * @param[in] size The SDRAM region size (see SDRAM_EX_MPU_REGION_SIZE).
 */
void sdram_ex_mpu_config(uint32_t addr, uint8_t size);

/**
 * @brief Not optimized memset (in C).
 * Note: The memset function does not restore the stack pointer address correctly
 * @param[out] ptr The address to fill.
 * @param[in] fill The value used to fill.
 * @param[in] length The number of bytes to fill.
 */
void sdram_ex_memset(void* ptr, int fill, size_t length);


/**
 * @brief Not optimized memcpy (in C).
 * Note: The memcpy function does not restore the stack pointer address correctly
 * @param[out] dest The destination address.
 * @param[in] src The source address.
 * @param[in] length The number of bytes to copy.
 */
void sdram_ex_memcpy(void* dest, const void* src, size_t length);

/**
 * @brief Not optimized memcmp (in C).
 * Note: The memcmp function does not restore the stack pointer address correctly
 * @param[int] a The first address to compare.
 * @param[in] b The second address to compare.
 * @param[in] length The number of bytes to compare.
 */
int8_t sdram_ex_memcmp(const void* a, const void* b, size_t length);


/**************************************************/
/*********** HEAP *********************************/
/**************************************************/

/**
 * @brief Checks if the address is in the heap.
 * @param[in] ptr The address to check.
 * @return 1 if the address is in the heap range, 0 else.
 */
uint8_t sdram_ex_is_in_heap(void * ptr);

/**
 * Simulates the malloc function in order to use the SDRAM heap instead of the microcontroller RAM heap.
 * @param size The requested size to allocate.
 * @return The address of the allocated memory.
 */
void *sdram_ex_malloc(size_t size);

/**
 * Simulates the free function in order to use the SDRAM heap instead of the microcontroller RAM heap.
 * @param ptr The address to be freed.
 */
void sdram_ex_free(void *ptr);

#endif /* __SDRAM_EX_H__ */
