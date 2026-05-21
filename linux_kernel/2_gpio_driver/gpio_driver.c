#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/gpio/consumer.h>
#include <linux/platform_device.h>

/* Metadata */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Kernel Mentor");
MODULE_DESCRIPTION("A modern Linux GPIO Platform Driver");

/* Device properties */
static dev_t device_number;
static struct class *device_class;
static struct cdev gpio_cdev;
static struct gpio_desc *gpio_led;

#define DEVICE_NAME "my_gpio_dev"
#define CLASS_NAME "my_gpio_class"

/* File Operations: When user writes to /dev/my_gpio_dev */
static ssize_t driver_write(struct file *File, const char __user *user_buffer, size_t count, loff_t *offs) 
{
    char value;

    if (count == 0)
        return 0;

    if (copy_from_user(&value, user_buffer, 1)) {
        return -EFAULT;
    }

    if (value == '1') {
        gpiod_set_value(gpio_led, 1); /* Turn GPIO ON */
        pr_info("GPIO Driver: Set pin HIGH\n");
    } else if (value == '0') {
        gpiod_set_value(gpio_led, 0); /* Turn GPIO LOW */
        pr_info("GPIO Driver: Set pin LOW\n");
    } else {
        pr_warn("GPIO Driver: Invalid command. Use '1' or '0'\n");
    }

    return count;
}

static int driver_open(struct inode *device_file, struct file *instance) {
    pr_info("GPIO Driver: Device file opened\n");
    return 0;
}

static int driver_close(struct inode *device_file, struct file *instance) {
    pr_info("GPIO Driver: Device file closed\n");
    return 0;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = driver_open,
    .release = driver_close,
    .write = driver_write
};

/* Platform Probe: Runs when a matching device is found in Device Tree */
static int gpio_probe(struct platform_device *pdev)
{
    struct device *dev = &pdev->dev;
    int retval;

    pr_info("GPIO Driver: Probe function called!\n");

    /* Allocate Device Numbers */
    if (alloc_chrdev_region(&device_number, 0, 1, DEVICE_NAME) < 0) {
        pr_err("GPIO Driver: Failed to allocate major number\n");
        return -1;
    }

    /* Create Device Class */
    if ((device_class = class_create(CLASS_NAME)) == NULL) {
        pr_err("GPIO Driver: Failed to create device class\n");
        goto free_chrdev;
    }

    /* Create Device File */
    if (device_create(device_class, NULL, device_number, NULL, DEVICE_NAME) == NULL) {
        pr_err("GPIO Driver: Failed to create device file\n");
        goto free_class;
    }

    /* Initialize Character Device */
    cdev_init(&gpio_cdev, &fops);
    if (cdev_add(&gpio_cdev, device_number, 1) < 0) {
        pr_err("GPIO Driver: Failed to add cdev\n");
        goto free_device;
    }

    /* Request GPIO from Device Tree (labeled "gpios" or "led-gpios" in DT) */
    gpio_led = gpiod_get(dev, "led", GPIOD_OUT_LOW);
    if (IS_ERR(gpio_led)) {
        pr_err("GPIO Driver: Could not setup GPIO pin\n");
        retval = PTR_ERR(gpio_led);
        goto free_cdev;
    }

    return 0;

free_cdev:
    cdev_del(&gpio_cdev);
free_device:
    device_destroy(device_class, device_number);
free_class:
    class_destroy(device_class);
free_chrdev:
    unregister_chrdev_region(device_number, 1);
    return -1;
}

/* Platform Remove: Runs when driver is unloaded */
static int gpio_remove(struct platform_device *pdev)
{
    pr_info("GPIO Driver: Remove function called!\n");
    gpiod_put(gpio_led);
    cdev_del(&gpio_cdev);
    device_destroy(device_class, device_number);
    class_destroy(device_class);
    unregister_chrdev_region(device_number, 1);
    return 0;
}

/* Device Tree matching table */
static const struct of_device_id gpio_dt_ids[] = {
    { .compatible = "mentor,custom-gpio", },
    { /* sentinel */ }
};
MODULE_DEVICE_TABLE(of, gpio_dt_ids);

/* Platform Driver Structure */
static struct platform_driver my_gpio_driver = {
    .probe = gpio_probe,
    .remove = gpio_remove,
    .driver = {
        .name = "my_platform_gpio",
        .of_match_table = gpio_dt_ids,
    },
};

module_platform_driver(my_gpio_driver);