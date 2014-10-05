#ifndef _U8G_ARM_H
#define _U8G_ARM_H

#include "stm32f10x_it.h"
#include "u8g.h"

/* system helper procedures */
// void init_system_clock(void);	 optional: can be called from init code 

void delay_system_ticks(uint32_t sys_ticks);	
void delay_micro_seconds(uint32_t us);


// #define PIN(base,bit) ((base)*32+(bit))
// #define DEFAULT_KEY PIN(0,12)

// void set_gpio_mode(uint16_t pin, uint8_t is_output, uint8_t is_pullup) U8G_NOINLINE;
// void set_gpio_level(uint16_t pin, uint8_t level) U8G_NOINLINE;
// uint8_t get_gpio_level(uint16_t pin) U8G_NOINLINE;

// void spi_init(uint32_t ns) U8G_NOINLINE;
// void spi_out(uint8_t data);

// extern uint16_t u8g_pin_a0;
// extern uint16_t u8g_pin_cs;
// extern uint16_t u8g_pin_rst;

uint8_t u8g_com_hw_spi_fn(u8g_t *u8g, uint8_t msg, uint8_t arg_val, void *arg_ptr);

#endif


