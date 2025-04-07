#! /bin/bash
# set -x
PATH_TO_TOOLS="$HOME/Library/Arduino15/packages/esp32/hardware/esp32/2.0.16/tools"

function upload() {
    python3 "$PATH_TO_TOOLS/espota.py" --timeout 10 --progress -i "$1" -p 3232 --auth= -f build/pedal.ino.bin
}

# ping pedal.local
upload pedal.local #10.0.0.20
