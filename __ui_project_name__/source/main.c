/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "FreeRTOS.h"
#include "task.h"

#include "fsl_debug_console.h"
#include "lvgl_support.h"
#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
#include "lvgl/lvgl.h"
#include "../ui/ui.h"

#include "fsl_gpio.h"
#include "display_support.h"
#include "lv_ex_conf.h"
#if LV_USE_GPU_NXP_VG_LITE == 1
#include "vglite_support.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
static volatile bool s_lvgl_initialized = false;

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

void print_cb(lv_log_level_t level, const char *file, uint32_t line, const char *func, const char *buf)
{
    /*Use only the file name not the path*/
    size_t p;
    for(p = strlen(file); p > 0; p--) {
	if(file[p] == '/' || file[p] == '\\') {
	    p++;    /*Skip the slash*/
	    break;
	}
    }

    static const char * lvl_prefix[] = {"Trace", "Info", "Warn", "Error", "User"};
    PRINTF("\r%s: %s \t(%s #%d %s())\n", lvl_prefix[level], buf, &file[p], line, func);
}

static void AppTask(void *param)
{
#if LV_USE_LOG
    lv_log_register_print_cb(print_cb);
#endif
    lv_port_pre_init();
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();

    s_lvgl_initialized = true;


    ui_init();
//    lv_img_set_pivot(ui_Image_Sec, 16, 156);
//    lv_img_set_pivot(ui_Image_Min, 9, 157);
//    lv_img_set_pivot(ui_Image_Hour, 9, 98);
//    lv_demo_widgets();
#if LV_USE_DEMO_WIDGETS != 0
#elif LV_USE_DEMO_BENCHMARK != 0
    lv_demo_benchmark();
#elif USE_DEMO_E_BIKE != 0
    ebike_demo_create();
#elif USE_DEMO_SMART_HOME != 0
    welecomScrOpen();
#elif USE_DEMO_MEDICAL != 0
    demo_create();
#elif USE_UNIT_TEST != 0
    my_demo();
#elif LV_USE_DEMO_MUSIC != 0
//    lv_demo_music();
#endif


//    lv_obj_set_flex_flow(lv_scr_act(), LV_FLEX_FLOW_COLUMN);
//    lv_obj_set_style_pad_row(lv_scr_act(), 0, 0);
//
//    lv_obj_t * rect = lv_obj_create(lv_scr_act());
//    lv_obj_remove_style_all(rect);
//    lv_obj_set_size(rect, lv_pct(100), lv_pct(25));
//    lv_obj_set_style_bg_color(rect, lv_color_hex3(0xf00), 0);
//    lv_obj_set_style_bg_opa(rect, LV_OPA_COVER, 0);
//
//    rect = lv_obj_create(lv_scr_act());
//    lv_obj_remove_style_all(rect);
//    lv_obj_set_size(rect, lv_pct(100), lv_pct(25));
//    lv_obj_set_style_bg_color(rect, lv_color_hex3(0x0f0), 0);
//    lv_obj_set_style_bg_opa(rect, LV_OPA_COVER, 0);
//
//    rect = lv_obj_create(lv_scr_act());
//    lv_obj_remove_style_all(rect);
//    lv_obj_set_size(rect, lv_pct(100), lv_pct(25));
//    lv_obj_set_style_bg_color(rect, lv_color_hex3(0x00f), 0);
//    lv_obj_set_style_bg_opa(rect, LV_OPA_COVER, 0);
//
//    rect = lv_obj_create(lv_scr_act());
//    lv_obj_remove_style_all(rect);
//    lv_obj_set_size(rect, lv_pct(100), lv_pct(25));
//    lv_obj_set_style_bg_color(rect, lv_color_hex3(0xfff), 0);
//    lv_obj_set_style_bg_grad_color(rect, lv_color_hex3(0x000), 0);
//    lv_obj_set_style_bg_grad_dir(rect, LV_GRAD_DIR_HOR, 0);
//    lv_obj_set_style_bg_opa(rect, LV_OPA_COVER, 0);


//    lv_demo_smartwatch_create();

    for (;;)
    {
        lv_task_handler();
        vTaskDelay(1);
    }
}

/*******************************************************************************
 * Code
 ******************************************************************************/
#if (DEMO_PANEL_RM67162 == DEMO_PANEL)
void GPIO_INTA_IRQHandler(void)
{
    uint32_t intStat;

    intStat = GPIO_PortGetInterruptStatus(GPIO, BOARD_MIPI_TE_PORT, kGPIO_InterruptA);

    if (intStat & (1UL << BOARD_MIPI_TE_PIN))
    {
        GPIO_PortClearInterruptFlags(GPIO, BOARD_MIPI_TE_PORT, kGPIO_InterruptA, (1UL << BOARD_MIPI_TE_PIN));
        BOARD_DisplayTEPinHandler();
    }

    intStat = GPIO_PortGetInterruptStatus(GPIO, BOARD_MIPI_PANEL_TOUCH_INT_PORT, kGPIO_InterruptA);

    if (intStat & (1UL << BOARD_MIPI_PANEL_TOUCH_INT_PIN))
    {
        GPIO_PortClearInterruptFlags(GPIO, BOARD_MIPI_PANEL_TOUCH_INT_PORT, kGPIO_InterruptA,
                                     (1UL << BOARD_MIPI_PANEL_TOUCH_INT_PIN));
        BOARD_TouchIntHandler();
    }
}
#endif

/*!
 * @brief Main function
 */
int main(void)
{
    BaseType_t stat;

    /* Init board hardware. */
    status_t status;

    BOARD_InitUARTPins();
    BOARD_InitPsRamPins();

#if (DEMO_PANEL_TFT_PROTO_5 == DEMO_PANEL)
    BOARD_InitFlexIOPanelPins();

    GPIO_PortInit(GPIO, BOARD_SSD1963_RST_PORT);
    GPIO_PortInit(GPIO, BOARD_SSD1963_CS_PORT);
    GPIO_PortInit(GPIO, BOARD_SSD1963_RS_PORT);
#else
    BOARD_InitMipiPanelPins();

    GPIO_PortInit(GPIO, BOARD_MIPI_POWER_PORT);
    GPIO_PortInit(GPIO, BOARD_MIPI_BL_PORT);
    GPIO_PortInit(GPIO, BOARD_MIPI_RST_PORT);
    GPIO_PortInit(GPIO, BOARD_MIPI_PANEL_TOUCH_RST_PORT);

#if (DEMO_PANEL_RM67162 == DEMO_PANEL)
    GPIO_PortInit(GPIO, BOARD_MIPI_TE_PORT);
    GPIO_PortInit(GPIO, BOARD_MIPI_PANEL_TOUCH_INT_PORT);
#endif

#endif

    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();

    PRINTF("\nLVGL DEMO RT500");
    LV_LOG_TRACE("\nTRACE enabled");
    LV_LOG_INFO("\nINFO enabled");
    LV_LOG_WARN("\nWARN enabled");
    LV_LOG_ERROR("\nERROR enabled");
    LV_LOG_USER("\nUSER enabled");

    status = BOARD_InitPsRam();
    if (status != kStatus_Success)
    {
        assert(false);
    }

#if ((DEMO_PANEL_RK055AHD091 == DEMO_PANEL) || (DEMO_PANEL_RK055IQH091 == DEMO_PANEL) || \
     (DEMO_PANEL_RM67162 == DEMO_PANEL))
    /* Use 48 MHz clock for the FLEXCOMM4 */
    CLOCK_AttachClk(kFRO_DIV4_to_FLEXCOMM4);

    BOARD_MIPIPanelTouch_I2C_Init();
#else
    /* Use 48 MHz clock for the FLEXCOMM4 */
    CLOCK_AttachClk(kFRO_DIV4_to_FLEXCOMM4);
#endif

#if LV_USE_GPU_NXP_VG_LITE == 1
    BOARD_PrepareVGLiteController();
#endif

    stat = xTaskCreate(AppTask, "lvgl", configMINIMAL_STACK_SIZE + 800, NULL, tskIDLE_PRIORITY + 2, NULL);

    if (pdPASS != stat)
    {
        PRINTF("Failed to create lvgl task");
        while (1)
            ;
    }

    vTaskStartScheduler();

    for (;;)
    {
    } /* should never get here */
}

/*!
 * @brief Malloc failed hook.
 */
void vApplicationMallocFailedHook(void)
{
    for (;;)
        ;
}

/*!
 * @brief FreeRTOS tick hook.
 */
void vApplicationTickHook(void)
{
    if (s_lvgl_initialized)
    {
        lv_tick_inc(1);
    }
}

/*!
 * @brief Stack overflow hook.
 */
void vApplicationStackOverflowHook(TaskHandle_t xTask, signed char *pcTaskName)
{
    (void)pcTaskName;
    (void)xTask;

    for (;;)
        ;
}
