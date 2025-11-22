/*   
 * MODULO DE CANAL CRIPTOGRAFICO - CRYPTOCHANNEL
 */

#include <linux/module.h> // Needed by all modules
#include <linux/printk.h> // Needed for pr_info()
#include <linux/device.h> // Para criar a classe e o dispositivo no sistema (o que faz o arquivo aparecer no /dev).
#include <linux/uaccess.h> //Para futuramente mover dados entre o usuário e o kernel (segurança).
  
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