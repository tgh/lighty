/*
 * COPYRIGHT (C) 2009 Sean Bruno and Tyler Hayes, Portland State University
 *
 * lighty.h
 */


//----------------------------------------------------------------------------

/*
 * MAJOR/MINOR NUMBERS
 *
 * device major number (0 means it will be dynamically allocated by kernel
 * in the init function).  A zero for the minor number is common practice,
 * but not required.  It is the starting number for devices associated with
 * the driver when allowing multiple devices to use the driver concurrently,
 * i.e. lighty0, lighty1, lighty2, etc.
 */
#define LIGHTY_MAJOR 0
#define LIGHTY_MINOR 0

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
#define SSTORE_IOCTL_MAX XXX

//----------------------------------------------------------------------------

/*
 * MISC. DEFINITIONS
 */

//the number of devices that can be associated with this driver
const int LIGHTY_DEVICE_COUNT = 1;

//----------------------------------------------------------------------------

/*
 * STRUCT DEFINITIONS
 */

//the device structure
struct lighty {
    /*
     * fd_count keeps track of how many open file descriptors in user space are
     * associated with the device represented by an instance of this struct.
     * This is done so that the release function in the driver can shut down
     * the device on the last close. (See "Linux Device Drivers" 3rd Ed. pg. 59)
     */
    unsigned int fd_count;
    /*
     * this is the head of the wait queue. wait_queue_head_t is a typedef for
     * struct __wait_queue_head (see linux/wait.h).  This queue is used in the
     * read() function of sstore.c.
     */
    wait_queue_head_t wait_queue;
    struct semaphore mutex;     //semaphore for mutal exclusion
    struct cdev cdev;
};
