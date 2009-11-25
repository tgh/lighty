/*
 * COPYRIGHT (C) 2009 Sean Bruno and Tyler Hayes, Portland State University
 *
 * lighty.c
 */

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>           /* for struct file, struct file_operations,
                                 * register_chrdev_region(),
                                 * alloc_chrdev_region() */
#include <linux/types.h>        /* for dev_t (represents device numbers),
                                 * ssize_t, size_t, loff_t types */
#include <linux/kdev_t.h>       /* for MKDEV(), MAJOR(), and MINOR() macros */
#include <linux/sched.h>        /* for current process info */
#include <linux/uaccess.h>      /* for copy_to_user() and copy_from_user() */
#include <linux/proc_fs.h>      /* for use of the /proc file system */
#include <linux/usb.h>          /* needed for usb device, obviously */
#include "lighty.h"             /* LIGHTY_MAJOR, LIGHTY_DEVICE_COUNT
                                   struct lighty, etc.*/

/*
 * Function prototypes
 */


/*
 * Global variables
 */
//major and minor numbers
unsigned int lighty_major = LIGHTY_MAJOR;
unsigned int lighty_minor = LIGHTY_MINOR;
//used for creating a /proc directory (used in init() and cleanup_and_exit())
struct proc_dir_entry * lighty;

/*
 * FOPS (file operations). This struct is a collection of function pointers
 * that point to a char driver's methods.
 */
struct file_operations lighty_fops = {
    .owner = THIS_MODULE,
    .ioctl = lighty_ioctl,
    .open = lighty_open,
    .release = lighty_release
};

//---------------------------------------------------------------------------

/*
 * INIT
 */
static int __init lighty_init(void) {

}

//---------------------------------------------------------------------------

/*
 * OPEN - this funciton is called when the device is opened in userspace (when
 * the "file" /dev/lighty0 or /dev/lighty1 is opened, for example)
 */

int lighty_open(struct inode * inode, struct file * filp) {

}

//---------------------------------------------------------------------------

/*
 * PROC: XXX
 */
int lighty_proc_XXX(char * page, char ** start, off_t offset, int count,
        int * eof, void * data) {

}

//---------------------------------------------------------------------------

/*
 * IOCTL
 */
int lighty_ioctl(struct inode * inode, struct file * filp, unsigned int command,
                                                        unsigned long arg) {

}

//---------------------------------------------------------------------------

/*
 * RELEASE
 */
int lighty_release(struct inode * inode, struct file * filp) {

}

//---------------------------------------------------------------------------

/*
 * EXIT
 */
static void lighty_cleanup_and_exit(void) {

}

//---------------------------------------------------------------------------


//tells kernel which functions run when driver is loaded/removed
module_init(lighty_init);
module_exit(lighty_cleanup_and_exit);

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Sean Bruno and Tyler Hayes");
