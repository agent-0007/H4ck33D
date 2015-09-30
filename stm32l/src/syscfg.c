/*
 * syscfg.c
 *
 *  Created on: 31-03-2013
 *      Author: Tomek
 */

#include <stm32l/syscfg.h>

/* usb pullup configuration */
void SYSCFG_USBPuCmd(int state)
{
	/* enable/ disable pullup */
	if (state) {
		SYSCFG->PMC |= SYSCFG_PMC_USB_PU;
	} else {
		SYSCFG->PMC &= ~SYSCFG_PMC_USB_PU;
	}
}

/* configures syscfg mux */
void SYSCFG_EXTILineConfig(uint8_t pin, uint8_t port)
{
	/* mask */
	uint32_t mask = ~(0x0f << ((pin & 0x3) * 4));
	/* temporary storage */
	uint32_t temp = SYSCFG->EXTICR[pin >> 2] & mask;
	/* update */
	temp |= port << ((pin & 0x3) * 4);
	/* rewrite */
	SYSCFG->EXTICR[pin >> 2] = temp;
}
