#include "bootsel.h"

#include "FreeRTOS.h"
#include "task.h"

#if DEBUGPROBE_ENABLE_BOOTSEL_BAUD_KNOCK || DEBUGPROBE_ENABLE_BOOTSEL_RESET_INTERFACE
#include "pico/bootrom.h"
#define DEBUGPROBE_ENABLE_BOOTSEL_REBOOT 1
#else
#define DEBUGPROBE_ENABLE_BOOTSEL_REBOOT 0
#endif

#if DEBUGPROBE_ENABLE_BOOTSEL_RESET_INTERFACE
#include "hardware/watchdog.h"
#include "tusb.h"
#include "device/usbd_pvt.h"
#endif

#if DEBUGPROBE_ENABLE_BOOTSEL_REBOOT
#define BOOTSEL_REBOOT_DELAY_MS 50
#define BOOTSEL_INTERFACE_DISABLE_MASK 0x7fu

extern TaskHandle_t tud_taskhandle;

static volatile bool bootsel_requested;
static volatile uint32_t bootsel_disable_interface_mask;
#endif

void bootsel_request(uint32_t disable_interface_mask)
{
#if DEBUGPROBE_ENABLE_BOOTSEL_REBOOT
    bootsel_disable_interface_mask = disable_interface_mask & BOOTSEL_INTERFACE_DISABLE_MASK;
    bootsel_requested = true;

    if (tud_taskhandle)
        xTaskNotify(tud_taskhandle, 0, eNoAction);
#else
    (void)disable_interface_mask;
#endif
}

void bootsel_task(void)
{
#if DEBUGPROBE_ENABLE_BOOTSEL_REBOOT
    if (!bootsel_requested)
        return;

    uint32_t disable_interface_mask = bootsel_disable_interface_mask;

    bootsel_requested = false;
    vTaskDelay(pdMS_TO_TICKS(BOOTSEL_REBOOT_DELAY_MS));
    rom_reset_usb_boot(0, disable_interface_mask);
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
        bootsel_request(0);
        return true;
    }

    knock_armed = false;
#else
    (void)baudrate;
#endif

    return false;
}

#if DEBUGPROBE_ENABLE_BOOTSEL_RESET_INTERFACE
static uint8_t reset_itf_num;

static void resetd_init(void)
{
}

static bool resetd_deinit(void)
{
    return true;
}

static void resetd_reset(uint8_t __unused rhport)
{
    reset_itf_num = 0;
}

static uint16_t resetd_open(uint8_t __unused rhport,
                            tusb_desc_interface_t const *itf_desc,
                            uint16_t max_len)
{
    TU_VERIFY(TUSB_CLASS_VENDOR_SPECIFIC == itf_desc->bInterfaceClass &&
              DEBUGPROBE_RESET_INTERFACE_SUBCLASS == itf_desc->bInterfaceSubClass &&
              DEBUGPROBE_RESET_INTERFACE_PROTOCOL == itf_desc->bInterfaceProtocol, 0);

    uint16_t const drv_len = sizeof(tusb_desc_interface_t);

    TU_VERIFY(max_len >= drv_len, 0);
    reset_itf_num = itf_desc->bInterfaceNumber;

    return drv_len;
}

static bool resetd_control_xfer_cb(uint8_t rhport, uint8_t stage,
                                   tusb_control_request_t const *request)
{
    if (stage != CONTROL_STAGE_SETUP)
        return true;

    if (request->wIndex != reset_itf_num ||
        request->bmRequestType_bit.recipient != TUSB_REQ_RCPT_INTERFACE ||
        request->bmRequestType_bit.type != TUSB_REQ_TYPE_CLASS) {
        return false;
    }

    switch (request->bRequest) {
    case DEBUGPROBE_RESET_REQUEST_BOOTSEL:
        bootsel_request(request->wValue);
        return tud_control_status(rhport, request);
    case DEBUGPROBE_RESET_REQUEST_FLASH:
        watchdog_reboot(0, 0, 50);
        return tud_control_status(rhport, request);
    default:
        return false;
    }
}

static bool resetd_xfer_cb(uint8_t __unused rhport,
                           uint8_t __unused ep_addr,
                           xfer_result_t __unused result,
                           uint32_t __unused xferred_bytes)
{
    return true;
}

usbd_class_driver_t const bootsel_reset_driver = {
    .init = resetd_init,
    .deinit = resetd_deinit,
    .reset = resetd_reset,
    .open = resetd_open,
    .control_xfer_cb = resetd_control_xfer_cb,
    .xfer_cb = resetd_xfer_cb,
    .sof = NULL,
#if CFG_TUSB_DEBUG >= 2
    .name = "RESET"
#endif
};
#endif
