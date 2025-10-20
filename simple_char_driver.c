// Purpose: Minimal character driver with open/read/write/release

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Darhil Shah");
MODULE_DESCRIPTION("Simple character driver (educational)");

static dev_t device_number;          // holds major+minor
static struct cdev simple_cdev;      // character device structure
static char device_buffer[1024];     // in-kernel storage
static size_t buffer_size = 0;       // bytes currently valid in buffer

static int simple_open(struct inode *inode, struct file *file)
{
    pr_info("simple_char: open\n");
    return 0;
}

static int simple_release(struct inode *inode, struct file *file)
{
    pr_info("simple_char: release\n");
    return 0;
}

static ssize_t simple_read(struct file *file, char __user *user_buf, size_t len, loff_t *ppos)
{
    size_t bytes_available;
    size_t bytes_to_read;

    if (*ppos >= buffer_size)
        return 0; // EOF

    bytes_available = buffer_size - *ppos;
    bytes_to_read = (len < bytes_available) ? len : bytes_available;

    if (copy_to_user(user_buf, device_buffer + *ppos, bytes_to_read))
        return -EFAULT;

    *ppos += bytes_to_read;
    pr_info("simple_char: read %zu bytes\n", bytes_to_read);
    return bytes_to_read;
}

static ssize_t simple_write(struct file *file, const char __user *user_buf, size_t len, loff_t *ppos)
{
    size_t space_left;
    size_t bytes_to_write;

    if (*ppos >= sizeof(device_buffer))
        return -ENOSPC;

    space_left = sizeof(device_buffer) - *ppos;
    bytes_to_write = (len < space_left) ? len : space_left;

    if (copy_from_user(device_buffer + *ppos, user_buf, bytes_to_write))
        return -EFAULT;

    *ppos += bytes_to_write;
    if (buffer_size < *ppos)
        buffer_size = *ppos;

    pr_info("simple_char: wrote %zu bytes\n", bytes_to_write);
    return bytes_to_write;
}

static const struct file_operations simple_fops = {
    .owner = THIS_MODULE,
    .open = simple_open,
    .release = simple_release,
    .read = simple_read,
    .write = simple_write,
};

static int __init simple_init(void)
{
    int ret;

    ret = alloc_chrdev_region(&device_number, 0, 1, "simple_char");
    if (ret) {
        pr_err("simple_char: alloc_chrdev_region failed (%d)\n", ret);
        return ret;
    }

    cdev_init(&simple_cdev, &simple_fops);
    simple_cdev.owner = THIS_MODULE;

    ret = cdev_add(&simple_cdev, device_number, 1);
    if (ret) {
        pr_err("simple_char: cdev_add failed (%d)\n", ret);
        unregister_chrdev_region(device_number, 1);
        return ret;
    }

    pr_info("simple_char: loaded. major=%d minor=%d\n", MAJOR(device_number), MINOR(device_number));
    pr_info("simple_char: create node with: mknod /dev/simple_char c %d %d\n", MAJOR(device_number), MINOR(device_number));
    return 0;
}

static void __exit simple_exit(void)
{
    cdev_del(&simple_cdev);
    unregister_chrdev_region(device_number, 1);
    pr_info("simple_char: unloaded\n");
}

module_init(simple_init);
module_exit(simple_exit);
