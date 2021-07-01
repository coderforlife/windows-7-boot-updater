#!/bin/bash

# NOTE: this script has many good parts in it but is woefully incomplete

DIR="$(dirname $(readlink -f $0))"

echo "This will run the tests using Win7BootUpdaterCmd2.exe"

source "$DIR/funcs.sh"
ensure_running

EXE="$SHR\\Win7BootUpdaterCmd2.exe"
TEST_NAME=test-new.bs7
TEST="$SHR\\Tests\\$TEST_NAME"

restore() {
	local COUNTER=0
	until (VBoxManage guestcontrol "$NAME" run --username "Administrator" "$EXE" "/restore" | \
		   grep -q "No files were restored") && [ $COUNTER -lt 10 ]; do
		COUNTER=$((COUNTER + 1))
		sleep 0.001
	done
	if [ $COUNTER -ge 10 ]; then
		>&2 echo -e "\e[91mProblem restoring backup files: \e[0m"
		VBoxManage guestcontrol "$NAME" run --username "Administrator" "$EXE" "/restore"
		exit 1
	fi
}

restore

# TODO: call reset permissions? restore_full?

TMP=$(mktemp)
OUT=$(VBoxManage guestcontrol "$NAME" run --username "Administrator" "$EXE" "$TEST" 2>"$TMP")
RETVAL=$?
ERR=$(cat "$TMP")
rm "$TMP"

if [ $RETVAL -ne 0 ] || [ -n "$ERR" ] || [[ "$OUT" != *Successfully* ]]; then
    echo -e "\e[91mFailed to apply $TEST_NAME. Error code: $RETVAL\e[0m"
	echo "Output:"
	echo "$OUT" | sed 's/^/    /'
	echo "Error:"
	echo "$ERR" | sed 's/^/    /'
	exit 1
fi
VBoxManage guestcontrol "$NAME" run --username "Administrator" "C:\\Windows\System32\\shutdown.exe" "/r" "/t" "0"
cd "$IMG_DIR"
# TODO: make image, check tolerance, faster checks (right now one sec apart)?
# TODO: what if it never comes? check for "ready.png" in tandem or even the corrupt notification
wait_for_img_match "test1.png" 20000
cd -

# TODO: hibernation test

# TODO: test-bg-new, restart and hibernation

possible_shutdown
