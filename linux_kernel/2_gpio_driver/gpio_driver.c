// gpio_driver.c
// date: 2024-05-21
// author: botsz
// description: A simple GPIO driver for Linux kernel.

// 1. GPIO port control: probed & removed
// 2. printk and KERN_ALERT in the probe and remove functions to print messages
// 3. device tree match table
// 5. register a platform driver, and implement the probe and remove functions

 
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h> // for struct platform_device
#include <linux/gpio/consumer.h>   // for gpio_desc
#include <linux/mod_devicetable.h> // for of_struct of_device_id

#include <linux/err.h>             // for IS_ERR and PTR_ERR

MODULE_LICENSE("GPL");

struct gpio_desc * led ;

// 1. GPIO port control: probed & removed ------------------------
static int my_gpio_probe( struct platform_device *pdev ) {
    
    printk(KERN_ALERT "gpio_driver initialized\n");

    // get the GPIO descriptor for the "led" GPIO pin
    // pdev->dev : assign the hardware
    // "led" : GPIO pin in the device tree
    // enum gpiod_flags : GPIO_OUT_HIGH , initialize the GPIO pin with HIGH state 
    led = gpiod_get( &(pdev->dev), "led", GPIOD_OUT_HIGH );

    // Check if the GPIO descriptor is valid
    if (IS_ERR(led)) { 
        printk(KERN_ALERT "Failed to get GPIO descriptor for led\n");
        return PTR_ERR(led); 
        // PTR_ERR : convert the pointer to a error code
        // in linux kernel, error pointers are used to indicate errors, 
        // and PTR_ERR is used to convert the error pointer to an error code. 
    }

    return 0;
}

static int my_gpio_remove( struct platform_device *pdev ) {

    printk(KERN_ALERT "gpio_driver exited\n"); 
    
    if (led) {
        gpiod_set_value(led, 0 );  // set the GPIO pin 0, turn off the LED
        gpiod_put(led); // release the GPIO pin
    }

    return 0;
}

// 3. device tree match table ------------------------------------
// struct of_device_id : list all the compatible and supported devices
static const struct of_device_id my_gpio_of_match[] = {
    { .compatible = "companyName,my_gpio_led", }, // compatible string, should match the compatible string in the device tree
    { } // empty, Sentinel node ,end of the table
};

// define the platform driver structure
static struct platform_driver my_gpio_driver = {
    .probe = my_gpio_probe,   // called when the driver is loaded
    .remove = my_gpio_remove, // called when the driver is removed
    .driver = {                // driver structure
        .name = "my_platform_gpio", // name of the driver, should match the compatible string in the device tree
        .of_match_table = my_gpio_of_match, // Link the device tree match table 
    },
};

// 4. register the platform driver ------------------------------
// automatically creates the module_init and module_exit
module_platform_driver( my_gpio_driver );
