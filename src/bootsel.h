#ifndef BOOTSEL_H
#define BOOTSEL_H

#include <stdint.h>
#include <stdbool.h>

#ifndef DEBUGPROBE_ENABLE_BOOTSEL_BAUD_KNOCK
#define DEBUGPROBE_ENABLE_BOOTSEL_BAUD_KNOCK 0
#endif

#ifndef DEBUGPROBE_ENABLE_BOOTSEL_RESET_INTERFACE
#define DEBUGPROBE_ENABLE_BOOTSEL_RESET_INTERFACE 0
#endif

#ifndef DEBUGPROBE_BOOTSEL_KNOCK_ARM_BAUD
#define DEBUGPROBE_BOOTSEL_KNOCK_ARM_BAUD 9729
#endif

#ifndef DEBUGPROBE_BOOTSEL_KNOCK_TRIGGER_BAUD
#define DEBUGPROBE_BOOTSEL_KNOCK_TRIGGER_BAUD 9727
#endif

#ifndef DEBUGPROBE_BOOTSEL_KNOCK_TIMEOUT_MS
#define DEBUGPROBE_BOOTSEL_KNOCK_TIMEOUT_MS 1000
#endif

#if DEBUGPROBE_ENABLE_BOOTSEL_RESET_INTERFACE
#include "tusb.h"
#include "device/usbd_pvt.h"
#endif

#define DEBUGPROBE_RESET_INTERFACE_SUBCLASS 0x00
#define DEBUGPROBE_RESET_INTERFACE_PROTOCOL 0x01
#define DEBUGPROBE_RESET_REQUEST_BOOTSEL 0x01
#define DEBUGPROBE_RESET_REQUEST_FLASH 0x02

void bootsel_task(void);
void bootsel_request(uint32_t disable_interface_mask);
bool bootsel_handle_baudrate(uint32_t baudrate);

#if DEBUGPROBE_ENABLE_BOOTSEL_RESET_INTERFACE
extern usbd_class_driver_t const bootsel_reset_driver;
#endif

#endif
