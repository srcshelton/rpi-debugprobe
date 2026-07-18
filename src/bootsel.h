#ifndef BOOTSEL_H
#define BOOTSEL_H

#include <stdbool.h>
#include <stdint.h>

#ifndef DEBUGPROBE_ENABLE_BOOTSEL_BAUD_KNOCK
#define DEBUGPROBE_ENABLE_BOOTSEL_BAUD_KNOCK 0
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

void bootsel_task(void);
bool bootsel_handle_baudrate(uint32_t baudrate);

#endif
