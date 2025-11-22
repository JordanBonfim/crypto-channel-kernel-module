/*   
 * MODULO DE CANAL CRIPTOGRAFICO - CRYPTOCHANNEL
 */
#include <linux/fs.h> // Necessário para a estrutura file_operations
#include <linux/module.h> // Needed by all modules
#include <linux/printk.h> // Needed for pr_info()
#include <linux/device.h> // Para criar a classe e o dispositivo no sistema (o que faz o arquivo aparecer no /dev).
#include <linux/uaccess.h> //Para futuramente mover dados entre o usuário e o kernel (segurança).
#include <linux/slab.h>  // Para kmalloc
#include <linux/mutex.h> // Para mutex

#define BUFFER_SIZE 1024
static char *kernel_buffer; // Buffer no kernel
static DEFINE_MUTEX(crypto_mutex); // Mutex para proteger o acesso ao buffer
static int data_length = 0; // Tamanho dos dados no buffer

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

// Variável GLOBAL para guardar o ID do driver
static int major_number;

int init_module(void)  
{  
    pr_info("CRYPTOCHANNEL: STARTED\n");  

    major_number = register_chrdev(0, "cryptochannel", &fops);
    if (major_number < 0) {
        pr_info("CRYPTOCHANNEL: FAILED TO REGISTER DEVICE\n");
        return major_number;
    }
    pr_info("CRYPTOCHANNEL: REGISTERED WITH MAJOR NUMBER %d\n", major_number);
    /* A nonzero return means init_module failed; module can't be loaded. */
 

    kernel_buffer = (BUFFER_SIZE, GFP_KERNEL); 

    // Verifica se a alocação funcionou 
    if (!kernel_buffer) {
        pr_info("CRYPTOCHANNEL: FAILED TO ALLOCATE MEMORY\n");
        
        // Se falhou memória, desfazer o registro do dispositivo antes de sair
        unregister_chrdev(major_number, "cryptochannel");
        
        return -ENOMEM; // Código de erro padrão para "Out of Memory"
    }

    // Limpar a memória com zeros
    memset(kernel_buffer, 0, BUFFER_SIZE);

    return 0;  
}  
  
void cleanup_module(void)  
{  
    // Liberar a memória alocada
    if(kernel_buffer){
        kfree(kernel_buffer);
    }
    pr_info("CRYPTOCHANNEL: ENDED\n");
    unregister_chrdev(major_number, "cryptochannel");
}
 
 
MODULE_LICENSE("GPL");