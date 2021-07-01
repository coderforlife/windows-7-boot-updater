#!/bin/bash

DIR="$(dirname $(readlink -f $0))"

echo "This will remove and delete the hard drive for a test VM"

source "$DIR/funcs.sh"

##### Shutdown the machine if it is running #####
shutdown

##### Destroy the old hard drive #####
cd "$VM_DIR"
VBoxManage showvminfo "$NAME" --machinereadable | grep SATA-0-0 >/dev/null # check if the old drive is still there
RETVAL=$?
if [ $RETVAL -eq 0 ]; then
    VBoxManage storageattach "$NAME" --storagectl SATA --port 0 --medium none # detattch it from VM
    VBoxManage closemedium disk "$NAME.vdi" --delete # remove it from virtual medium manager and delete it
elif [ -f "$NAME.vdi" ]; then rm -f "$NAME.vdi"; fi # delete the file if it exists
