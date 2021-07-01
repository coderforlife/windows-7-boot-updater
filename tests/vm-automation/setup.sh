#!/bin/bash

DIR="$(dirname $(readlink -f $0))"

echo "This will install Win 7 and the Guest Additions on a new hard drive for a test VM"

source "$DIR/funcs.sh"

##### Destroy the old hard drive, make a new one, and insert the installation DVD #####
cd "$VM_DIR"
VBoxManage showvminfo "$NAME" --machinereadable | grep SATA-0-0 >/dev/null # check if the old drive is still there
RETVAL=$?
if [ $RETVAL -eq 0 ]; then
    VBoxManage storageattach "$NAME" --storagectl SATA --port 0 --medium none # detattch it from VM
    VBoxManage closemedium disk "$NAME.vdi" --delete # remove it from virtual medium manager and delete it
elif [ -f "$NAME.vdi" ]; then rm -f "$NAME.vdi"; fi # delete the file if it exists
VBoxManage createmedium disk --filename "$NAME.vdi" --size 25600 # create new 25 GB dynamic hard drive
VBoxManage storageattach "$NAME" --storagectl SATA --port 0 --type hdd --medium "$NAME.vdi" # attach it to the VM
cd "$DVD_DIR" # mount the newest install DVD
DVD=`ls -t *$ARCH*.iso | head -n 1`
echo Installing using "$DVD"
VBoxManage storageattach "$NAME" --storagectl SATA --port 1 --medium "$DVD"

##### Start the VM and run through the various installation screens #####
VBoxManage startvm "$NAME" --type gui # TODO: headless
cd "$IMG_DIR"
sleep 5s # First boot takes some time
wait_for_img_match "install-1.png" 50
type_enter # Select language
type_enter # Install
wait_for_img_match "install-2.png" 7500 # tolerance for differences between x64 and x86
type_down; if [ "$ARCH" == "x86" ]; then type_down; fi; type_enter # Select "Home Premium" and Next
type_space; type_enter # Check that we accept terms and Next
type_down;  type_enter # Select "Custom (advanced)"
type_enter # Next
sleep 5m # Now a long waiting period while it installs...
wait_for_img_match "install-3.png" 5000
type_text "$USERNAME"; type_enter # Type the username and Next
type_enter; type_enter # Next, Next (no password, no product key)
type_down; type_down; type_enter # Select "Ask me later" for Windows Updates and Next
type_down; type_down; type_down; type_down; type_down; type_down
type_down; type_down; type_down; type_down; type_down; type_enter # Select "Eastern Time" and Next
type_up; type_up; type_enter # Select "Home network" and Next
sleep 30s # A waiting period (shorter and no restarts though)
wait_for_img_match "install-4.png" 1000 # tolerance for time and date

##### Install Guest Additions #####
VBoxManage storageattach "$NAME" --storagectl SATA --port 1 --medium additions
wait_for_img_match "vboxadd-install-1.png" 5000 # tolerance for time, date, and action center icon
type_enter # "Run VBoxWindowsAdditions.exe"
wait_for_img_match "vboxadd-install-2.png" 7500
type_left; type_enter # Select "Yes"
wait_for_img_match "vboxadd-install-3.png" 10000
type_alt_tab # Focus the installer window
type_enter; type_enter; type_enter # Next, Next, Next
wait_for_img_match "vboxadd-install-4.png" 50000 # tolerance for time, date, AC icon, and blinking taskbar
type_alt_tab_tab # Select Windows Security dialog
type_left; type_left; type_space # Check "Always trust software from 'Oracle Corp'"
type_right; type_enter # Install
wait_for_img_match "vboxadd-install-5.png" 10000
VBoxManage controlvm "$NAME" setvideomodehint 1024 768 32 # set hinted screen resolution
type_enter # Finish and Restart
sleep 10s # Restarts are not instantaneous
wait_for_img_match "ready.png" 5000

##### Adjust settings so guestcontrol works #####
type_win; type_text "cmd"; type_ctrl_shift_enter; sleep 2; type_left; type_enter; sleep 1 # Open Admin CMD line
type_text "net user administrator /active:yes"; type_enter # Enables administrator account
type_text "reg add \"HKLM\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" /v AutoAdminLogon /d 1 /f"; type_enter; sleep 0.2
type_text "reg add \"HKLM\\Software\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\" /v DefaultUserName /d \"$USERNAME\" /f"; type_enter; sleep 0.2
type_text "reg add \"HKLM\\System\\CurrentControlSet\\Control\\Lsa\" /v LimitBlankPasswordUse /t REG_DWORD /d 0 /f"; type_enter; sleep 0.2
type_text "exit"; type_enter; sleep 1

##### Setup Windows Updates #####
type_win; type_text "update"; type_enter # Open Windows Updates
sleep 10; wait_for_img_match "win-updates-open.png" 20000
type_down; sleep 0.2; type_enter; sleep 1 # Select "Let me choose my settings"
type_tab; type_tab; type_tab; type_tab; sleep 0.2 # Select the drop-down menu
type_down; sleep 0.2; type_down; sleep 0.2; type_down; sleep 0.2 # Select "Check for updates byt let me..." on the drop-down menu
type_tab; type_tab; type_tab; type_tab; type_enter # Select the "Ok" button and press it
sleep 1m # Takes quite a bit of time to search for updates...
wait_for_img_match "win-updates-ready.png" 25000
type_shift_tab; type_shift_tab; type_enter # Open the list of available important updates
