/*
 * @file sdram_ex_ex.c
 * @author Keidan
 * @copyright GNU LESSER GENERAL PUBLIC LICENSE Version 3
 */
#include <sdram_ex.h>
#include "stm32f7xx_hal.h"

enum sdram_ex_ret_e {
  SDRAM_RET_FALSE = 0,
  SDRAM_RET_TRUE = 0,
};
//Struct for block linked list that contains data about the memory blocks
typedef struct sdram_ex_block_s {
  size_t size;
  enum sdram_ex_ret_e free;
  struct sdram_ex_block_s *next;
} sdram_ex_block_t;

static char __sdram_ex_heap[SDRAM_EX_HEAP_SIZE] __attribute__((section (".sdram_data")));
sdram_ex_block_t *__free_blocks = (void*)__sdram_ex_heap;

/**
 * @brief Returns the current stack pointer address.
 * @return The SP register address.
 */
int sdram_ex_get_sp(void) {
  register int sp asm ("sp");
  return sp;
}


/**
 * @brief Adds the address of the SDRAMM region to the MPU configuration to manage memory alignment.
 * @param[in] addr The SDRAM address (see SDRAM_EX_BASE_ADDRESS).
 * @param[in] size The SDRAM region size (see SDRAM_EX_MPU_REGION_SIZE).
 */
void sdram_ex_mpu_config_add_region(uint32_t addr, uint8_t size) {
  MPU_Region_InitTypeDef mpu;
  /* Configure the MPU attributes for SDRAM */
  mpu.Enable = MPU_REGION_ENABLE;
  mpu.BaseAddress = addr;
  mpu.Size = size;
  mpu.AccessPermission = MPU_REGION_FULL_ACCESS;
  mpu.IsBufferable = MPU_ACCESS_BUFFERABLE;
  mpu.IsCacheable = MPU_ACCESS_CACHEABLE;
  mpu.IsShareable = MPU_ACCESS_SHAREABLE;
  mpu.Number = MPU_REGION_NUMBER0;
  mpu.TypeExtField = MPU_TEX_LEVEL1;
  mpu.SubRegionDisable = 0x00;
  mpu.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  HAL_MPU_ConfigRegion(&mpu);
}

/**
 * @brief Restarts the MPU (with default privileges) and adds the address of the SDRAMM region to the MPU configuration to manage memory alignment.
 * @param[in] addr The SDRAM address (see SDRAM_EX_BASE_ADDRESS).
 * @param[in] size The SDRAM region size (see SDRAM_EX_MPU_REGION_SIZE).
 */
void sdram_ex_mpu_config(uint32_t addr, uint8_t size) {
  /* Disable the MPU */
  HAL_MPU_Disable();
  /* Configure the MPU attributes for SDRAM */
  sdram_ex_mpu_config_add_region(addr, size);
  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
 * @brief Not optimized memset (in C).
 * Note: The memset function does not restore the stack pointer address correctly
 * @param[out] ptr The address to fill.
 * @param[in] fill The value used to fill.
 * @param[in] length The number of bytes to fill.
 */
void sdram_ex_memset(void* ptr, int fill, size_t length) {
   while(length--)
     ((char*)ptr)[length] = fill;
}


/**
 * @brief Not optimized memcpy (in C).
 * Note: The memcpy function does not restore the stack pointer address correctly
 * @param[out] dest The destination address.
 * @param[in] src The source address.
 * @param[in] length The number of bytes to copy.
 */
void sdram_ex_memcpy(void* dest, const void* src, size_t length) {
  while(length--)
    ((char*)dest)[length] = ((char*)src)[length];
}

/**
 * @brief Not optimized memcmp (in C).
 * Note: The memcmp function does not restore the stack pointer address correctly
 * @param[int] a The first address to compare.
 * @param[in] b The second address to compare.
 * @param[in] length The number of bytes to compare.
 */
int8_t sdram_ex_memcmp(const void* a, const void* b, size_t length) {
  for(size_t i = 0; i < length; i++)
    if(((char*)a)[i] < ((char*)b)[i])
      return -1;
    else if(((char*)a)[i] > ((char*)b)[i])
      return 1;
  return 0;
}


/**************************************************/
/*********** HEAP *********************************/
/**************************************************/

/**
 * @brief Initializes the heap
 */
static void sdram_ex_initialize_heap(void) {
  __free_blocks->size = SDRAM_EX_HEAP_SIZE - sizeof(sdram_ex_block_t);
  __free_blocks->free = SDRAM_RET_TRUE;
  __free_blocks->next = NULL;
}

/**
 * @brief Checks if the address is in the heap.
 * @param[in] ptr The address to check.
 * @return 1 if the address is in the heap range, 0 else.
 */
uint8_t sdram_ex_is_in_heap(void * ptr) {
  return !((void*)__sdram_ex_heap > ptr || ptr > (void*)(__sdram_ex_heap + SDRAM_EX_HEAP_SIZE));
}

/**
 * @brief Merges blocks to avoid fragmentation of memory.
 */
static void sdram_ex_merge_blocks(void) {
  if (__free_blocks == NULL)
    return;
  sdram_ex_block_t *curr = __free_blocks;
  while (curr != NULL) {
    if (!sdram_ex_is_in_heap(curr))
      return;
    if (curr->free == SDRAM_RET_TRUE && sdram_ex_is_in_heap(curr->next) && curr->next->free == SDRAM_RET_TRUE) {
      curr->size += (curr->next->size + sizeof(sdram_ex_block_t));
      curr->next = curr->next->next;
    }
    else curr = curr->next;
  }
}

/**
 * @brief Splits memory blocks if the current block is larger than the requested block.
 * @param block Current block.
 * @param size The size to split.
 */
static void sdram_ex_split_blocks(sdram_ex_block_t *block, size_t size){
  sdram_ex_block_t *new_block = (void*)((void*)block + size + sizeof(sdram_ex_block_t));
  block->free = SDRAM_RET_FALSE;
  new_block->size = (block->size) - size - sizeof(sdram_ex_block_t);
  new_block->next = block->next;
  new_block->free = SDRAM_RET_TRUE;
  block->size = size;
  block->next = new_block;
}

/**
 * Simulates the malloc function in order to use the SDRAM heap instead of the microcontroller RAM heap.
 * @param size The requested size to allocate.
 * @return The address of the allocated memory.
 */
void *sdram_ex_malloc(size_t size) {
  sdram_ex_block_t *curr;
  void *result = NULL;
  if (size == 0) {
    return result;
  }

  /* Initialize the heap if it is not initialized. */
  if (!__free_blocks->size)
    sdram_ex_initialize_heap();

  //Beginning of heap blocks.
  curr = __free_blocks;
  while (((curr->size) < size || curr->free == 0) && sdram_ex_is_in_heap(curr->next))
    curr = curr->next;

  /* A free block with the same size has been found. */
  if (curr->size == size) {
    curr->free = SDRAM_RET_FALSE;
    result = (void*)(curr + sizeof(sdram_ex_block_t));
  }
  /* The current block is larger than the requested block. */
  else if (curr->size > (size + sizeof(sdram_ex_block_t))){
    sdram_ex_split_blocks(curr, size);
    result = (void*)(curr + sizeof(sdram_ex_block_t));
  }
  //else not enough memory.

  return result;
}

/**
 * Simulates the free function in order to use the SDRAM heap instead of the microcontroller RAM heap.
 * @param ptr The address to be freed.
 */
void sdram_ex_free(void *ptr) {
  if (ptr == NULL)
    return;
  if (sdram_ex_is_in_heap(ptr)) {
    sdram_ex_block_t *curr = ptr;
    curr -= sizeof(sdram_ex_block_t);
    /* If we are in a double-free context, just leave */
    if (curr->free == SDRAM_RET_TRUE) {
      sdram_ex_merge_blocks();
      return;
    }
    curr->free = SDRAM_RET_TRUE;
    sdram_ex_merge_blocks();
  }
}
