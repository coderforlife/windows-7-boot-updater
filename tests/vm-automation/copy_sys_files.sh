#!/bin/bash

DIR="$(dirname $(readlink -f $0))"

fmt <<EOF
This will copy the files winload.exe, winresume.exe, etc from a test VM (but not MUIs).
This will copy them to the current directory or a directory given as an argument
EOF

source "$DIR/funcs.sh"
ensure_running

OUT="$PWD"
if [ $# -ne 0 ]; then OUT="$1"; fi
OUT="$(readlink -f $OUT)"
SYS32="C:\\Windows\\System32"

##### Copy the files #####
# TODO: This command *should* work but there is a bug, see https://www.virtualbox.org/ticket/14336
#VBoxManage guestcontrol "$NAME" copyfrom --username "$USERNAME" --target-directory "$OUT" \
#    "$SYS32\\winload.exe" "$SYS32\\winresume.exe" "$SYS32\\bootres.dll" "$SYS32\\ntoskrnl.exe"
# For now the workaround is to do each copy seperately (and call them the target-directory)
for FILE in "winload.exe" "winresume.exe" "bootres.dll" "ntoskrnl.exe"; do
    VBoxManage guestcontrol "$NAME" copyfrom --username "$USERNAME" "$SYS32\\$FILE" --target-directory "$OUT/$FILE"
done

# Copy the bootmgr file
FILE="C:\\Users\\$USERNAME\\AppData\\Local\\Temp\\bootmgr"
VBoxManage guestcontrol "$NAME" run --username "$USERNAME" "$SHR\\Utilities\\copy_bootmgr.exe" "$FILE"
VBoxManage guestcontrol "$NAME" copyfrom --username "$USERNAME" "$FILE" --target-directory "$OUT/bootmgr"
VBoxManage guestcontrol "$NAME" rm --username "$USERNAME" "$FILE" # TODO: --force

possible_shutdown
