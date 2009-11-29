/*
 * COPYRIGHT (C) 2009 Sean Bruno and Tyler Hayes, Portland State University
 *
 * lighty.h
 */


//----------------------------------------------------------------------------

/*
 * MAJOR/MINOR NUMBERS
 *
 * USB has a reserved major number of 180, so we do not need to set up our own
 * major number, or have one setup dynamically like a char device would require.
 * Instead we just have a base number for minor numbers, which here is
 * arbitrary.  This is used in the usb_class_driver struct, which is used when
 * registering the device with a call to usb_register_dev() in the probe()
 * function.
 * All devices associated with this driver are created with unique, increasing
 * minor numbers beginning with this value.
 */
#define LIGHTY_MINOR_BASE 100

//----------------------------------------------------------------------------

/*
 * IOCTL DEFINITIONS
 *
 * 0xFF is chosen as the driver's "magic number" simply because it's not listed
 * as being used in the Documentaion/ioctl/ioctl-number.txt file.  (See
 * "Linux Device Drivers" 3rd Ed. pgs. 137-140 for more detail,
 * asm-generic/ioctl.h for macro definitions, and Documentation/ioctl/ioctl-
 * number.txt for magic numbers)
 */
#define LIGHTY_IOCTL_MAGIC 0xFF
XXX
/*
 * this max value is used in driver's ioctl() to test that user's command number
 * passed in is valid.  The number corresponds to the largest command number.
 * Each command is given a sequential number (using the _IO, IOR, _IOW, or _IOWR
 * macros) starting with 0.
 */
#define LIGHTY_IOCTL_MAX XXX

//----------------------------------------------------------------------------

/*
 * MISC. DEFINITIONS
 */

//Vendor and product ID's for the device
#define USB_LIGHTY_VENDOR_ID	0xfff0
#define USB_LIGHTY_PRODUCT_ID	0xfff0
//macro to get the lighty_device struct given a kref object
#define to_skel_dev(d) container_of(d, struct usb_skel, kref)

//----------------------------------------------------------------------------

/*
 * STRUCT DEFINITIONS
 */

//the device structure
struct usb_lighty {
	struct usb_device * udev;         /* the usb device for this device */
	struct usb_interface * interface; /* the interface for this device */
	unsigned char * bulk_in_buffer;   /* the buffer to receive data */
	size_t bulk_in_size;              /* the size of the receive buffer */
	__u8 bulk_in_endpointAddr;        /* the address of the bulk in endpoint */
	__u8 bulk_out_endpointAddr;       /* the address of the bulk out endpoint */
	struct kref refcount;             /* a reference count for this struct */
};
