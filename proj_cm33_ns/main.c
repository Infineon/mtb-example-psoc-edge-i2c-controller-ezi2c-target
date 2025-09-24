/*******************************************************************************
* File Name        : main.c
*
* Description      : This source file contains the main routine for non-secure
*                    application of I2C Controller EZI2C Target in the CM33 CPU.
*
* Related Document : See README.md
*
********************************************************************************
* Copyright 2023-2025, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/

/*******************************************************************************
* Header Files
*******************************************************************************/
#include "retarget_io_init.h"


/*******************************************************************************
* Macros
*******************************************************************************/
/* This code example requires two I2C ports, one as controller and the other one
 * as target. You can configure the kit as I2C_MODE_CONTROLLER or I2C_MODE_TARGET
 * or I2C_MODE_BOTH. See README.md to know more on the kit specific
 * configuration. */
 
#define I2C_MODE                   (I2C_MODE_BOTH)
#define I2C_MODE_BOTH              (0U)
#define I2C_MODE_CONTROLLER        (1U)
#define I2C_MODE_TARGET            (2U)
#define CMD_TO_CMD_DELAY           (1000UL)
#define TX_PACKET_INIT_POS         (0UL)
#define TX_PACKET_SOP_POS          (1UL)
#define TX_PACKET_CMD_POS          (2UL)
#define TX_PACKET_EOP_POS          (3UL)
#define PACKET_RPLY_SOP_POS        (3UL)
#define PACKET_RPLY_STS_POS        (4UL)
#define PACKET_RPLY_EOP_POS        (5UL)
#define STS_CMD_DONE               (0x00UL)
#define STS_CMD_FAIL               (0xFFUL)
#define EZI2C_BUFFER_SIZE          (6UL)
#define TX_PACKET_SIZE             (6UL)
#define TX_WRITE_PACKET_SIZE       (4UL)
#define EzPACKET_SOP_POS           (0x00UL)
#define EzPACKET_CMD_POS           (0x01UL)
#define EzPACKET_EOP_POS           (0x02UL)
#define I2C_SUCCESS                (0UL)
#define I2C_FAILURE                (1UL)
#define I2C_TARGET_GENERIC_ADDRESS (0UL)
#define TX_WRITE_READ_TIMEOUT      (0UL)
#define SIZE_VALUE_ZERO            (0UL)
#define SIZE_VALUE_ONE             (1UL)
#define PACKET_SOP                 (0x01UL)
#define PACKET_EOP                 (0x17UL)
#define I2C_TARGET_ADDR            (0x24UL)
#define I2C_TARGET_IRQ_PRIORITY    (7UL)
#define EZI2C_BUFF_CLR_VAL         (0UL)
#define ZERO_VALUE                 (0UL)
#define BUFFER_INIT_VAL            (0UL)
#define BUFFER_SIZE_INIT_VAL       (0UL)

/* The timeout value in microsecond used to wait for core to be booted */
#define CM55_BOOT_WAIT_TIME_USEC   (10U)


/*******************************************************************************
* Global Variables
*******************************************************************************/
uint32_t status;

#if ((I2C_MODE_BOTH == I2C_MODE) || (I2C_MODE_TARGET == I2C_MODE))
uint8_t ezi2c_buffer[EZI2C_BUFFER_SIZE] = {BUFFER_INIT_VAL};
cy_stc_sysint_t CYBSP_EZ_I2C_TARGET_SCB_IRQ_cfg =
{
    /* Source of interrupt signal */      
    CYBSP_EZ_I2C_TARGET_IRQ,
    /* Interrupt priority */
    I2C_TARGET_IRQ_PRIORITY
};
cy_stc_scb_ezi2c_context_t CYBSP_EZ_I2C_TARGET_context;
#endif

#if ((I2C_MODE_BOTH == I2C_MODE) || (I2C_MODE_CONTROLLER == I2C_MODE))
cy_stc_scb_i2c_context_t CYBSP_I2C_CONTROLLER_context;
uint8_t  buffer[TX_PACKET_SIZE] = {BUFFER_INIT_VAL};
uint8_t size;
uint8_t *data;
uint8_t cmd = CYBSP_LED_STATE_ON;
#endif

/*******************************************************************************
* Function Definitions
********************************************************************************/

#if ((I2C_MODE_BOTH == I2C_MODE) || (I2C_MODE_CONTROLLER == I2C_MODE))
/*******************************************************************************
* Function Name: I2C_controller_write
********************************************************************************
* Summary: 
*  Function for controller to write content to the target.
*
* Parameters:  
*  None
*
* Return: 
*  None
*
*******************************************************************************/
uint32_t I2C_controller_write()
{
    size = TX_WRITE_PACKET_SIZE;
    data = &buffer[TX_PACKET_INIT_POS];
    cy_en_scb_i2c_status_t status = CY_SCB_I2C_SUCCESS;

    /* Controller send start command */
    if (CYBSP_I2C_CONTROLLER_context.state == CY_SCB_I2C_IDLE)
    {
        status = Cy_SCB_I2C_MasterSendStart(CYBSP_I2C_CONTROLLER_HW,
            I2C_TARGET_ADDR, CY_SCB_I2C_WRITE_XFER, TX_WRITE_READ_TIMEOUT,
            &CYBSP_I2C_CONTROLLER_context);
    }
    else
    {
        status = Cy_SCB_I2C_MasterSendReStart(CYBSP_I2C_CONTROLLER_HW,
            I2C_TARGET_ADDR, CY_SCB_I2C_WRITE_XFER, TX_WRITE_READ_TIMEOUT,
            &CYBSP_I2C_CONTROLLER_context);
    }

    if (CY_SCB_I2C_SUCCESS == status)
    {
        while (SIZE_VALUE_ZERO < size)
        {
            /* Controller write byte command */
            status = Cy_SCB_I2C_MasterWriteByte(CYBSP_I2C_CONTROLLER_HW, *data,
                     TX_WRITE_READ_TIMEOUT, &CYBSP_I2C_CONTROLLER_context);

            if (CY_SCB_I2C_SUCCESS != status)
            {
                break;
            }
            --size;
            ++data;
        }
    }

    /* Controller send stop command */
    Cy_SCB_I2C_MasterSendStop(CYBSP_I2C_CONTROLLER_HW,TX_WRITE_READ_TIMEOUT,
            &CYBSP_I2C_CONTROLLER_context);

    return status;
}

/*******************************************************************************
* Function Name: I2C_controller_read
********************************************************************************
* Summary:
*  Function for controller to read reply after controller send function.
*
* Parameters:  
*  None
*
* Return: 
*  None
*
*******************************************************************************/
uint32_t I2C_controller_read()
{
    cy_en_scb_i2c_command_t ack = CY_SCB_I2C_ACK;
    size = TX_PACKET_SIZE;
    data = &buffer[TX_PACKET_INIT_POS];
    cy_en_scb_i2c_status_t status = CY_SCB_I2C_SUCCESS;

    if (CYBSP_I2C_CONTROLLER_context.state == CY_SCB_I2C_IDLE)
    {
        status = Cy_SCB_I2C_MasterSendStart(CYBSP_I2C_CONTROLLER_HW,
            I2C_TARGET_ADDR, CY_SCB_I2C_READ_XFER, TX_WRITE_READ_TIMEOUT,
            &CYBSP_I2C_CONTROLLER_context);
    }
    else
    {
        status = Cy_SCB_I2C_MasterSendReStart( CYBSP_I2C_CONTROLLER_HW,
            I2C_TARGET_ADDR, CY_SCB_I2C_READ_XFER, TX_WRITE_READ_TIMEOUT,
            &CYBSP_I2C_CONTROLLER_context);
    }

    if (CY_SCB_I2C_SUCCESS == status)
    {
        while (SIZE_VALUE_ZERO < size)
        {
            if (SIZE_VALUE_ONE == size)
            {
                ack = CY_SCB_I2C_NAK;
            }
            /* Controller read byte command */
            status = Cy_SCB_I2C_MasterReadByte(CYBSP_I2C_CONTROLLER_HW,
                    ack,(uint8_t *)data,TX_WRITE_READ_TIMEOUT,
                    &CYBSP_I2C_CONTROLLER_context);
            if (CY_SCB_I2C_SUCCESS != status)
            {
                break;
            }
            --size;
            ++data;
        }
    }
    
    /* Controller send stop command */
    Cy_SCB_I2C_MasterSendStop(CYBSP_I2C_CONTROLLER_HW, TX_WRITE_READ_TIMEOUT,
                             &CYBSP_I2C_CONTROLLER_context);
    return status;
}

/*******************************************************************************
* Function Name: controller_init
********************************************************************************
* Summary: 
*  This function initiates and enables the controller SCB.
*
* Parameters: 
*  None
*
* Return: 
*  Status of initialization.
*
*******************************************************************************/
uint32_t controller_init(void)
{
    printf(">> Configuring I2C controller.....\r\n");
    cy_en_scb_i2c_status_t initStatus = CY_SCB_I2C_SUCCESS;

    /* Initialize and enable the I2C in controller mode. */
    initStatus = Cy_SCB_I2C_Init(CYBSP_I2C_CONTROLLER_HW,
                 &CYBSP_I2C_CONTROLLER_config, &CYBSP_I2C_CONTROLLER_context);

    if(CY_SCB_I2C_SUCCESS != initStatus)
    {
        handle_app_error();
    }

    /* Enable I2C controller hardware. */
    Cy_SCB_I2C_Enable(CYBSP_I2C_CONTROLLER_HW);
    printf("Done\r\n\n");

    return I2C_SUCCESS;
}
#endif

#if ((I2C_MODE_BOTH == I2C_MODE) || (I2C_MODE_TARGET == I2C_MODE))
/*******************************************************************************
* Function Name: CYBSP_EZ_I2C_TARGET_InterruptHandler
********************************************************************************
* Summary:
*  This function executes interrupt service routine.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void CYBSP_EZ_I2C_TARGET_InterruptHandler(void)
{
    /* ISR implementation for EZI2C. */
    Cy_SCB_EZI2C_Interrupt(CYBSP_EZ_I2C_TARGET_HW,
                          &CYBSP_EZ_I2C_TARGET_context);

    /* Read the EZi2C status */
    uint32_t ezi2cState = Cy_SCB_EZI2C_GetActivity(CYBSP_EZ_I2C_TARGET_HW,
                          &CYBSP_EZ_I2C_TARGET_context);

    /* Write complete without errors: parse packets, otherwise ignore. */
    if((ZERO_VALUE != (ezi2cState & CY_SCB_EZI2C_STATUS_WRITE1)) &&
        (ZERO_VALUE == (ezi2cState & CY_SCB_EZI2C_STATUS_ERR)))
    {
        /* Check buffer content to know any new packets are written from
        controller. */
        if((PACKET_SOP == ezi2c_buffer[EzPACKET_SOP_POS]) &&
           (PACKET_EOP == ezi2c_buffer[EzPACKET_EOP_POS]))
        {
            Cy_GPIO_Write(CYBSP_USER_LED_PORT, CYBSP_USER_LED_NUM,
                          ezi2c_buffer[EzPACKET_CMD_POS]);

            /* Clear the location so that any new packets written to buffer
            will be known. */
            ezi2c_buffer[EzPACKET_SOP_POS]      = EZI2C_BUFF_CLR_VAL;
            ezi2c_buffer[EzPACKET_EOP_POS]      = EZI2C_BUFF_CLR_VAL;

            /* Write to buffer the data related to status. */
            ezi2c_buffer[PACKET_RPLY_SOP_POS] = PACKET_SOP;
            ezi2c_buffer[PACKET_RPLY_STS_POS] = STS_CMD_DONE;
            ezi2c_buffer[PACKET_RPLY_EOP_POS] = PACKET_EOP;
        }
        else
        {
            /* Write to buffer the data related to status. */
            ezi2c_buffer[PACKET_RPLY_SOP_POS] = PACKET_SOP;
            ezi2c_buffer[PACKET_RPLY_STS_POS] = STS_CMD_FAIL;
            ezi2c_buffer[PACKET_RPLY_EOP_POS] = PACKET_EOP;
        }
    }
}
#endif

#if ((I2C_MODE_BOTH == I2C_MODE) || (I2C_MODE_TARGET == I2C_MODE))
/*******************************************************************************
* Function Name: target_init
********************************************************************************
* Summary: 
*  This function initiates and enables the target SCB.
*
* Parameters: 
*  None
*
* Return: 
*  Status of initialization.
*
*******************************************************************************/
uint32_t target_init(void)
{
    int result = I2C_FAILURE;

    /* Configure EzI2C target */
    printf(">> Configuring EzI2C target.....\r\n");
    cy_en_scb_ezi2c_status_t initEzI2Cstatus;
    cy_en_sysint_status_t sysEzI2Cstatus;

    /*Initialize EzI2C target.*/
    initEzI2Cstatus = Cy_SCB_EZI2C_Init(CYBSP_EZ_I2C_TARGET_HW,
            &CYBSP_EZ_I2C_TARGET_config,&CYBSP_EZ_I2C_TARGET_context);

    if(CY_SCB_EZI2C_SUCCESS == initEzI2Cstatus)
    {
        /* Hook interrupt service routine and enable interrupt. */
        sysEzI2Cstatus = Cy_SysInt_Init(&CYBSP_EZ_I2C_TARGET_SCB_IRQ_cfg,
                                        &CYBSP_EZ_I2C_TARGET_InterruptHandler);

        if(CY_SYSINT_SUCCESS == sysEzI2Cstatus)
        {
            NVIC_EnableIRQ((IRQn_Type) CYBSP_EZ_I2C_TARGET_SCB_IRQ_cfg.intrSrc);

            /* Configure buffer for communication with controller. */
            Cy_SCB_EZI2C_SetBuffer1(CYBSP_EZ_I2C_TARGET_HW, ezi2c_buffer,
                                    EZI2C_BUFFER_SIZE,EZI2C_BUFFER_SIZE,
                                    &CYBSP_EZ_I2C_TARGET_context);

            /* Enable SCB for the EZI2C operation. */
            Cy_SCB_EZI2C_Enable(CYBSP_EZ_I2C_TARGET_HW);
            printf("Done\r\n\n");
            result = I2C_SUCCESS;
        }
    }

    return result;
}
#endif

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
*  This is the main function. It performs the below functions.
*    1. Initializes BSP (Board Support Package) and retarget-io for UART logs.
*    2. Initializes I2C controller and EzI2C target.
*    3. I2C controller sends command packet to the target.
*    4. I2C controller reads the response packet to generate the next command.
*
* Parameters:
*  void
*
* Return:
*  Int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result = CY_RSLT_SUCCESS;

    /* Set up the device based on configurator selections */
    result = cybsp_init();
    if (CY_RSLT_SUCCESS != result)
    {
        handle_app_error();
    }

    /* Initialize retarget-io middleware */
    init_retarget_io();

    /* CY_CM55_APP_BOOT_ADDR must be updated if CM55 memory layout is changed.*/
    Cy_SysEnableCM55(MXCM55, CY_CM55_APP_BOOT_ADDR, CM55_BOOT_WAIT_TIME_USEC);

    /* \x1b[2J\x1b[;H - ANSI ESC sequence for clear screen */
    printf("\x1b[2J\x1b[;H");
    printf("****************** ");
    printf("PSOC Edge MCU: I2C Controller EzI2C Target ");
    printf("******************\r\n\n");

#if ((I2C_MODE_BOTH == I2C_MODE) || (I2C_MODE_CONTROLLER == I2C_MODE))
    /*Initiate and enable controller SCB*/
    if(I2C_FAILURE == controller_init())
    {
        handle_app_error();
    }
#endif

#if ((I2C_MODE_BOTH == I2C_MODE) || (I2C_MODE_TARGET == I2C_MODE))
    /*Initiate and enable target SCB*/
    if(I2C_FAILURE ==  target_init())
    {
        handle_app_error();
    }
#endif

   /* Enable interrupts */
   __enable_irq();

#if ((I2C_MODE_BOTH == I2C_MODE) || (I2C_MODE_TARGET == I2C_MODE))
    printf("USER LED 1 on the kit should start blinking \r\n");
#endif

    for (;;)
    {
#if ((I2C_MODE_BOTH == I2C_MODE) || (I2C_MODE_CONTROLLER == I2C_MODE))

        /* create packet to be sent to target. */
        buffer[TX_PACKET_INIT_POS] = I2C_TARGET_GENERIC_ADDRESS;
        buffer[TX_PACKET_SOP_POS]  = PACKET_SOP;
        buffer[TX_PACKET_CMD_POS]  = cmd;
        buffer[TX_PACKET_EOP_POS]  = PACKET_EOP;

        /*Write contents of buffer to target*/
        if(I2C_FAILURE == I2C_controller_write())
        {
            handle_app_error();
        }

        /*Read response packet from target*/
        if(I2C_FAILURE ==  I2C_controller_read())
        {
            handle_app_error();
        }

        /* Check packet structure and status */
        if ((PACKET_SOP   == buffer[PACKET_RPLY_SOP_POS]) && 
            (PACKET_EOP   == buffer[PACKET_RPLY_EOP_POS]) && 
            (STS_CMD_DONE == buffer[PACKET_RPLY_STS_POS]))
        {
            /* Next command to be written. */
            cmd = (CYBSP_LED_STATE_ON == cmd) ? CYBSP_LED_STATE_OFF : 
                   CYBSP_LED_STATE_ON;
        }
        else
        {
            handle_app_error();
        }

        /* Give delay between commands. */
        Cy_SysLib_Delay(CMD_TO_CMD_DELAY);
#endif
    }
}


/* [] END OF FILE */