# Useful pages:
#   https://www.virtualbox.org/manual/ch08.html
#   https://www.win.tue.nl/~aeb/linux/kbd/scancodes-1.html

while [[ ! $ARCH =~ ^x(86|64)$ ]]; do read -p "Architecture (x86 or x64): " ARCH; done
NAME="Win 7 Test $ARCH"
USERNAME="test-$ARCH"
VM_DIR=~/"VirtualBox VMs"/"$NAME"/
DVD_DIR=~/jeff-backup/VMs/Windows/"Windows 7"/"Disc Creation"/
IMG_DIR=~/jeff-backup/W7BU/VM-auto
EXE_CMD=~/jeff-backup/W7BU/Win7BootUpdaterCmd2.exe
SHR="\\\\vboxsrv\\W7BU"

##### Define useful functions #####
wait_for_img_match() {
    # Waits for the VM to have an image that is quite similar to the one provided and threshold.
    # Polls the VM every second. Requires ImageMagick to be installed.
    # Note: the mouse pointer is ~300 pixels
    local IMG="$1" THRESH=$2 DIFF=786432 FIRST=1 RETVAL # 1024*768
    while [ $DIFF -gt $THRESH ]; do
        ! (( $FIRST )) && [[ $DIFF -ge $(( TRESH * 4 )) ]] && sleep 5 || sleep 1
        VBoxManage controlvm "$NAME" screenshotpng screen.png
        DIFF=$(compare -metric AE -fuzz 10% screen.png "$IMG" bmp:/dev/null 2>&1)
        RETVAL=$?
        if [ $RETVAL -ne 0 ] && [ $RETVAL -ne 1 ]; then DIFF=786432; fi
        echo Difference from $IMG is $DIFF pixels...
        rm -f screen.png
        FIRST=0
    done
}
get_status() {
    local STATUS=$(VBoxManage showvminfo "$NAME" --machinereadable | grep VMState=)
    STATUS="${STATUS//\"/}"
    echo "${STATUS:8}"
}
is_running() { if [ $(get_status) == "running" ]; then return 0; else return 1; fi; }
is_paused()  { if [ $(get_status) == "paused"  ]; then return 0; else return 1; fi; }
shutdown() {
    # Sends the Power Button signal, waits up to 30 secs for it to shutdown, then kills the machine
    if is_paused;  then VBoxManage controlvm "$NAME" resume; fi
    if is_running; then VBoxManage controlvm "$NAME" acpipowerbutton; fi
    local COUNTER=0
    while is_running && [ $COUNTER -lt 30 ]; do sleep 1; COUNTER=$((COUNTER + 1)); done
    if is_running; then
        >&2 echo -e "\e[91mForcing shutdown of VM\e[0m"
        VBoxManage controlvm "$NAME" poweroff
    fi
}
ensure_running() {
    # Resumes or starts the VM if it wasn't already running and waits for it to get to the desktop
    # Saves the variable "STARTED" so that possible_shutdown works
    STARTED="false"
    if is_paused; then VBoxManage controlvm "$NAME" resume; fi
    if ! is_running; then
        cd "$IMG_DIR"
        VBoxManage startvm "$NAME" --type gui # TODO: headless
        wait_for_img_match "ready.png" 500 # tolerance for time, date, and action center
        cd -
        STARTED="true"
    fi
}
possible_shutdown() {
    # Calls `shudtown` if STARTED is "true"
    if [ "$STARTED" == "true" ]; then shutdown; fi
}

# Typing on the VM
type_scancodes() {
    # Takes any number of arguments which are series of scancodes to type
    # Sends each group 10 ms apart
    for sc in "$@"; do
        VBoxManage controlvm "$NAME" keyboardputscancode $sc
        sleep 0.01
    done
}
type_text() {
    # Type a string of text on the VM
    # Handles all printable characters, uppercase/lowercase, space, tab, enter, and backspace
    TEXT="$1"
    for (( i=0; i < ${#TEXT}; i++ )); do
        type_scancodes "${scancodes_press[${TEXT:$i:1}]}" "${scancodes_release[${TEXT:$i:1}]}"
    done
    sleep 0.25
}

# Special keys not available with type_text and press/release
type_esc()      { type_scancodes "01" "81"; sleep 0.25; }
type_del()      { type_scancodes "e0 53" "e0 D3"; sleep 0.25; }
type_win()      { type_scancodes "e0 5B" "e0 DB"; sleep 0.25; }
press_ctrl()    { type_scancodes "1D"; }
release_ctrl()  { type_scancodes "9D"; sleep 0.25; }
press_shift()   { type_scancodes "2A"; }
release_shift() { type_scancodes "AA"; sleep 0.25; }
press_alt()     { type_scancodes "38"; }
release_alt()   { type_scancodes "B8"; sleep 0.25; }
press_win()     { type_scancodes "e0 5B"; }
release_win()   { type_scancodes "e0 DB"; sleep 0.25; }
type_down()     { type_scancodes "e0 50" "e0 D0"; sleep 0.25; }
type_up()       { type_scancodes "e0 48" "e0 C8"; sleep 0.25; }
type_left()     { type_scancodes "e0 4B" "e0 CB"; sleep 0.25; }
type_right()    { type_scancodes "e0 4D" "e0 CD"; sleep 0.25; }

# Can be sent using type_text but for one-shots these are easier
type_bcksp() { type_scancodes "0E" "8E"; sleep 0.25; }
type_enter() { type_scancodes "1C" "9C"; sleep 0.25; }
type_space() { type_scancodes "39" "B9"; sleep 0.25; }
type_tab()   { type_scancodes "0F" "8F"; sleep 0.25; }

# Common combinations
type_shift_tab()   { type_scancodes "2A 0F" "AA 8F"; sleep 0.25; }
type_alt_tab()     { type_scancodes "38 0F" "B8 8F"; sleep 0.25; }
type_alt_tab_tab() { type_scancodes "38 0F" "8F" "0F" "B8 8F"; sleep 0.25; }
type_ctrl_alt_del() { type_scancodes "1D 38 e0 53" "9D B8 e0 D3"; sleep 0.25; }
type_ctrl_shift_enter() { type_scancodes "1D 2A 1C" "9D AA 9C"; sleep 0.25; }
type_ctrl_x() {
    local CMD=$1
    type_scancodes "1D ${scancodes_press[$CMD]}" "9D ${scancodes_release[$CMD]}"
    sleep 0.25
}
type_alt_x() {
    local CMD=$1
    type_scancodes "38 ${scancodes_press[$CMD]}" "B8 ${scancodes_release[$CMD]}"
    sleep 0.25
}
type_ctrl_alt_x() {
    local CMD=$1
    type_scancodes "1D 38 ${scancodes_press[$CMD]}" "9D B8 ${scancodes_release[$CMD]}"
    sleep 0.25
}
type_win_x() {
    local CMD=$1
    type_scancodes "e0 5B ${scancodes_press[$CMD]}" "e0 DB ${scancodes_release[$CMD]}"
    sleep 0.25
}


# The scancode values used by type_text
declare -A scancodes_press=(
    ["1"]="02" ["2"]="03" ["3"]="04" ["4"]="05" ["5"]="06"
    ["6"]="07" ["7"]="08" ["8"]="09" ["9"]="0A" ["0"]="0B"
    ["q"]="10" ["w"]="11" ["e"]="12" ["r"]="13" ["t"]="14"
    ["y"]="15" ["u"]="16" ["i"]="17" ["o"]="18" ["p"]="19"
    ["a"]="1E" ["s"]="1F" ["d"]="20" ["f"]="21" ["g"]="22"
    ["h"]="23" ["j"]="24" ["k"]="25" ["l"]="26"
    ["z"]="2C" ["x"]="2D" ["c"]="2E" ["v"]="2F" ["b"]="30"
    ["n"]="31" ["m"]="32"
    ["-"]="0C" ["="]="0D" ["["]="1A" ["]"]="1B" [";"]="27"
    ["'"]="28" ['`']="29" ["\\"]="2B" [","]="33" ["."]="34"
    ["/"]="35" [" "]="39"
    [$'\b']="0E" [$'\t']="0F" [$'\n']="1C"
    # With shift
    ["!"]="2A 02" ["@"]="2A 03" ["#"]="2A 04" ["$"]="2A 05" ["%"]="2A 06"
    ["^"]="2A 07" ["&"]="2A 08" ["*"]="2A 09" ["("]="2A 0A" [")"]="2A 0B"
    ["Q"]="2A 10" ["W"]="2A 11" ["E"]="2A 12" ["R"]="2A 13" ["T"]="2A 14"
    ["Y"]="2A 15" ["U"]="2A 16" ["I"]="2A 17" ["O"]="2A 18" ["P"]="2A 19"
    ["A"]="2A 1E" ["S"]="2A 1F" ["D"]="2A 20" ["F"]="2A 21" ["G"]="2A 22"
    ["H"]="2A 23" ["J"]="2A 24" ["K"]="2A 25" ["L"]="2A 26"
    ["Z"]="2A 2C" ["X"]="2A 2D" ["C"]="2A 2E" ["V"]="2A 2F" ["B"]="2A 30"
    ["N"]="2A 31" ["M"]="2A 32"
    ["_"]="2A 0C" ["+"]="2A 0D" ["{"]="2A 1A" ["}"]="2A 1B" [":"]="2A 27"
    ['"']="2A 28" ["~"]="2A 29" ["|"]="2A 2B" ["<"]="2A 33" [">"]="2A 34" ["?"]="2A 35"
)

declare -A scancodes_release=(
    ["1"]="82" ["2"]="83" ["3"]="84" ["4"]="85" ["5"]="86"
    ["6"]="87" ["7"]="88" ["8"]="89" ["9"]="8A" ["0"]="8B"
    ["q"]="90" ["w"]="91" ["e"]="92" ["r"]="93" ["t"]="94"
    ["y"]="95" ["u"]="96" ["i"]="97" ["o"]="98" ["p"]="99"
    ["a"]="9E" ["s"]="9F" ["d"]="A0" ["f"]="A1" ["g"]="A2"
    ["h"]="A3" ["j"]="A4" ["k"]="A5" ["l"]="A6"
    ["z"]="AC" ["x"]="AD" ["c"]="AE" ["v"]="AF" ["b"]="B0"
    ["n"]="B1" ["m"]="B2"
    ["-"]="8C" ["="]="8D" ["["]="9A" ["]"]="9B" [";"]="A7"
    ["'"]="A8" ['`']="A9" ["\\"]="AB" [","]="B3" ["."]="B4"
    ["/"]="B5" [" "]="B9"
    [$'\b']="8E" [$'\t']="8F" [$'\n']="9C"
    # With shift
    ["!"]="AA 82" ["@"]="AA 83" ["#"]="AA 84" ["$"]="AA 85" ["%"]="AA 86"
    ["^"]="AA 87" ["&"]="AA 88" ["*"]="AA 89" ["("]="AA 8A" [")"]="AA 8B"
    ["Q"]="AA 90" ["W"]="AA 91" ["E"]="AA 92" ["R"]="AA 93" ["T"]="AA 94"
    ["Y"]="AA 95" ["U"]="AA 96" ["I"]="AA 97" ["O"]="AA 98" ["P"]="AA 99"
    ["A"]="AA 9E" ["S"]="AA 9F" ["D"]="AA A0" ["F"]="AA A1" ["G"]="AA A2"
    ["H"]="AA A3" ["J"]="AA A4" ["K"]="AA A5" ["L"]="AA A6"
    ["Z"]="AA AC" ["X"]="AA AD" ["C"]="AA AE" ["V"]="AA AF" ["B"]="AA B0"
    ["N"]="AA B1" ["M"]="AA B2"
    ["_"]="AA 8C" ["+"]="AA 8D" ["{"]="AA 9A" ["}"]="AA 9B" [":"]="AA A7"
    ['"']="AA A8" ["~"]="AA A9" ["|"]="AA AB" ["<"]="AA B3" [">"]="AA B4" ["?"]="AA B5"
)
