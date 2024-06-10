#include <linux/init.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/delay.h> 

#define DRIVER_NAME "BH1750_driverH"
#define CLASS_NAME "BH1750H"
#define DEVICE_NAME "BH1750H"

#define Power_Down 0x00
#define Power_On 0x01
#define Reset 0x07

#define Continuously_H_Resolution_Mode1 0x10
#define One_Time_H_Resolution_Mode1 0x20
#define Continuously_H_Resolution_Mode2 0x11
#define One_Time_H_Resolution_Mode2 0x21

#define Change_Measurement_Time 0x44

#define IOCTL_SET_POWER_ON _IOW('k', 1, int)
#define IOCTL_SET_POWER_OFF _IOW('k', 2, int)
#define IOCTL_READ_Continuously_Mode1 _IOR('k', 3, int)
#define IOCTL_READ_One_Mode1 _IOR('k', 4, int)
#define IOCTL_READ_Continuously_Mode2 _IOR('k', 5, int)
#define IOCTL_READ_One_Mode2 _IOR('k', 6, int)
#define IOCTL_RESET _IOW('k', 7, int)
#define IOCTL_CHANGE_Measurement_Time _IOW('k', 8, int)

static struct i2c_client *BH1750_client;
static struct class *BH1750_class = NULL;
static struct device *BH1750_device = NULL;
static int major_number;

static int Change_Time(struct i2c_client *client, int value)
{  
    uint8_t temp1 = 0xE0 & value;
    uint8_t temp2 = 0x1F & value;
    temp1 |= 0x40;
    temp2 |= 0x60; 
    if (i2c_smbus_write_byte(client, temp1) < 0) {
        printk(KERN_ERR "Failed to change measurement time\n");
        return -EIO;
    } 
    if (i2c_smbus_write_byte(client, temp2) < 0) {
        printk(KERN_ERR "Failed to change measurement time\n");
        return -EIO;
    } 

    printk("Succeed in changing measurement time\n");

    return 0;
}

static int BH1750_Power_Opr(struct i2c_client *client, int value)
{
    if (i2c_smbus_write_byte(client, value) < 0) {
        if(value == Power_Down)
            printk(KERN_ERR "Failed to set power down\n");
        if(value == 0x01)
            printk(KERN_ERR "Failed to set power on\n"); 
        if(value == 0x07)
            printk(KERN_ERR "Failed to set reset\n");       
    }

    printk("Succeed in setting operation mode\n");

    return 0;
}

static int BH1750_Read_Continuously_Mode1(struct i2c_client *client, int *value)
{
    u8 buf[2];

    if (i2c_smbus_read_i2c_block_data(client, Continuously_H_Resolution_Mode1, sizeof(buf), buf) < 0) {
        printk(KERN_ERR "Failed to Read data from Continuously Mode1\n");
        return -EIO;
    }

    *value = (buf[0] << 8) | buf[1];

    printk("Succeed in Reading data from Continuously Mode1\n");

    return 0;
}

static int BH1750_Read_One_Mode1(struct i2c_client *client, int *value)
{
    u8 buf[2];

    if (i2c_smbus_read_i2c_block_data(client, One_Time_H_Resolution_Mode1, sizeof(buf), buf) < 0) {
        printk(KERN_ERR "Failed to read data from one Mode 1 \n");
        return -EIO;
    }

    *value = (buf[0] << 8) | buf[1];
    
    i2c_smbus_write_byte(client, Power_Down);

    printk("Succeed in Reading data from One Mode1\n");

    return 0;
}

static int BH1750_Read_Continuously_Mode2(struct i2c_client *client, int *value)
{
    u8 buf[2];

    if (i2c_smbus_read_i2c_block_data(client, Continuously_H_Resolution_Mode2, sizeof(buf), buf) < 0) {
        printk(KERN_ERR "Failed to read data from Continuously Mode 2 \n");
        return -EIO;
    }

    *value = (buf[0] << 8) | buf[1];

    printk("Succeed in Reading data from Continuously Mode 2 \n");

    return 0;
}

static int BH1750_Read_One_Mode2(struct i2c_client *client, int *value)
{
    u8 buf[2];

    if (i2c_smbus_read_i2c_block_data(client, One_Time_H_Resolution_Mode1, sizeof(buf), buf) < 0) {
        printk(KERN_ERR "Failed to read One Mode 2 data\n");
        return -EIO;
    }

    *value = (buf[0] << 8) | buf[1];
    
    i2c_smbus_write_byte(client, Power_Down);

    printk("Succeed in Reading data from One Mode 2 data\n");

    return 0;
}

static long BH1750_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    msleep(250); 
    int data;

    switch (cmd) {
        case IOCTL_CHANGE_Measurement_Time:
            if (copy_from_user(&data, (int *)arg, sizeof(int)) != 0) {
                printk(KERN_ERR "Failed to copy value from user space\n");
                return -EFAULT;
            }
            Change_Time(BH1750_client, data);
            break;
        case IOCTL_SET_POWER_ON:
            BH1750_Power_Opr(BH1750_client, Power_On);
            break;
        case IOCTL_SET_POWER_OFF:
            BH1750_Power_Opr(BH1750_client, Power_Down);
            break;
        case IOCTL_RESET:
            BH1750_Power_Opr(BH1750_client, Reset);
            break; 
        case IOCTL_READ_Continuously_Mode1:
            BH1750_Read_Continuously_Mode1(BH1750_client, &data);
            break;      
        case IOCTL_READ_One_Mode1:
            BH1750_Read_One_Mode1(BH1750_client, &data);
            break;  
        case IOCTL_READ_Continuously_Mode2:
            BH1750_Read_Continuously_Mode2(BH1750_client, &data);
            break;  
        case IOCTL_READ_One_Mode2:
            BH1750_Read_One_Mode2(BH1750_client, &data);
            break;  
        default:
            return -EINVAL;
    }

    if (data > 0){
        if (copy_to_user((int __user *)arg, &data, sizeof(data)))
            return -EFAULT;
    }
    
    return 0;
}

static int BH1750_open(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "BH1750 device opened\n");
    return 0;
}

static int BH1750_release(struct inode *inodep, struct file *filep)
{
    printk(KERN_INFO "BH1750 device closed\n");
    return 0;
}

static struct file_operations fops = {
    .open = BH1750_open,
    .unlocked_ioctl = BH1750_ioctl,
    .release = BH1750_release,
};

static int BH1750_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
    BH1750_client = client;

    //on device
    i2c_smbus_write_byte(client, Power_On);

    // Create a char device
    major_number = register_chrdev(0, DEVICE_NAME, &fops);
    if (major_number < 0) {
        printk(KERN_ERR "Failed to register a major number\n");
        return major_number;
    }

    BH1750_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(BH1750_class)) {
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ERR "Failed to register device class\n");
        return PTR_ERR(BH1750_class);
    }

    BH1750_device = device_create(BH1750_class, NULL, MKDEV(major_number, 0), NULL, DEVICE_NAME);
    if (IS_ERR(BH1750_device)) {
        class_destroy(BH1750_class);
        unregister_chrdev(major_number, DEVICE_NAME);
        printk(KERN_ERR "Failed to create the device\n");
        return PTR_ERR(BH1750_device);
    }

    printk(KERN_INFO "BH1750H driver installed\n");
    return 0;   
}

static void BH1750_remove(struct i2c_client *client)
{
    device_destroy(BH1750_class, MKDEV(major_number, 0));
    class_unregister(BH1750_class);
    class_destroy(BH1750_class);
    unregister_chrdev(major_number, DEVICE_NAME);

    printk(KERN_INFO "BH1750H driver removed\n");
}

static const struct of_device_id BH1750_of_match[] = {
    { .compatible = "trungduc,BH1750H", },
    { },
};
MODULE_DEVICE_TABLE(of, BH1750_of_match);

static struct i2c_driver BH1750_driver = {
    .driver = {
        .name   = DRIVER_NAME,
        .owner  = THIS_MODULE,
        .of_match_table = of_match_ptr(BH1750_of_match),
    },
    .probe      = BH1750_probe,
    .remove     = BH1750_remove,
};

static int __init BH1750_init(void)
{
    printk(KERN_INFO "Initializing BH1750H driver\n");
    return i2c_add_driver(&BH1750_driver);
}

static void __exit BH1750_exit(void)
{
    printk(KERN_INFO "Exiting BH1750H driver\n");
    i2c_del_driver(&BH1750_driver);
}

module_init(BH1750_init);
module_exit(BH1750_exit);

MODULE_AUTHOR("Trung Duc");
MODULE_DESCRIPTION("BH1750 I2C Client Driver with IOCTL Interface");
MODULE_LICENSE("GPL");