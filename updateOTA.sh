#! /bin/bash
# set -x
PATH_TO_TOOLS="/Users/tinmarbook/Library/Arduino15/packages/esp32/hardware/esp32/2.0.16/tools"

function upload() {
    python3 "$PATH_TO_TOOLS/espota.py" --timeout 10 --progress -i "$1" -p 3232 --auth= -f build/pedal.ino.bin
}

# ping pedal.local
# upload pedal.local
# upload 10.0.0.20
upload 192.168.138.160


#"/Users/tinmarbook/Library/Arduino15/packages/esp32/tools/esptool_py/4.5.1/esptool" --chip esp32s3 --port "/dev/tty.usbmodem1101" --baud 921600  --before default_reset --after hard_reset write_flash  -z --flash_mode dio --flash_freq 80m --flash_size 4MB 0x0 "/Users/tinmarbook/Work/boumboumchar/pedal/build/pedal.ino.bootloader.bin" 0x8000 "/Users/tinmarbook/Work/boumboumchar/pedal/build/pedal.ino.partitions.bin" 0xe000 "/Users/tinmarbook/Library/Arduino15/packages/esp32/hardware/esp32/2.0.16/tools/partitions/boot_app0.bin" 0x10000 "/Users/tinmarbook/Work/boumboumchar/pedal/build/pedal.ino.bin" 
