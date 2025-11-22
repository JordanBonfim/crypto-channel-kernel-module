/*   
 * MODULO DE CANAL CRIPTOGRAFICO - CRYPTOCHANNEL
 */
#include <linux/fs.h> // Necessário para a estrutura file_operations
#include <linux/module.h> // Needed by all modules
#include <linux/printk.h> // Needed for pr_info()
#include <linux/device.h> // Para criar a classe e o dispositivo no sistema (o que faz o arquivo aparecer no /dev).
#include <linux/uaccess.h> //Para futuramente mover dados entre o usuário e o kernel (segurança).
static ssize_t device_read(struct file *file, char __user *buf, size_t count, loff_t *offset){
    // Implementar leitura do dispositivo
    pr_info("CRYPTOCHANNEL: READ OPERATION\n");
    return 0;
}

static ssize_t device_write(struct file *file, const char __user *buf, size_t count, loff_t *offset){
    // Implementar escrita no dispositivo
    pr_info("CRYPTOCHANNEL: WRITE OPERATION\n");
    return count;
}

static int device_open(struct inode *inode, struct file *file){
    pr_info("CRYPTOCHANNEL: DEVICE OPENED\n");
    return 0;
}

static int device_release(struct inode *inode, struct file *file){
    pr_info("CRYPTOCHANNEL: DEVICE CLOSED\n");
    return 0;
}   

struct file_operations fops = {

    .read = device_read,
    .write = device_write,

    .open = device_open,
    .release = device_release
};

  
int init_module(void)  
{  
    pr_info("CRYPTOCHANNEL: STARTED\n");  

 
    /* A nonzero return means init_module failed; module can't be loaded. */
 
    return 0;  
}  
  
void cleanup_module(void)  
{  
    pr_info("CRYPTOCHANNEL: ENDED\n");  

}
 
 
MODULE_LICENSE("GPL");