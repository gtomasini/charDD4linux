/*
 *
 * basic and essential serial linux device driver
 *
 *
 *
 *
 */
#include <linux/init.h>         
#include <linux/module.h> 
#include <linux/device.h>    
#include <linux/kernel.h>      
#include <linux/fs.h>     
#include <linux/uaccess.h>  

#define  DEVICE_NAME "basicChar"  ///< The device will appear at /dev/basicChar using this value
#define  CLASS_NAME  "xxx"        ///< The device class -- this is a character device driver

MODULE_LICENSE("GPL");            ///< The license type -- this affects available functionality
MODULE_AUTHOR("Anonimo");    ///< The author -- visible when you use modinfo
MODULE_DESCRIPTION("basic Linux char driver");  ///< The description -- see modinfo
MODULE_VERSION("0.2");            ///< A version number to inform users

static int    majorNum;            ///< Stores the device number -- determined automatically
static char   mydata[256] = {0};   ///< Memory for the string that is passed from userspace, would be dinamyc alloc
static short  mydata_len;         
static int    opensNum = 0;        ///< Counts the number of times the device is opened
static struct class*  basicCharClass  = NULL; ///< The device-driver class struct pointer
static struct device* basicCharDevice = NULL; ///< The device-driver device struct pointer

static int     open(struct inode *, struct file *);
static int     release(struct inode *, struct file *);
static ssize_t read(struct file *, char *, size_t, loff_t *);
static ssize_t write(struct file *, const char *, size_t, loff_t *);

static struct file_operations fops = {
   .open = open,
   .read = read,
   .write = write,
   .release = release,
};

static int __init basicChar_init(void){
   printk(KERN_INFO "basicChar: initializing...\n");

   // try to dynamically allocate a major number for the device 
   majorNum = register_chrdev(0, DEVICE_NAME, &fops);
   if (majorNum<0){
      printk(KERN_ALERT "basicChar failed to register a major number\n");
      return majorNum;
   }
   printk(KERN_INFO "basicChar: registered correctly with major number %d\n", majorNum);

   basicCharClass = class_create(THIS_MODULE, CLASS_NAME);
   if (IS_ERR(basicCharClass)){              
      printk(KERN_ALERT "Failed to register device class\n");
      goto ERROR0;
   }
   printk(KERN_INFO "basicChar: device class registered correctly\n");

   basicCharDevice = device_create(basicCharClass, NULL, MKDEV(majorNum, 0), NULL, DEVICE_NAME);
   if (IS_ERR(basicCharDevice)){              // Clean up if there is an error
      class_destroy(basicCharClass);           // Repeated code but the alternative is goto statements
      printk(KERN_ALERT "failed to create the device\n");
      goto ERROR0;
   }
   printk(KERN_INFO "basicChar: device class created correctly\n"); 
   return 0;

ERROR0:
      unregister_chrdev(majorNum, DEVICE_NAME);
      return PTR_ERR(basicCharClass);          // Correct way to return an error on a pointer
}

static void __exit basicChar_exit(void){
   device_destroy(basicCharClass, MKDEV(majorNum, 0));     // remove the device
   class_unregister(basicCharClass);                          // unregister the device class
   class_destroy(basicCharClass);                             // remove the device class
   unregister_chrdev(majorNum, DEVICE_NAME);             // unregister the major number
   printk(KERN_INFO "basicChar: bye\n");
}

static int open(struct inode *inodep, struct file *filep){
   opensNum++;
   printk(KERN_INFO "basicChar: opened device %d time(s)\n", opensNum);
   return 0;
}

static ssize_t read(struct file *filep, char *buffer, size_t len, loff_t *offset){
   int st;
   if (mydata_len>len){
	   mydata_len=len-1;
	   mydata[len-1]=0;
	 }
   st = copy_to_user(buffer, mydata, mydata_len);

   if (st){         
      printk(KERN_INFO "basicChar: failed to copy %d chars to the user\n", st);
      return -EFAULT;
   }
   
    printk(KERN_INFO "basicChar: %d chars copied to the user\n", mydata_len);
    return (mydata_len=0);  // clear the position to the start and return 0
}

static ssize_t write(struct file *filep, const char *buffer, size_t len, loff_t *offset){
   char aux[256];
   int st;
   if (len>255)
	   len=255;
   st=copy_from_user(aux, buffer, len);
   aux[255]=0;
   if (st){
      printk(KERN_INFO "basicChar: failed to receive %d chars from user\n", st);
      return -EFAULT;
   }
   sprintf(mydata, "%s(%zu chars)", aux, len);   
   mydata_len = strlen(mydata);                
   printk(KERN_INFO "basicChar: %zu chars copied from the user\n", len);
   return len;
}

static int release(struct inode *inodep, struct file *filep){
   printk(KERN_INFO "basicChar: device closed\n");
   return 0;
}

module_init(basicChar_init);
module_exit(basicChar_exit);
