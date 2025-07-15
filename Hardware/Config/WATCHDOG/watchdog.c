#include "watchdog.h"

void YA_Watchdog_Init(void)
{
    /* enable IRC40K */
    rcu_osci_on(RCU_IRC40K);
    /* wait till IRC40K is ready */
    while(SUCCESS != rcu_osci_stab_wait(RCU_IRC40K));
    fwdgt_config(0xfff, FWDGT_PSC_DIV256);  
    fwdgt_counter_reload();
    fwdgt_enable();
    dbg_periph_enable(DBG_FWDGT_HOLD);
}


void Feed_Watchdog(void)
{
    fwdgt_counter_reload();
}
