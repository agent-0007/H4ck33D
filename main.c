/*
 * main.c
 *
 *      Author: Tomek
 */

#include <stm32l/rcc.h>
#include <sys/time.h>

#define DEBUG
#include <dev/debug.h>

/* timer */
static timer_t timer;
/* frame payload */
static uint8_t pld1[] = {0x62, 0x41, 0x27, 0xed, 0xa2, 0x4d,
		0xfe, 0xdf , 0x01};
/* frame payload */
static uint8_t pld2[] = {0x48, 0x3f, 0xda, 0xc5, 0xf8, 0xa1,
		0xf7, 0xdf, 0x00};
/* frame payload */
static uint8_t pld3[] = {0x15, 0x5c, 0x81, 0x91, 0xf8, 0xa1,
		0xf7, 0xdf, 0x00};
/* current state */
static uint8_t state;

/* system entry point */
int main(void)
{
	/* reset interrupt unit */
	NVIC_Init();
	/* enable interrupts */
	STM32_EnableInterrupts();

	RCC_MSIRangeConfig(MSI_FREQ_4M194);

	/* initialize m2m transfers */
	M2M_Init();
	/* initialize time base */
	SysTime_Init();
	/* initialize debug interface */
	USART1_Init();
	/* initialize modulator */
	MOD_Init();

	/* main program loop */
	for (;;) {

		if (time - timer < 105) {
			continue;
		}

		dprintf("state = %d\n", state);

		switch (state) {
		case 0 : {
			MOD_Send(pld1, sizeof(pld1)); state = 1;
		} break;
		case 1 : {
			MOD_Send(pld2, sizeof(pld2)); state = 2;
		} break;
		case 2 : {
			MOD_Send(pld3, sizeof(pld3)); state = 0;
		} break;
		}

		timer = time;

	}

	/* never reached */
	return 0;
}


