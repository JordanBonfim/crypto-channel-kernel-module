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
#include <linux/crypto.h>     // Core da API de criptografia   
#include <linux/scatterlist.h>   // Estrutura para passar dados para a crypto
#include <crypto/skcipher.h>     // Symmetric Key Cipher API | API de Cifras Simétricas
#include <linux/proc_fs.h> // Criar pastas e arquivos no /proc
#include <linux/seq_file.h> // Para escrever em arquivos do /proc
#define BLOCK_SIZE 16


#define BUFFER_SIZE 1024 
static char *kernel_buffer; // Buffer no kernel
static DEFINE_MUTEX(crypto_mutex); // Mutex para proteger o acesso ao buffer
static int write_pos = 0; // Posição de escrita no buffer
static int read_pos = 0; // Posição de leitura no buffer
static int stored_count = 0; // Quantidade de dados armazenados no buffer

//CRIPTOGRAFIA
struct crypto_skcipher *tfm; // "Transform": O objeto que guarda o algoritmo (ex: AES)
struct skcipher_request *req; // "Request": O pedido de encriptação
char *crypto_key = "chave12345678901"; // Chave fixa por enquanto (16 bytes para AES-128)

// /proc
static struct proc_dir_entry *proc_folder; // Pasta no /proc/cryptochannel
static struct proc_dir_entry *proc_stats; // Arquivos stats
static struct proc_dir_entry *proc_config; // Arquivo config

static unsigned long stat_total_bytes = 0;
static unsigned long stat_total_msgs = 0;
static unsigned long stat_crypto_errors = 0;

static char current_key[17] = "chave12345678901"; // Chave atual (16 bytes + null terminator)

// Função para mostrar as estatísticas no /proc/cryptochannel/stats
static int stats_show(struct seq_file *m, void *v) {
    seq_printf(m, "Total Bytes Processed: %lu\n", stat_total_bytes);
    seq_printf(m, "Total Messages Processed: %lu\n", stat_total_msgs);
    seq_printf(m, "Total Cryptography Errors: %lu\n", stat_crypto_errors);
    seq_printf(m, "Occupied Buffer: %d / %d bytes\n", stored_count, BUFFER_SIZE);
    seq_printf(m, "Current Key: %s\n", current_key);
    return 0;
}

// Função para abrir o arquivo de estatísticas
static int stats_open(struct inode *inode, struct file *file) {
    return single_open(file, stats_show, NULL);
}

static const struct proc_ops stats_fops = {
    .proc_open = stats_open,
    .proc_read = seq_read,
    .proc_lseek = seq_lseek,
    .proc_release = single_release,
};

static ssize_t config_write(struct file *file, const char __user *buf, size_t count, loff_t *offset) {
    char temp_buf[32]; // Buffer temporário para receber a chave do usuário
    int ret;

    if (count > 31) count = 31; // Limita o tamanho para evitar overflow

    if (copy_from_user(temp_buf, buf, count) != 0) {
        pr_info("CRYPTOCHANNEL: ERROR COPYING KEY FROM USER\n");
        return -EFAULT; // Memory fault
    }

    temp_buf[count] = '\0'; // Se \n terminar, substitui por null terminator

    // Remove \n se vier do comando echo
    if (count > 0 && temp_buf[count - 1] == '\n') {
        temp_buf[count - 1] = '\0';
        count--;
    }

    // Validacao simples: AES-128 requer chave de 16 bytes (padding)
    if(strlen(temp_buf) != BLOCK_SIZE) {
        pr_info("CRYPTOCHANNEL: KEY MUST BE 16 CHARACTERS LONG\n");
        return -EINVAL; // Invalid argument
    }

    mutex_lock(&crypto_mutex);

    // Tenta definir a nova chave no transform
    ret = crypto_skcipher_setkey(tfm, current_key, BLOCK_SIZE);

    // Verifica se a definição da chave antiga foi bem-sucedida
    if(ret) {
        pr_info("CRYPTOCHANNEL: FAILED TO SET OLD KEY BEFORE UPDATING\n");
        stat_crypto_errors++;
        mutex_unlock(&crypto_mutex);
        return -EINVAL; // Invalid argument
    } else {
        pr_info("CRYPTOCHANNEL: OLD KEY SET SUCCESSFULLY BEFORE UPDATING\n");
    }

    mutex_unlock(&crypto_mutex);
    return count;
}

// Definição das operações do arquivo /proc/cryptochannel/config
static const struct proc_ops config_fops = {
    .proc_write = config_write,
};


static ssize_t apply_padding(const char *in, size_t in_len, char **out) {
    size_t pad_len = BLOCK_SIZE - (in_len % BLOCK_SIZE);
    if (pad_len == 0) pad_len = BLOCK_SIZE;

    size_t out_len = in_len + pad_len;

    *out = kmalloc(out_len, GFP_KERNEL);
    if (!(*out)) {
        return -ENOMEM;
    }

    memcpy(*out, in, in_len);
    memset((*out) + in_len, pad_len, pad_len);

    return out_len;
}

static ssize_t remove_padding(char *buf, size_t buf_len) {
    if (buf_len == 0) return -EINVAL;

    unsigned char last = buf[buf_len - 1]; // byte de padding
    if (last < 1 || last > BLOCK_SIZE) {
        return -EINVAL;
    }

    size_t pad_len = (size_t)last;

    if (pad_len > buf_len) {
        return -EINVAL;
    }

    /* validar bytes de padding */
    for (size_t i = 0; i < pad_len; i++) {
        if ((unsigned char)buf[buf_len - 1 - i] != last) {
            return -EINVAL;
        }
    }

    return buf_len - pad_len;
}

static ssize_t device_read(struct file *file, char __user *buf, size_t count, loff_t *offset){
    char *temp_buf;
    size_t bytes_to_copy;

    pr_info("CRYPTOCHANNEL: READ OPERATION\n");

    mutex_lock(&crypto_mutex);

    if (stored_count == 0) {
        mutex_unlock(&crypto_mutex);
        return 0; // EOF
    }

    if (count > stored_count) {
        bytes_to_copy = stored_count;
    } else {
        bytes_to_copy = count;
    }

    temp_buf = kmalloc(bytes_to_copy, GFP_KERNEL);
    if (!temp_buf) {
        mutex_unlock(&crypto_mutex);
        pr_info("CRYPTOCHANNEL: MEMORY ALLOCATION FAILED\n");
        return -ENOMEM;
    }

    for (size_t i = 0; i < bytes_to_copy; i++) {
        temp_buf[i] = kernel_buffer[read_pos];
        read_pos = (read_pos + 1) % BUFFER_SIZE;
    }

    struct scatterlist sg;
    sg_init_one(&sg, temp_buf, bytes_to_copy);
    skcipher_request_set_crypt(req, &sg, &sg, bytes_to_copy, NULL);

    int ret = crypto_skcipher_decrypt(req);
    if (ret) {
        pr_info("CRYPTOCHANNEL: Decryption failed: %d\n", ret);
        stat_crypto_errors++;
        kfree(temp_buf);
        mutex_unlock(&crypto_mutex);
        return ret;
    }

    /* Remover PKCS#7 padding */
    ssize_t unpadded_len = remove_padding(temp_buf, bytes_to_copy);
    if (unpadded_len < 0) {
        pr_info("CRYPTOCHANNEL: INVALID PADDING\n");
        stat_crypto_errors++;
        kfree(temp_buf);
        mutex_unlock(&crypto_mutex);
        return -EINVAL;
    }

    if (copy_to_user(buf, temp_buf, unpadded_len) != 0) {
        kfree(temp_buf);
        mutex_unlock(&crypto_mutex);
        pr_info("CRYPTOCHANNEL: ERROR COPYING TO USER\n");
        return -EFAULT;
    }

    stored_count -= bytes_to_copy;
    kfree(temp_buf);

    mutex_unlock(&crypto_mutex);
    return unpadded_len;
}

static ssize_t device_write(struct file *file, const char __user *buf, size_t count, loff_t *offset){
    char *temp_buf;
    int free_space;

    pr_info("CRYPTOCHANNEL: WRITE OPERATION\n");

    mutex_lock(&crypto_mutex);

    free_space = BUFFER_SIZE - stored_count;

    if (free_space <= 0) {
        mutex_unlock(&crypto_mutex);
        pr_info("CRYPTOCHANNEL: NO SPACE LEFT ON DEVICE\n");
        return -ENOSPC;
    }

    if (count > free_space) {
        count = free_space;
    }

    temp_buf = kmalloc(count, GFP_KERNEL);
    if (!temp_buf) {
        mutex_unlock(&crypto_mutex);
        pr_info("CRYPTOCHANNEL: MEMORY ALLOCATION FAILED\n");
        return -ENOMEM;
    }

    if (copy_from_user(temp_buf, buf, count) != 0) {
        kfree(temp_buf);
        mutex_unlock(&crypto_mutex);
        pr_info("CRYPTOCHANNEL: ERROR COPYING FROM USER\n");
        return -EFAULT;
    }

    /* Aplicar padding */
    char *padded_buf;
    ssize_t padded_len = apply_padding(temp_buf, count, &padded_buf);
    kfree(temp_buf);
    if (padded_len < 0) {
        mutex_unlock(&crypto_mutex);
        pr_info("CRYPTOCHANNEL: PADDING FAILED\n");
        return padded_len;
    }

    struct scatterlist sg;
    sg_init_one(&sg, padded_buf, padded_len);
    skcipher_request_set_crypt(req, &sg, &sg, padded_len, NULL);

    int ret = crypto_skcipher_encrypt(req);
    if (ret) {
        pr_info("CRYPTOCHANNEL: ENCRYPTION FAILED: %d\n", ret);
        stat_crypto_errors++;
        kfree(padded_buf);
        mutex_unlock(&crypto_mutex);
        return ret;
    }

    for (size_t i = 0; i < padded_len; i++) {
        kernel_buffer[write_pos] = padded_buf[i];
        write_pos = (write_pos + 1) % BUFFER_SIZE;
    }

    stored_count += padded_len;
    stat_total_bytes += count;
    stat_total_msgs++;

    kfree(padded_buf);
    mutex_unlock(&crypto_mutex);

    return padded_len;
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

    // Alocar o transform de criptografia 
    // tfm é o algoritmo em si
    tfm = crypto_alloc_skcipher("ecb(aes)", 0, 0);
    if(IS_ERR(tfm)){
        pr_err("CRYPTOCHANNEL: FAILED TO ALLOCATE cbc(aes)");
        return PTR_ERR(tfm); 
    }

    // Request de operação
    // (req) é uma operação específica que você vai executar, como:
    // cifrar ou decifrar um buffer
    req = skcipher_request_alloc(tfm, GFP_KERNEL);
    if(!req){
        pr_err("CRYPTOCHANNEL: FAILED TO ALLOCATE request");
        crypto_free_skcipher(tfm); // Libera algoritmo em caso de erro
        kfree(kernel_buffer); // Libera também o buffer
        return -ENOMEM;
    }


    // Definir chave utilizada
    // 16 bytes é o padrão para AES-128
    crypto_skcipher_setkey(tfm, crypto_key, BLOCK_SIZE);



    // Limpar a memória com zeros
    memset(kernel_buffer, 0, BUFFER_SIZE);

    // Criar pasta e arquivos no /proc
    proc_folder = proc_mkdir("cryptochannel", NULL);
    if (!proc_folder) {
        pr_info("CRYPTOCHANNEL: FAILED TO CREATE /proc/cryptochannel\n");
        cleanup_module();
        return -ENOMEM;
    }

    // Criar arquivo stats (0444 = somente leitura) 
    proc_stats = proc_create("stats", 0444, proc_folder, &stats_fops);
    if (!proc_stats) {
        pr_info("CRYPTOCHANNEL: FAILED TO CREATE /proc/cryptochannel/stats\n");
        cleanup_module();
        return -ENOMEM;
    }

    // Criar arquivo config (0666 = leitura e escrita)
    proc_config = proc_create("config", 0666, proc_folder, &config_fops);
    if (!proc_config) {
        pr_info("CRYPTOCHANNEL: FAILED TO CREATE /proc/cryptochannel/config\n");
        cleanup_module();
        return -ENOMEM;
    }

    
    return 0;  
}  
  
void cleanup_module(void)  {  
    if (proc_config) {
        proc_remove(proc_config);
    }
    if (proc_stats) {
        proc_remove(proc_stats);
    }
    if (proc_folder) {
        proc_remove(proc_folder);
    }

    // Liberar a memória alocada
    if(kernel_buffer){
        kfree(kernel_buffer);
    }

    skcipher_request_free(req);
    crypto_free_skcipher(tfm);

    pr_info("CRYPTOCHANNEL: ENDED\n");
    unregister_chrdev(major_number, "cryptochannel");
}
 
 
MODULE_LICENSE("GPL");
