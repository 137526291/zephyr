
#ifndef ZEPHYR_INCLUDE_DT_BINDINGS_MEMORY_CONTROLLER_STM32_FMC_SRAM_H_
#define ZEPHYR_INCLUDE_DT_BINDINGS_MEMORY_CONTROLLER_STM32_FMC_SRAM_H_


/* Exported constants --------------------------------------------------------*/
/** @addtogroup FMC_LL_Exported_Constants FMC Low Layer Exported Constants
  * @{
  */

/** @defgroup FMC_LL_NOR_SRAM_Controller FMC NOR/SRAM Controller
  * @{
  */

/** @defgroup FMC_NORSRAM_Bank FMC NOR/SRAM Bank
  * @{
  */
#define FMC_NORSRAM_BANK1                       (0x00000000U)
#define FMC_NORSRAM_BANK2                       (0x00000002U)
#define FMC_NORSRAM_BANK3                       (0x00000004U)
#define FMC_NORSRAM_BANK4                       (0x00000006U)
/**
  * @}
  */

/** @defgroup FMC_Data_Address_Bus_Multiplexing FMC Data Address Bus Multiplexing
  * @{
  */
#define FMC_DATA_ADDRESS_MUX_DISABLE            (0x00000000U)
#define FMC_DATA_ADDRESS_MUX_ENABLE             (0x00000002U)
/**
  * @}
  */

/** @defgroup FMC_Memory_Type FMC Memory Type
  * @{
  */
#define FMC_MEMORY_TYPE_SRAM                    (0x00000000U)
#define FMC_MEMORY_TYPE_PSRAM                   (0x00000004U)
#define FMC_MEMORY_TYPE_NOR                     (0x00000008U)
/**
  * @}
  */

/** @defgroup FMC_NORSRAM_Data_Width FMC NORSRAM Data Width
  * @{
  */
#define FMC_NORSRAM_MEM_BUS_WIDTH_8             (0x00000000U)
#define FMC_NORSRAM_MEM_BUS_WIDTH_16            (0x00000010U)
#define FMC_NORSRAM_MEM_BUS_WIDTH_32            (0x00000020U)
/**
  * @}
  */

/** @defgroup FMC_NORSRAM_Flash_Access FMC NOR/SRAM Flash Access
  * @{
  */
#define FMC_NORSRAM_FLASH_ACCESS_ENABLE         (0x00000040U)
#define FMC_NORSRAM_FLASH_ACCESS_DISABLE        (0x00000000U)
/**
  * @}
  */

/** @defgroup FMC_Burst_Access_Mode FMC Burst Access Mode
  * @{
  */
#define FMC_BURST_ACCESS_MODE_DISABLE           (0x00000000U)
#define FMC_BURST_ACCESS_MODE_ENABLE            (0x00000100U)
/**
  * @}
  */

/** @defgroup FMC_Wait_Signal_Polarity FMC Wait Signal Polarity
  * @{
  */
#define FMC_WAIT_SIGNAL_POLARITY_LOW            (0x00000000U)
#define FMC_WAIT_SIGNAL_POLARITY_HIGH           (0x00000200U)
/**
  * @}
  */

/** @defgroup FMC_Wait_Timing FMC Wait Timing
  * @{
  */
#define FMC_WAIT_TIMING_BEFORE_WS               (0x00000000U)
#define FMC_WAIT_TIMING_DURING_WS               (0x00000800U)
/**
  * @}
  */

/** @defgroup FMC_Write_Operation FMC Write Operation
  * @{
  */
#define FMC_WRITE_OPERATION_DISABLE             (0x00000000U)
#define FMC_WRITE_OPERATION_ENABLE              (0x00001000U)
/**
  * @}
  */

/** @defgroup FMC_Wait_Signal FMC Wait Signal
  * @{
  */
#define FMC_WAIT_SIGNAL_DISABLE                 (0x00000000U)
#define FMC_WAIT_SIGNAL_ENABLE                  (0x00002000U)
/**
  * @}
  */

/** @defgroup FMC_Extended_Mode FMC Extended Mode
  * @{
  */
#define FMC_EXTENDED_MODE_DISABLE               (0x00000000U)
#define FMC_EXTENDED_MODE_ENABLE                (0x00004000U)
/**
  * @}
  */

/** @defgroup FMC_AsynchronousWait FMC Asynchronous Wait
  * @{
  */
#define FMC_ASYNCHRONOUS_WAIT_DISABLE           (0x00000000U)
#define FMC_ASYNCHRONOUS_WAIT_ENABLE            (0x00008000U)
/**
  * @}
  */

/** @defgroup FMC_Page_Size FMC Page Size
  * @{
  */
#define FMC_PAGE_SIZE_NONE                      (0x00000000U)
#define FMC_PAGE_SIZE_128                       (FMC_BCRx_CPSIZE_0)
#define FMC_PAGE_SIZE_256                       (FMC_BCRx_CPSIZE_1)
#define FMC_PAGE_SIZE_512                       ((FMC_BCRx_CPSIZE_0 | FMC_BCRx_CPSIZE_1))
#define FMC_PAGE_SIZE_1024                      (FMC_BCRx_CPSIZE_2)
/**
  * @}
  */

/** @defgroup FMC_Write_Burst FMC Write Burst
  * @{
  */
#define FMC_WRITE_BURST_DISABLE                 (0x00000000U)
#define FMC_WRITE_BURST_ENABLE                  (0x00080000U)
/**
  * @}
  */

/** @defgroup FMC_Continous_Clock FMC Continuous Clock
  * @{
  */
#define FMC_CONTINUOUS_CLOCK_SYNC_ONLY          (0x00000000U)
#define FMC_CONTINUOUS_CLOCK_SYNC_ASYNC         (0x00100000U)
/**
  * @}
  */

/** @defgroup FMC_Write_FIFO FMC Write FIFO
  * @{
  */
#define FMC_WRITE_FIFO_DISABLE                  (1UL<<21)
#define FMC_WRITE_FIFO_ENABLE                   (0x00000000U)
/**
  * @}
  */

  /** @defgroup FMC_Access_Mode FMC Access Mode
  * @{
  */
#define FMC_ACCESS_MODE_A                       (0x00000000U)
#define FMC_ACCESS_MODE_B                       (0x10000000U)
#define FMC_ACCESS_MODE_C                       (0x20000000U)
#define FMC_ACCESS_MODE_D                       (0x30000000U)

#endif