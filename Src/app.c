/*
 * @file app.c
 * @author Keidan
 * @copyright GNU LESSER GENERAL PUBLIC LICENSE Version 3
 */
#include "app.h"
#include "stm32746g_discovery_sdram.h"
#include <string.h>
#include <sdram_ex.h>


#define SDRAM_BUFFER_SIZE ((1024*1024))
char sdram_buffer[SDRAM_BUFFER_SIZE] __attribute__((section (".sdram_data")));
struct a_struct {
    int i;
    char b[SDRAM_BUFFER_SIZE];
};
struct a_second_struct {
    uint8_t u8;
    uint32_t u32_1;
    uint32_t u32_2;
};
struct a_struct first_struct __attribute__((section (".sdram_data")));
struct a_second_struct sec_struct __attribute__((section (".sdram_data")));
struct a_struct *third_struct = NULL;
struct a_second_struct *fourth_struct = NULL;

/**
 * @brief Init the SDRAM and configure the MPU.
 */
void app_init(void) {
  printf("SDRAM Initialization ...\r\n");
  /* SDRAM device configuration */
  if (BSP_SDRAM_Init() != SDRAM_OK) {
    printf("SDRAM Initialization : FAILED.\r\n");
    printf("SDRAM Test Aborted.\r\n");
    __asm("bkpt #0");
  }
  else {
    printf("SDRAM Initialization : OK.\r\n");
  }
  /* Configures the MPU */
  sdram_ex_mpu_config(SDRAM_EX_BASE_ADDRESS, SDRAM_EX_MPU_REGION_SIZE);
  /* First use of the SDRAM */
  sdram_ex_memset(&first_struct, 0, SDRAM_BUFFER_SIZE);
}


/**
 * @brief Called by the FreeRTOS main task.
 */
void app_task_loop(void) {
  int z = 0;
  /**
   * The goal of the loop is to :
   * - When z = 0, Cleanup/Free all buffers, either using a manual C loop, either the SDRAM Ex API function.
   * - When z = 1, Allocate, Filling the buffers with some values.
   *
   * Each iteration prints the buffers values.
   * Waits for 1 second.
   */
  int step_free_fourth_struct = 0;
  for(;;) {

    /* Cleanup all buffers, either using a manual C loop, either the SDRAM Ex API function. */
    if(!z) {
      printf("RAZ buffer using C loop.\r\n");
      for(int i = 0; i < SDRAM_BUFFER_SIZE; i++)
        sdram_buffer[i] = 0;
      /* Refill the buffer. */
      strcat(sdram_buffer, "Hello after RAZ.");

      printf("RAZ first_struct using SDRAM Ex API.\r\n");
      sdram_ex_memset(&first_struct, 0, sizeof(struct a_struct));

      printf("RAZ sec_struct using SDRAM Ex API.\r\n");
      sdram_ex_memset(&sec_struct, 0, sizeof(struct a_second_struct));
      /* sdram heap */
      if(third_struct != NULL) {
        printf("FREE third_struct using SDRAM Heap Ex API.\r\n");
        sdram_ex_free(third_struct);
        third_struct = NULL;
        step_free_fourth_struct++;
      }
      if(fourth_struct != NULL && step_free_fourth_struct == 2) {
        printf("FREE fourth_struct using SDRAM Heap Ex API.\r\n");
        sdram_ex_free(fourth_struct);
        fourth_struct = NULL;
        step_free_fourth_struct = 0;
      }
      z++;
    } /* Filling the buffer with some values. */
    else if(z == 1){
      printf("FILL buffer using C loop.\r\n");
      for(int i = 0; i < 50; i++)
        sdram_buffer[i] = 33 + i;

      printf("FILL first_struct using SDRAM Ex API.\r\n");
      sdram_ex_memcpy(first_struct.b, sdram_buffer, SDRAM_BUFFER_SIZE);
      first_struct.i = 1460;

      printf("FILL sec_struct using C.\r\n");
      sec_struct.u8 = 120;
      sec_struct.u32_1 = 30;
      sec_struct.u32_2 = 10;

      /* sdram heap */
      printf("ALLOC third_struct using SDRAM Heap Ex API.\r\n");
      third_struct = sdram_ex_malloc(sizeof(struct a_struct));
      if(third_struct == NULL)
        printf("Unable to allocate memory from the SDRAM heap.\r\n");
      else {
        printf("FILL third_struct using SDRAM Heap Ex API.\r\n");
        const char* t = "Hello from SDRAM heap.";
        third_struct->i = 1515;
        sdram_ex_memcpy(third_struct->b, t, strlen(t));
      }
      if(fourth_struct == NULL) {
        printf("ALLOC fourth_struct using SDRAM Heap Ex API.\r\n");
        fourth_struct = sdram_ex_malloc(sizeof(struct a_second_struct));
        if(fourth_struct == NULL)
          printf("Unable to allocate memory from the SDRAM heap.\r\n");
        else {
          printf("FILL fourth_struct using SDRAM Heap Ex API.\r\n");
          fourth_struct->u8 = 40;
          fourth_struct->u32_1 = 38;
          fourth_struct->u32_2 = 100;
        }
      }
      z = 0;
    }
    /* Prints the buffers values. */
    printf("Z=%d -> READ buffer: \"%s\".\r\n", z, sdram_buffer);
    printf("Z=%d -> READ a_struct: %d -> \"%s\".\r\n", z, first_struct.i, first_struct.b);
    printf("Z=%d -> READ sec_struct: u8=%d, u32_1=%d, u32_2=%d.\r\n", z, sec_struct.u8, (int)sec_struct.u32_1, (int)sec_struct.u32_2);
    printf("Z=%d -> READ compare first_struct.b with sdram_buffer : %d.\r\n", z, sdram_ex_memcmp(first_struct.b, sdram_buffer, 50));
    printf("Z=%d -> READ a_struct: %d -> \"%s\".\r\n", z, (third_struct == NULL) ? -1 : third_struct->i, (third_struct == NULL) ? "NULL" : third_struct->b);
    printf("Z=%d -> READ fourth_struct: u8=%d, u32_1=%d, u32_2=%d.\r\n", z, (fourth_struct == NULL) ? 0 : fourth_struct->u8, (fourth_struct == NULL) ? 0 : (int)fourth_struct->u32_1, (fourth_struct == NULL) ? 0 : (int)fourth_struct->u32_2);
    osDelay(1000);
  }
}
