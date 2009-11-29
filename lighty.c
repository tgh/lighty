/*
 * COPYRIGHT (C) 2009 Sean Bruno and Tyler Hayes, Portland State University
 *
 * lighty.c
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/uaccess.h>      /* for copy_to_user() and copy_from_user() */
#include <linux/proc_fs.h>      /* for use of the /proc file system */
/*
 * The usb header file is needed for any usb driver, obviously.  The usb.h file
 * itself inludes the following:
 * - linux/mod_devicetable.h
 * - linux/usb/ch9.h
 * - linux/errno.h              (for -ENODEV)
 * - linux/delay.h              (for mdelay() )
 * - linux/interrupt.h          (for in_interrupt() )
 * - linux/list.h               (for struct list_head)
 * - linux/kref.h               (for struct kref)
 * - linux/device.h             (for struct device)
 * - linux/fs.h                 (for struct file_operations)
 * - linux/completion.h         (for struct completion)
 * - linux/sched.h              (for current && schedule_timeout)
 * - linux/mutex.h              (for struct mutex)
 *
 * The usb.h file itself defines just about all structs with the 'usb_' prefix
 * among other things.  linux/mod_devicetable.h defines struct usb_device_id.
 */
#include <linux/usb.h>
#include "lighty.h"             /* for struct lighty, vendor and product id's */

//---------------------------------------------------------------------------

/*
 * Function prototypes
 */
static int __init lighty_init(void);
static int lighty_probe(struct usb_interface *interface,
                                                const struct usb_device_id *id);
int lighty_open(struct inode *inode, struct file *filp);
int lighty_proc_XXX(char *page, char **start, off_t offset, int count,
                                                        int *eof, void *data);
int lighty_ioctl(struct usb_interface *intf, unsigned int code, void *buf);
void lighty_disconnect(struct usb_interface *intf);
int lighty_release(struct inode *inode, struct file *filp);
static void __exit lighty_cleanup_and_exit(void);

//---------------------------------------------------------------------------

/*
 * Global variables
 */
//used for creating a /proc directory (used in init() and cleanup_and_exit())
struct proc_dir_entry * lighty;

/*
 * This table conatins a list of all of the different kinds of USB devices this
 * driver can accept. If this is not set, the probe() function is never called.
 * Here, each usb_device_id struct (which is only one in this case--simulating a
 * driver that controls only a single device from a single vendor) is
 * initialized by the USB_DEVICE macro using the vendor and product ID's given
 * in the lighty.h file.  This is not the only macro you can use to initialize
 * the usb_device_id structs; see "Linux Device Drivers" 3rd. Ed. page 347. 
 */
static struct usb_device_id lighty_table [] = {
	{ USB_DEVICE(USB_LIGHTY_VENDOR_ID, USB_LIGHTY_PRODUCT_ID) },
	{ } //Terminating entry
};

/*
 * This is the main structure that all USB drivers must create.  It consists of
 * function pointers (i.e. probe, disconnect, etc), and variables that describe
 * the driver to the USB Core.
 * The name (which must be unique among all drivers in the kernel) will show up
 * in sysfs under /sys/bus/usb/drivers/ when the driver is loaded.
 */
static struct usb_driver lighty_driver = {
    .owner = THIS_MODULE,
    .name = "lighty",
    .id_table = lighty_table,
    .probe = lighty_probe,
    .disconnect = lighty_disconnect
    .ioctl = lighty_ioctl
};

/*
 * FOPS (file operations). This struct is a collection of function pointers
 * that point to a driver's methods.
 */
struct file_operations lighty_fops = {
    .owner = THIS_MODULE,
    .open = lighty_open,
    .release = lighty_release
};

/* 
 * usb class driver info in order to get a minor number from the usb core,
 * and to have the device registered with devfs and the driver core.  This is
 * used in the probe() function with a call to usb_register_dev().
 */
static struct usb_class_driver lighty_class = {
    .name = "usb/lighty%d",
    .fops = &skel_fops,
    .minor_base = USB_SKEL_MINOR_BASE
};

//---------------------------------------------------------------------------

/*
 * This macro uses the table that was just initialized and is necessary to allow
 * user-space tools to figure put what devices this driver can control.  Of
 * course, this macro is not restricted to USB, so be sure to use the string
 * 'usb' as the first argument.
 */
MODULE_DEVICE_TABLE (usb, lighty_table);

//---------------------------------------------------------------------------

/*
 * INIT
 */
static int __init lighty_init(void)
{
    int result;

    result = usb_register(&lighty_driver);
    if (result)
        err("usb_register failed.  Error number %d", result);

    return result;
}

//---------------------------------------------------------------------------

/*
 * PROBE
 * 
 * This function is called by the USB Core when it thinks it has a struct
 * usb_interface that this driver can handle.  A pointer to the struct
 * usb_device_id that the USB Core used to make this decision is also passed to
 * this function.  If the driver claims the interface that is passed to it, it
 * should initialize the device properly and return 0.  If the driver does not
 * want to claim the device (or an error occurs, of course), it should return a
 * negative error value. [taken from "Linux Device Drivers" 3rd. Ed. page 348]
 */
static int lighty_probe(struct usb_interface *interface,
                                                const struct usb_device_id *id)
{
    struct usb_lighty *dev = NULL;              //our device struct
    struct usb_host_interface *iface_desc;      //temp pointer
    struct usb_endpoint_descriptor *endpoint;   //temp pointer
    size_t buffer_size;     //size of the device's buffer for bulk transfers
    int i;
    int retval;

    //allocate memory for the device (struct usb_lighty)
    dev = kmalloc(sizeof(struct usb_lighty), GFP_KERNEL);
    if (dev == NULL)
        return -ENOMEM;
    //clean the memory allocated with zeroes
    memset(dev, 0x00, sizeof (*dev));
    //initialize the reference count of this device
    kref_init(&dev->refcount);

    //connect usb_device struct passed in by the core with this device struct
    dev->udev = usb_get_dev(interface_to_usbdev(interface));
    //connect the interface passed in by the usb core to this device struct
    dev->interface = interface;

    /*
     * traverse the array of endpoints that the currently active interface
     * of the device contains in order to find the appropriate endpoints for
     * this driver.  The active interface (a usb_host_interface struct)
     * consists of a struct usb_interface_descriptor ('desc', which we use to
     * see how many endpoints this interface has), and an array of struct
     * usb_host_endpoint ('endpoint').
     */
    //use temporary pointer to get device's current interface setting
    iface_desc = interface->cur_altsetting;
    //traverse the endpoint array
    for (i = 0; i < iface_desc->desc.bNumEndpoints; ++i) {
        /*
         * use temporary usb_endpoint_descriptor to point to current endpoint.
         * (a usb_host_endpoint struct contains a usb_endpoint_descriptor struct
         * ('desc'). 
         */
        endpoint = &iface_desc->endpoint[i].desc;

        /*
         * check for 3 things for each endpoint we're looking for:
         *   1. our device struct has not already found an appropriate endpoint
         *   2. the device endpoint's direction is correct
         *   3. the device endpoint is of correct type (i.e. bulk transfer, etc)
         * If all are true, the appropriate endpoint has been found.
         */
        //check for bulk IN endpoint
        if (!dev->bulk_in_endpointAddr &&
            (endpoint->bEndpointAddress & USB_DIR_IN) &&
            ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
                == USB_ENDPOINT_XFER_BULK)) {
            //grab the size of the device's buffer for bulk transfer
            buffer_size = endpoint->wMaxPacketSize;
            //set that size in our device struct
            dev->bulk_in_size = buffer_size;
            //set the address of the endpoint in our device struct
            dev->bulk_in_endpointAddr = endpoint->bEndpointAddress;
            //allocate space of buffer size for our device struct bulk IN buffer
            dev->bulk_in_buffer = kmalloc(buffer_size, GFP_KERNEL);
            //check for kmalloc error
            if (!dev->bulk_in_buffer) {
                err("Could not allocate bulk_in_buffer");
                //decrement our device struct reference count
                kref_put(&dev->refcount, skel_delete);
                return -ENOMEM;
            }
        }
        //check for bulk OUT endpoint
        if (!dev->bulk_out_endpointAddr &&
            !(endpoint->bEndpointAddress & USB_DIR_IN) &&
            ((endpoint->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
                == USB_ENDPOINT_XFER_BULK)) {
			//set the address of the endpoint in our device struct
            dev->bulk_out_endpointAddr = endpoint->bEndpointAddress;
        }
    }
    if (!(dev->bulk_in_endpointAddr && dev->bulk_out_endpointAddr)) {
        err("Could not find both bulk-in and bulk-out endpoints");
		//decrement our device struct reference count
        kref_put(&dev->refcount, skel_delete);
        //not sure what error value to return here...
        return -ENOMEM;
    }

	//save our data pointer in this interface device
    usb_set_intfdata(interface, dev);

	//now we can register the device
    retval = usb_register_dev(interface, &lighty_class);
    //check return value of registration
    if (retval) {
        err("Not able to get a minor for this device.");
        //reset our data pointer
        usb_set_intfdata(interface, NULL);
		//decrement our device struct reference count
        kref_put(&dev->refcount, skel_delete);
        //not sure what error value to return here...
        return -ENOMEM;
    }

    //let the user know what node this device is now attached to
    info("USB lighty device now attached to USBlighty-%d", interface->minor);
    //return success
    return 0;
}

//---------------------------------------------------------------------------

/*
 * OPEN - this funciton is called when the device is opened in userspace (when
 * the "file" /dev/lighty0 or /dev/lighty1 is opened, for example)
 */

int lighty_open(struct inode *inode, struct file *filp)
{

}

//---------------------------------------------------------------------------

/*
 * PROC: XXX
 */
int lighty_proc_XXX(char *page, char **start, off_t offset, int count,
        int *eof, void *data)
{

}

//---------------------------------------------------------------------------

/*
 * IOCTL
 */
int lighty_ioctl(struct usb_interface *intf, unsigned int code, void *buf)
{

}

//---------------------------------------------------------------------------

/*
 * DISCONNECT
 *
 * This function is called by the USB Core when the struct usb_interface has
 * been removed from the system or when the driver is being unloaded from the
 * USB Core. [taken from "Linux Device Drivers" 3rd. Ed. page 348]
 */

void lighty_disconnect(struct usb_interface *intf)
{

}

//---------------------------------------------------------------------------

/*
 * RELEASE
 */
int lighty_release(struct inode *inode, struct file *filp)
{

}

//---------------------------------------------------------------------------

/*
 * EXIT
 */
static void __exit lighty_cleanup_and_exit(void)
{
    /*
     * unregister the lighty_driver struct from the kernel.  When this call
     * happens, any USB interfaces that were currently bound to this driver are
     * disconnected, and lighty_disconnect() is called for them.
     * ["Linux Device Drivers" 3rd. Ed. page 349] 
     */
    usb_deregister(%lighty_driver);
}

//---------------------------------------------------------------------------


//tells kernel which functions run when driver is loaded/removed
module_init(lighty_init);
module_exit(lighty_cleanup_and_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Sean Bruno and Tyler Hayes");
