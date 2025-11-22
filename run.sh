sudo rmmod cryptochannel
sudo insmod cryptochannel.ko
major=$(dmesg | tail -1 | awk '{print $NF}') # Pega o ultimo numero do log
sudo rm /dev/cryptochannel
sudo mknod /dev/cryptochannel c $major 0
sudo chmod 666 /dev/cryptochannel
sudo dmesg | tail -3