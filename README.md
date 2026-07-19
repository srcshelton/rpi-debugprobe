# Debugprobe

Firmware source for the Raspberry Pi Debug Probe SWD/UART accessory. Can also be run on a Raspberry Pi Pico or Pico 2.

[Raspberry Pi Debug Probe product page](https://www.raspberrypi.com/products/debug-probe/)

[Raspberry Pi Pico product page](https://www.raspberrypi.com/products/raspberry-pi-pico/)

[Raspberry Pi Pico 2 product page](https://www.raspberrypi.com/products/raspberry-pi-pico-2/)

## Documentation

Debug Probe documentation can be found at the [Raspberry Pi documentation](https://www.raspberrypi.com/documentation/microcontrollers/debug-probe.html#about-the-debug-probe) and in the [Getting Started with Pico PDF](https://pip.raspberrypi.com/documents/RP-008276-DS).

## Hacking

For the purpose of making changes or studying of the code, you may want to compile the code yourself.

Building Debug Probe requires Pico SDK 2.3.0 or newer.

First, clone the repository:
```bash
git clone https://github.com/raspberrypi/debugprobe
cd debugprobe
```

Initialize and update the submodules:
```bash
 git submodule update --init --recursive
```

Then create and switch to the build directory:
```bash
 mkdir build
 cd build
```

If your environment doesn't contain `PICO_SDK_PATH`, then either add it to your environment variables with `export PICO_SDK_PATH=/path/to/sdk` or add `-DPICO_SDK_PATH=/path/to/sdk` to the arguments to CMake below.

Run cmake and build the code:
```bash
 cmake ..
 make
```

Done! You should now have a `debugprobe.uf2` that you can upload to your Debug Probe via the UF2 bootloader.

## Building for the Pico 1

If you want to create the version that runs on the Pico, then you need to invoke `cmake` in the sequence above with the `DEBUG_ON_PICO=ON` option:
```bash
cmake -DDEBUG_ON_PICO=ON ..
```

This will build with the configuration for the Pico and call the output program `debugprobe_on_pico.uf2`, as opposed to `debugprobe.uf2` for the accessory hardware.

Note that if you first ran through the whole sequence to compile for the Debug Probe, then you don't need to start back at the top. You can just go back to the `cmake` step and start from there.

## Building for the Pico 2

If using an existing debugprobe clone:
- You must completely regenerate your build directory, or use a different one.
- You must also sync and update submodules.
- `PICO_SDK_PATH` must point to a version 2.3.0 or newer install.

```bash
git submodule sync
git submodule update --init --recursive
mkdir build-pico2
cd build-pico2
cmake -DDEBUG_ON_PICO=1 -DPICO_BOARD=pico2 ../
```

This will build with the configuration for the Pico 2 and call the output program `debugprobe_on_pico2.uf2`.

## Software BOOTSEL reset

Debug Probe exposes the Pico SDK-compatible USB reset interface, so a connected
probe can be rebooted into BOOTSEL mode without pressing its BOOTSEL button:

```bash
picotool reboot -f -u
```

An optional two-step baud-rate sequence on the CDC UART interface can provide an
alternative remote reset mechanism. It is disabled by default. Enable it with:

```bash
cmake -DDEBUGPROBE_ENABLE_BOOTSEL_BAUD_KNOCK=ON ..
```

The default sequence is `9729`, then `9727` within one second, with no other baud
rate selected in between.

pySerial supports non-standard baud rates on Linux. It is not normally installed
by default, and is commonly packaged as `python3-serial`, `python3-pyserial`, or
`python-pyserial`:

```bash
python3 - <<'PY'
import time
import serial

with serial.Serial("/dev/ttyACM0", 9729) as probe:
    time.sleep(0.5)
    probe.baudrate = 9727
    time.sleep(0.1)
PY
```

GNU `stty` is normally part of a base Linux installation. It can issue the
default sequence on systems which support arbitrary baud rates:

```bash
stty -F /dev/ttyACM0 9729
sleep 0.5
stty -F /dev/ttyACM0 9727
```

However, versions shipped by many Linux distributions accept only standard
termios baud rates, so this command is not portable with the defaults.

For a sequence usable by those `stty` versions, configure the firmware with
standard rates. For example, use `200` followed by `50`:

```bash
cmake -DDEBUGPROBE_ENABLE_BOOTSEL_BAUD_KNOCK=ON \
      -DDEBUGPROBE_BOOTSEL_KNOCK_ARM_BAUD=200 \
      -DDEBUGPROBE_BOOTSEL_KNOCK_TRIGGER_BAUD=50 ..
```

The decreasing, non-adjacent sequence is less likely to be selected by a baud
rate scanner which tests standard rates in ascending order. Trigger it with:

```bash
stty -F /dev/ttyACM0 200
sleep 0.5
stty -F /dev/ttyACM0 50
```

The timeout can also be overridden at build time:

```bash
cmake -DDEBUGPROBE_ENABLE_BOOTSEL_BAUD_KNOCK=ON \
      -DDEBUGPROBE_BOOTSEL_KNOCK_TIMEOUT_MS=1500 ..
```

The two baud rates must be non-negative, distinct, and must not use the AutoBaud
control rate `9728`. This allows `0` to be used deliberately as an invalid-baud
knock value if the host's serial API can request it.

## AutoBaud Mode

Mode which automatically detects and sets the UART baud rate as data arrives.

To enable AutoBaud, configure the USB CDC port to the following custom baud rate:
```
9728 (0x2600)
```
> **Note:** Some Linux serial tools cannot set custom baud values. PuTTY on Windows and any terminal that supports arbitrary baud rates works.

Changing the baud rate to any other value disables AutoBaud.
