/* File: startup_ARMCM7.S
 * Purpose: startup file for Cortex-M7 devices. Should use with
 *   GCC for ARM Embedded Processors
 * Version: V2.0
 * Date: 01 August 2014
 *
 * Copyright (c) 2011 - 2014 ARM LIMITED

   All rights reserved.
   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:
   - Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.
   - Neither the name of ARM nor the names of its contributors may be used
     to endorse or promote products derived from this software without
     specific prior written permission.
   *
   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS AND CONTRIBUTORS BE
   LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
   CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
   SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
   INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
   CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
   ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
   ---------------------------------------------------------------------------*/
    .syntax unified
    .arch   armv7e-m

    .section .stack
    .align  3
#ifdef __STACK_SIZE
    .equ    Stack_Size, __STACK_SIZE
#else
    .equ    Stack_Size, 0xc00
#endif
    .globl  __StackTop
    .globl  __StackLimit
__StackLimit:
    .space  Stack_Size
    .size   __StackLimit, . - __StackLimit
__StackTop:
    .size   __StackTop, . - __StackTop

    .section .heap
    .align  3
#ifdef __HEAP_SIZE
    .equ    Heap_Size, __HEAP_SIZE
#else
    .equ    Heap_Size, 0
#endif
    .globl  __HeapBase
    .globl  __HeapLimit
__HeapBase:
    .if Heap_Size
    .space  Heap_Size
    .endif
    .size   __HeapBase, . - __HeapBase
__HeapLimit:
    .size   __HeapLimit, . - __HeapLimit

    .section .isr_vector
    .align  2
    .globl  __isr_vector
__isr_vector:
    .long   __StackTop            /* Top of Stack */
    .long   Reset_Handler         /* Reset Handler */
    .long   NMI_Handler           /* NMI Handler */
    .long   HardFault_Handler     /* Hard Fault Handler */
    .long   MemManage_Handler     /* MPU Fault Handler */
    .long   BusFault_Handler      /* Bus Fault Handler */
    .long   UsageFault_Handler    /* Usage Fault Handler */
    .long   0                     /* Reserved */
    .long   0                     /* Reserved */
    .long   0                     /* Reserved */
    .long   0                     /* Reserved */
    .long   SVC_Handler           /* SVCall Handler */
    .long   DebugMon_Handler      /* Debug Monitor Handler */
    .long   0                     /* Reserved */
    .long   PendSV_Handler        /* PendSV Handler */
    .long   SysTick_Handler       /* SysTick Handler */

    /* External Interrupts */
    .long    WWDG_IRQHandler                   /* Window WatchDog              */
    .long    PVD_IRQHandler                    /* PVD through EXTI Line detection */
    .long    TAMP_STAMP_IRQHandler             /* Tamper and TimeStamps through the EXTI line */
    .long    RTC_WKUP_IRQHandler               /* RTC Wakeup through the EXTI line */
    .long    FLASH_IRQHandler                  /* FLASH                        */
    .long    RCC_IRQHandler                    /* RCC                          */
    .long    EXTI0_IRQHandler                  /* EXTI Line0                   */
    .long    EXTI1_IRQHandler                  /* EXTI Line1                   */
    .long    EXTI2_IRQHandler                  /* EXTI Line2                   */
    .long    EXTI3_IRQHandler                  /* EXTI Line3                   */
    .long    EXTI4_IRQHandler                  /* EXTI Line4                   */
    .long    DMA1_Stream0_IRQHandler           /* DMA1 Stream 0                */
    .long    DMA1_Stream1_IRQHandler           /* DMA1 Stream 1                */
    .long    DMA1_Stream2_IRQHandler           /* DMA1 Stream 2                */
    .long    DMA1_Stream3_IRQHandler           /* DMA1 Stream 3                */
    .long    DMA1_Stream4_IRQHandler           /* DMA1 Stream 4                */
    .long    DMA1_Stream5_IRQHandler           /* DMA1 Stream 5                */
    .long    DMA1_Stream6_IRQHandler           /* DMA1 Stream 6                */
    .long    ADC_IRQHandler                    /* ADC1, ADC2 and ADC3s         */
    .long    CAN1_TX_IRQHandler                /* CAN1 TX                      */
    .long    CAN1_RX0_IRQHandler               /* CAN1 RX0                     */
    .long    CAN1_RX1_IRQHandler               /* CAN1 RX1                     */
    .long    CAN1_SCE_IRQHandler               /* CAN1 SCE                     */
    .long    EXTI9_5_IRQHandler                /* External Line[9:5]s          */
    .long    TIM1_BRK_TIM9_IRQHandler          /* TIM1 Break and TIM9          */
    .long    TIM1_UP_TIM10_IRQHandler          /* TIM1 Update and TIM10        */
    .long    TIM1_TRG_COM_TIM11_IRQHandler     /* TIM1 Trigger and Commutation and TIM11 */
    .long    TIM1_CC_IRQHandler                /* TIM1 Capture Compare         */
    .long    TIM2_IRQHandler                   /* TIM2                         */
    .long    TIM3_IRQHandler                   /* TIM3                         */
    .long    TIM4_IRQHandler                   /* TIM4                         */
    .long    I2C1_EV_IRQHandler                /* I2C1 Event                   */
    .long    I2C1_ER_IRQHandler                /* I2C1 Error                   */
    .long    I2C2_EV_IRQHandler                /* I2C2 Event                   */
    .long    I2C2_ER_IRQHandler                /* I2C2 Error                   */
    .long    SPI1_IRQHandler                   /* SPI1                         */
    .long    SPI2_IRQHandler                   /* SPI2                         */
    .long    USART1_IRQHandler                 /* USART1                       */
    .long    USART2_IRQHandler                 /* USART2                       */
    .long    USART3_IRQHandler                 /* USART3                       */
    .long    EXTI15_10_IRQHandler              /* External Line[15:10]s        */
    .long    RTC_Alarm_IRQHandler              /* RTC Alarm (A and B) through EXTI Line */
    .long    OTG_FS_WKUP_IRQHandler            /* USB OTG FS Wakeup through EXTI line */
    .long    TIM8_BRK_TIM12_IRQHandler         /* TIM8 Break and TIM12         */
    .long    TIM8_UP_TIM13_IRQHandler          /* TIM8 Update and TIM13        */
    .long    TIM8_TRG_COM_TIM14_IRQHandler     /* TIM8 Trigger and Commutation and TIM14 */
    .long    TIM8_CC_IRQHandler                /* TIM8 Capture Compare         */
    .long    DMA1_Stream7_IRQHandler           /* DMA1 Stream7                 */
    .long    FMC_IRQHandler                    /* FMC                          */
    .long    SDMMC1_IRQHandler                 /* SDMMC1                       */
    .long    TIM5_IRQHandler                   /* TIM5                         */
    .long    SPI3_IRQHandler                   /* SPI3                         */
    .long    UART4_IRQHandler                  /* UART4                        */
    .long    UART5_IRQHandler                  /* UART5                        */
    .long    TIM6_DAC_IRQHandler               /* TIM6 and DAC1&2 underrun errors */
    .long    TIM7_IRQHandler                   /* TIM7                         */
    .long    DMA2_Stream0_IRQHandler           /* DMA2 Stream 0                */
    .long    DMA2_Stream1_IRQHandler           /* DMA2 Stream 1                */
    .long    DMA2_Stream2_IRQHandler           /* DMA2 Stream 2                */
    .long    DMA2_Stream3_IRQHandler           /* DMA2 Stream 3                */
    .long    DMA2_Stream4_IRQHandler           /* DMA2 Stream 4                */
    .long    ETH_IRQHandler                    /* Ethernet                     */
    .long    ETH_WKUP_IRQHandler               /* Ethernet Wakeup through EXTI line */
    .long    CAN2_TX_IRQHandler                /* CAN2 TX                      */
    .long    CAN2_RX0_IRQHandler               /* CAN2 RX0                     */
    .long    CAN2_RX1_IRQHandler               /* CAN2 RX1                     */
    .long    CAN2_SCE_IRQHandler               /* CAN2 SCE                     */
    .long    OTG_FS_IRQHandler                 /* USB OTG FS                   */
    .long    DMA2_Stream5_IRQHandler           /* DMA2 Stream 5                */
    .long    DMA2_Stream6_IRQHandler           /* DMA2 Stream 6                */
    .long    DMA2_Stream7_IRQHandler           /* DMA2 Stream 7                */
    .long    USART6_IRQHandler                 /* USART6                       */
    .long    I2C3_EV_IRQHandler                /* I2C3 event                   */
    .long    I2C3_ER_IRQHandler                /* I2C3 error                   */
    .long    OTG_HS_EP1_OUT_IRQHandler         /* USB OTG HS End Point 1 Out   */
    .long    OTG_HS_EP1_IN_IRQHandler          /* USB OTG HS End Point 1 In    */
    .long    OTG_HS_WKUP_IRQHandler            /* USB OTG HS Wakeup through EXTI */
    .long    OTG_HS_IRQHandler                 /* USB OTG HS                   */
    .long    DCMI_IRQHandler                   /* DCMI                         */
    .long    0                                 /* Reserved                     */
    .long    RNG_IRQHandler                    /* Rng                          */
    .long    FPU_IRQHandler                    /* FPU                          */
    .long    UART7_IRQHandler                  /* UART7                        */
    .long    UART8_IRQHandler                  /* UART8                        */
    .long    SPI4_IRQHandler                   /* SPI4                         */
    .long    SPI5_IRQHandler                   /* SPI5                         */
    .long    SPI6_IRQHandler                   /* SPI6                         */
    .long    SAI1_IRQHandler                   /* SAI1                         */
    .long    LTDC_IRQHandler                   /* LTDC                         */
    .long    LTDC_ER_IRQHandler                /* LTDC error                   */
    .long    DMA2D_IRQHandler                  /* DMA2D                        */
    .long    SAI2_IRQHandler                   /* SAI2                         */
    .long    QUADSPI_IRQHandler                /* QUADSPI                      */
    .long    LPTIM1_IRQHandler                 /* LPTIM1                       */
    .long    CEC_IRQHandler                    /* HDMI_CEC                     */
    .long    I2C4_EV_IRQHandler                /* I2C4 Event                   */
    .long    I2C4_ER_IRQHandler                /* I2C4 Error                   */
    .long    SPDIF_RX_IRQHandler               /* SPDIF_RX                     */

    .size   __isr_vector, . - __isr_vector

    .text
    .thumb
    .thumb_func
    .align  2
    .globl  Reset_Handler
    .type   Reset_Handler, %function
Reset_Handler:
/*  Firstly it copies data from read only memory to RAM. There are two schemes
 *  to copy. One can copy more than one sections. Another can only copy
 *  one section.  The former scheme needs more instructions and read-only
 *  data to implement than the latter.
 *  Macro __STARTUP_COPY_MULTIPLE is used to choose between two schemes.  */

#ifdef __STARTUP_COPY_MULTIPLE
/*  Multiple sections scheme.
 *
 *  Between symbol address __copy_table_start__ and __copy_table_end__,
 *  there are array of triplets, each of which specify:
 *    offset 0: LMA of start of a section to copy from
 *    offset 4: VMA of start of a section to copy to
 *    offset 8: size of the section to copy. Must be multiply of 4
 *
 *  All addresses must be aligned to 4 bytes boundary.
 */
    ldr r4, =__copy_table_start__
    ldr r5, =__copy_table_end__

.L_loop0:
    cmp r4, r5
    bge .L_loop0_done
    ldr r1, [r4]
    ldr r2, [r4, #4]
    ldr r3, [r4, #8]

.L_loop0_0:
    subs    r3, #4
    ittt    ge
    ldrge   r0, [r1, r3]
    strge   r0, [r2, r3]
    bge .L_loop0_0

    adds    r4, #12
    b   .L_loop0

.L_loop0_done:
#else
/*  Single section scheme.
 *
 *  The ranges of copy from/to are specified by following symbols
 *    __etext: LMA of start of the section to copy from. Usually end of text
 *    __data_start__: VMA of start of the section to copy to
 *    __data_end__: VMA of end of the section to copy to
 *
 *  All addresses must be aligned to 4 bytes boundary.
 */
    ldr r1, =__etext
    ldr r2, =__data_start__
    ldr r3, =__data_end__

.L_loop1:
    cmp r2, r3
    ittt    lt
    ldrlt   r0, [r1], #4
    strlt   r0, [r2], #4
    blt .L_loop1
#endif /*__STARTUP_COPY_MULTIPLE */

/*  This part of work usually is done in C library startup code. Otherwise,
 *  define this macro to enable it in this startup.
 *
 *  There are two schemes too. One can clear multiple BSS sections. Another
 *  can only clear one section. The former is more size expensive than the
 *  latter.
 *
 *  Define macro __STARTUP_CLEAR_BSS_MULTIPLE to choose the former.
 *  Otherwise efine macro __STARTUP_CLEAR_BSS to choose the later.
 */
#ifdef __STARTUP_CLEAR_BSS_MULTIPLE
/*  Multiple sections scheme.
 *
 *  Between symbol address __copy_table_start__ and __copy_table_end__,
 *  there are array of tuples specifying:
 *    offset 0: Start of a BSS section
 *    offset 4: Size of this BSS section. Must be multiply of 4
 */
    ldr r3, =__zero_table_start__
    ldr r4, =__zero_table_end__

.L_loop2:
    cmp r3, r4
    bge .L_loop2_done
    ldr r1, [r3]
    ldr r2, [r3, #4]
    movs    r0, 0

.L_loop2_0:
    subs    r2, #4
    itt ge
    strge   r0, [r1, r2]
    bge .L_loop2_0

    adds    r3, #8
    b   .L_loop2
.L_loop2_done:
#elif defined (__STARTUP_CLEAR_BSS)
/*  Single BSS section scheme.
 *
 *  The BSS section is specified by following symbols
 *    __bss_start__: start of the BSS section.
 *    __bss_end__: end of the BSS section.
 *
 *  Both addresses must be aligned to 4 bytes boundary.
 */
    ldr r1, =__bss_start__
    ldr r2, =__bss_end__

    movs    r0, 0
.L_loop3:
    cmp r1, r2
    itt lt
    strlt   r0, [r1], #4
    blt .L_loop3
#endif /* __STARTUP_CLEAR_BSS_MULTIPLE || __STARTUP_CLEAR_BSS */

#ifndef __NO_SYSTEM_INIT
    bl  system_init
#endif

#ifndef __START
#define __START _start
#endif
    bl  __START

    .pool
    .size   Reset_Handler, . - Reset_Handler

    .align  1
    .thumb_func
    .weak   Default_Handler
    .type   Default_Handler, %function
Default_Handler:
    b   .
    .size   Default_Handler, . - Default_Handler

/*    Macro to define default handlers. Default handler
 *    will be weak symbol and just dead loops. They can be
 *    overwritten by other handlers */
    .macro  def_irq_handler handler_name
    .weak   \handler_name
    .set    \handler_name, Default_Handler
    .endm

    def_irq_handler    NMI_Handler
    def_irq_handler    HardFault_Handler
    def_irq_handler    MemManage_Handler
    def_irq_handler    BusFault_Handler
    def_irq_handler    UsageFault_Handler
    def_irq_handler    SVC_Handler
    def_irq_handler    DebugMon_Handler
    def_irq_handler    PendSV_Handler
    def_irq_handler    SysTick_Handler
    def_irq_handler    DEF_IRQHandler
    def_irq_handler    WWDG_IRQHandler
    def_irq_handler    PVD_IRQHandler
    def_irq_handler    TAMP_STAMP_IRQHandler
    def_irq_handler    RTC_WKUP_IRQHandler
    def_irq_handler    FLASH_IRQHandler
    def_irq_handler    RCC_IRQHandler
    def_irq_handler    EXTI0_IRQHandler
    def_irq_handler    EXTI1_IRQHandler
    def_irq_handler    EXTI2_IRQHandler
    def_irq_handler    EXTI3_IRQHandler
    def_irq_handler    EXTI4_IRQHandler
    def_irq_handler    DMA1_Stream0_IRQHandler
    def_irq_handler    DMA1_Stream1_IRQHandler
    def_irq_handler    DMA1_Stream2_IRQHandler
    def_irq_handler    DMA1_Stream3_IRQHandler
    def_irq_handler    DMA1_Stream4_IRQHandler
    def_irq_handler    DMA1_Stream5_IRQHandler
    def_irq_handler    DMA1_Stream6_IRQHandler
    def_irq_handler    ADC_IRQHandler
    def_irq_handler    CAN1_TX_IRQHandler
    def_irq_handler    CAN1_RX0_IRQHandler
    def_irq_handler    CAN1_RX1_IRQHandler
    def_irq_handler    CAN1_SCE_IRQHandler
    def_irq_handler    EXTI9_5_IRQHandler
    def_irq_handler    TIM1_BRK_TIM9_IRQHandler
    def_irq_handler    TIM1_UP_TIM10_IRQHandler
    def_irq_handler    TIM1_TRG_COM_TIM11_IRQHandler
    def_irq_handler    TIM1_CC_IRQHandler
    def_irq_handler    TIM2_IRQHandler
    def_irq_handler    TIM3_IRQHandler
    def_irq_handler    TIM4_IRQHandler
    def_irq_handler    I2C1_EV_IRQHandler
    def_irq_handler    I2C1_ER_IRQHandler
    def_irq_handler    I2C2_EV_IRQHandler
    def_irq_handler    I2C2_ER_IRQHandler
    def_irq_handler    SPI1_IRQHandler
    def_irq_handler    SPI2_IRQHandler
    def_irq_handler    USART1_IRQHandler
    def_irq_handler    USART2_IRQHandler
    def_irq_handler    USART3_IRQHandler
    def_irq_handler    EXTI15_10_IRQHandler
    def_irq_handler    RTC_Alarm_IRQHandler
    def_irq_handler    OTG_FS_WKUP_IRQHandler
    def_irq_handler    TIM8_BRK_TIM12_IRQHandler
    def_irq_handler    TIM8_UP_TIM13_IRQHandler
    def_irq_handler    TIM8_TRG_COM_TIM14_IRQHandler
    def_irq_handler    TIM8_CC_IRQHandler
    def_irq_handler    DMA1_Stream7_IRQHandler
    def_irq_handler    FMC_IRQHandler
    def_irq_handler    SDMMC1_IRQHandler
    def_irq_handler    TIM5_IRQHandler
    def_irq_handler    SPI3_IRQHandler
    def_irq_handler    UART4_IRQHandler
    def_irq_handler    UART5_IRQHandler
    def_irq_handler    TIM6_DAC_IRQHandler
    def_irq_handler    TIM7_IRQHandler
    def_irq_handler    DMA2_Stream0_IRQHandler
    def_irq_handler    DMA2_Stream1_IRQHandler
    def_irq_handler    DMA2_Stream2_IRQHandler
    def_irq_handler    DMA2_Stream3_IRQHandler
    def_irq_handler    DMA2_Stream4_IRQHandler
    def_irq_handler    ETH_IRQHandler
    def_irq_handler    ETH_WKUP_IRQHandler
    def_irq_handler    CAN2_TX_IRQHandler
    def_irq_handler    CAN2_RX0_IRQHandler
    def_irq_handler    CAN2_RX1_IRQHandler
    def_irq_handler    CAN2_SCE_IRQHandler
    def_irq_handler    OTG_FS_IRQHandler
    def_irq_handler    DMA2_Stream5_IRQHandler
    def_irq_handler    DMA2_Stream6_IRQHandler
    def_irq_handler    DMA2_Stream7_IRQHandler
    def_irq_handler    USART6_IRQHandler
    def_irq_handler    I2C3_EV_IRQHandler
    def_irq_handler    I2C3_ER_IRQHandler
    def_irq_handler    OTG_HS_EP1_OUT_IRQHandler
    def_irq_handler    OTG_HS_EP1_IN_IRQHandler
    def_irq_handler    OTG_HS_WKUP_IRQHandler
    def_irq_handler    OTG_HS_IRQHandler
    def_irq_handler    DCMI_IRQHandler
    def_irq_handler    RNG_IRQHandler
    def_irq_handler    FPU_IRQHandler
    def_irq_handler    UART7_IRQHandler
    def_irq_handler    UART8_IRQHandler
    def_irq_handler    SPI4_IRQHandler
    def_irq_handler    SPI5_IRQHandler
    def_irq_handler    SPI6_IRQHandler
    def_irq_handler    SAI1_IRQHandler
    def_irq_handler    LTDC_IRQHandler
    def_irq_handler    LTDC_ER_IRQHandler
    def_irq_handler    DMA2D_IRQHandler
    def_irq_handler    SAI2_IRQHandler
    def_irq_handler    QUADSPI_IRQHandler
    def_irq_handler    LPTIM1_IRQHandler
    def_irq_handler    CEC_IRQHandler
    def_irq_handler    I2C4_EV_IRQHandler
    def_irq_handler    I2C4_ER_IRQHandler
    def_irq_handler    SPDIF_RX_IRQHandler
   .end
