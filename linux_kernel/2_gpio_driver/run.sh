
exeFILE=gpio_driver

# 1. Load the module
sudo insmod build/${exeFILE}.ko

# 2. Check if the module is loaded and run the "proble" function
sudo dmesg | tail -n 10

# 3. Test: sending an "ON" signal to the virtual pin
echo "1" > /dev/my_gpio_dev

# 4. Check the logs again to see the driver's output response
dmesg | tail -n 5

# 5. Test: sending an "OFF" signal
echo "0" > /dev/my_gpio_dev
dmesg | tail -n 5

# 3. Unload the module
sudo rmmod ${exeFILE}