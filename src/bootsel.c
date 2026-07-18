#include "bootsel.h"

#include "FreeRTOS.h"
#include "task.h"

#if DEBUGPROBE_ENABLE_BOOTSEL_BAUD_KNOCK
#include "pico/bootrom.h"

#define BOOTSEL_REBOOT_DELAY_MS 50

static bool bootsel_requested;
#endif

void bootsel_task(void)
{
#if DEBUGPROBE_ENABLE_BOOTSEL_BAUD_KNOCK
    if (!bootsel_requested)
        return;

    bootsel_requested = false;
    vTaskDelay(pdMS_TO_TICKS(BOOTSEL_REBOOT_DELAY_MS));
    rom_reset_usb_boot(0, 0);
#endif
}

bool bootsel_handle_baudrate(uint32_t baudrate)
{
#if DEBUGPROBE_ENABLE_BOOTSEL_BAUD_KNOCK
    static bool knock_armed;
    static TickType_t knock_deadline;

    TickType_t now = xTaskGetTickCount();

    if (baudrate == DEBUGPROBE_BOOTSEL_KNOCK_ARM_BAUD) {
        knock_armed = true;
        knock_deadline = now + pdMS_TO_TICKS(DEBUGPROBE_BOOTSEL_KNOCK_TIMEOUT_MS);
        return true;
    }

    if (baudrate == DEBUGPROBE_BOOTSEL_KNOCK_TRIGGER_BAUD && knock_armed &&
        ((int32_t)(knock_deadline - now) >= 0)) {
        knock_armed = false;
        bootsel_requested = true;
        return true;
    }

    knock_armed = false;
#else
    (void)baudrate;
#endif

    return false;
}
