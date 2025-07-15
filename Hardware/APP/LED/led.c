
#include "led.h"
#include "global.h"
#include "gpio.h"
Led_Def ledstatus;
// Led_Port led_red_port[PORTMAX] = {{LED_R1_PORT,LED_R1_PIN},{LED_R2_PORT,LED_R2_PIN},{LED_R3_PORT,LED_R3_PIN},{LED_R4_PORT,LED_R4_PIN}};
// Led_Port led_green_port[PORTMAX] = {{LED_G1_PORT,LED_G1_PIN},{LED_G2_PORT,LED_G2_PIN},{LED_G3_PORT,LED_G3_PIN},{LED_G4_PORT,LED_G4_PIN}};;


void App_Led_Ctrl(uint8_t door,Led_Def flag)
{
    // if(door == 0)
    // {
    //     for(uint8_t i = 0; i < PORTMAX; i++)
    //     {
    //         if(flag == LED_OFF)
    //         {
    //             gpio_bit_set(led_red_port[i].port,led_red_port[i].pin);
    //             gpio_bit_reset(led_green_port[i].port,led_green_port[i].pin);   
    //         }
    //         else if(flag == LED_ON)
    //         {
    //             gpio_bit_set(led_green_port[i].port,led_green_port[i].pin);
    //             gpio_bit_reset(led_red_port[i].port,led_red_port[i].pin);
    //         }
            
    //     }
    // }
    // else
    // {
    //     if(flag == LED_OFF)
    //     {
    //         gpio_bit_set(led_red_port[door-1].port,led_red_port[door-1].pin);
    //         gpio_bit_reset(led_green_port[door-1].port,led_green_port[door-1].pin);
    //     }
    //     else if(flag == LED_ON)
    //     {
    //         gpio_bit_set(led_green_port[door-1].port,led_green_port[door-1].pin);
    //         gpio_bit_reset(led_red_port[door-1].port,led_red_port[door-1].pin);
    //     }
    // }

}

void App_Ledturnoff(void)
{
    // for(uint8_t i = 0; i < PORTMAX; i++)
    // {
    //     gpio_bit_reset(led_green_port[i].port,led_green_port[i].pin);
    //     gpio_bit_reset(led_red_port[i].port,led_red_port[i].pin);
    // }

}

void App_Qrledtoggle(void)
{
    static uint32_t time = 0;
    time++;
    switch (ledstatus)
    {
        case LED_FLASH:
            if(time % 2 == 0)
            {
                EN_QR_ON;
            }
            else
            {
                EN_QR_OFF;
            }
            /* code */
            break;
        
        case LED_ON:
            EN_QR_ON;
            break;
        
        default:
            break;
    }
}