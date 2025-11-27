/*   
 * MODULO DE CANAL CRIPTOGRAFICO - CRYPTOCHANNEL
 */
#include <linux/fs.h> // Necessário para a estrutura file_operations
#include <linux/module.h> // Needed by all modules
#include <linux/printk.h> // Needed for pr_info()
#include <linux/device.h> // Para criar a classe e o dispositivo no sistema (o que faz o arquivo aparecer no /dev).
#include <linux/uaccess.h> // Para futuramente mover dados entre o usuário e o kernel (segurança).
#include <linux/slab.h>  // Para kmalloc
#include <linux/mutex.h> // Para mutex

#define BUFFER_SIZE 1024 
static char *kernel_buffer; // Buffer no kernel
static DEFINE_MUTEX(crypto_mutex); // Mutex para proteger o acesso ao buffer
static int write_pos = 0; // Posição de escrita no buffer
static int read_pos = 0; // Posição de leitura no buffer
static int stored_count = 0; // Quantidade de dados armazenados no buffer

static ssize_t device_read(struct file *file, char __user *buf, size_t count, loff_t *offset){
    char *temp_buf;
    size_t bytes_to_copy;

    pr_info("CRYPTOCHANNEL: READ OPERATION\n");

    mutex_lock(&crypto_mutex);

    if(stored_count == 0){
        mutex_unlock(&crypto_mutex);    
        return 0; // EOF
    }
    
    // Ajusta o count se for maior que os dados armazenados
    if (count > stored_count) {
        bytes_to_copy = stored_count;
    } else {
        bytes_to_copy = count;
    }

    // Aloca memória temporária para copiar os dados
    temp_buf = kmalloc(bytes_to_copy, GFP_KERNEL);
    if(!temp_buf){
        mutex_unlock(&crypto_mutex);
        pr_info("CRYPTOCHANNEL: MEMORY ALLOCATION FAILED\n");
        return -ENOMEM; // Not enough memory
    }

    // Copia os dados do buffer circular para o buffer temporário
    for (int i = 0; i < bytes_to_copy; i++) {
        // DECRIPITAÇÃO ENTRA AQUI

        temp_buf[i] = kernel_buffer[read_pos];
        read_pos = (read_pos + 1) % BUFFER_SIZE;
    }
    
    if(copy_to_user(buf, temp_buf, bytes_to_copy) != 0){
        kfree(temp_buf);
        mutex_unlock(&crypto_mutex);
        pr_info("CRYPTOCHANNEL: ERROR COPYING TO USER\n");
        return -EFAULT; //  Memory fault
    } 
    
    // Atualiza a contagem de dados armazenados
    stored_count -= bytes_to_copy;
    kfree(temp_buf);

    mutex_unlock(&crypto_mutex);    
    return bytes_to_copy;

}

static ssize_t device_write(struct file *file, const char __user *buf, size_t count, loff_t *offset){
    char *temp_buf;
    int free_space;

    // Implementar escrita no dispositivo
    pr_info("CRYPTOCHANNEL: WRITE OPERATION\n");

    mutex_lock(&crypto_mutex);

    free_space = BUFFER_SIZE - stored_count;

    // Verifica se há espaço suficiente no buffer
    if(free_space <= 0){
        mutex_unlock(&crypto_mutex);
        pr_info("CRYPTOCHANNEL: NO SPACE LEFT ON DEVICE\n");
        return -ENOSPC; // No space left on device
    }

    // Ajusta o count se for maior que o espaço disponível
    if(count > free_space){
        count = free_space; // Ajusta o count para o espaço disponível
        return -ENOMEM; // Not enough memory
    }

    // Aloca memória temporária para receber os dados do usuário
    temp_buf = kmalloc(count, GFP_KERNEL);
    if(!temp_buf){
        mutex_unlock(&crypto_mutex);
        pr_info("CRYPTOCHANNEL: MEMORY ALLOCATION FAILED\n");
        return -ENOMEM; // Not enough memory
    }

    if(copy_from_user(temp_buf, buf, count) != 0){
        kfree(temp_buf);
        mutex_unlock(&crypto_mutex);
        pr_info("CRYPTOCHANNEL: ERROR COPYING FROM USER\n");
        return -EFAULT; // Memory fault
    }

    // Escreve os dados no buffer circular
    for (int i = 0; i < count; i++) {
        // ENCRIPTAÇÃO ENTRA AQUI

        kernel_buffer[write_pos] = temp_buf[i];
        write_pos = (write_pos + 1) % BUFFER_SIZE;
    }
    
    stored_count += count;

    kfree(temp_buf);
    mutex_unlock(&crypto_mutex);


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

int init_module(void){  
    pr_info("CRYPTOCHANNEL: STARTED\n");  

    major_number = register_chrdev(0, "cryptochannel", &fops);
    if (major_number < 0) {
        pr_info("CRYPTOCHANNEL: FAILED TO REGISTER DEVICE\n");
        return major_number;
    }
    pr_info("CRYPTOCHANNEL: REGISTERED WITH MAJOR NUMBER %d\n", major_number);
    /* A nonzero return means init_module failed; module can't be loaded. */
 

    kernel_buffer = kmalloc(BUFFER_SIZE, GFP_KERNEL); 

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