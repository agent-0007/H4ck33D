/*
 * mod.c
 *
 *  Created on: 12-07-2014
 *      Author: Tomek
 */

#include <stm32l/rcc.h>
#include <stm32l/nvic.h>
#include <stm32l/gpio.h>
#include <stm32l/timer.h>
#include <sys/err.h>
#include <sys/time.h>
#include <util/stdlib.h>

#define DEBUG
#include <dev/debug.h>

/* buffer */
typedef struct {
	/* bit counter, byte counter */
	uint16_t cnt, size, enable;
	/* byte ptr, frame size */
	uint32_t *ptr;
	/* data buffer */
	uint32_t buf[32];
	/* oscillator startup */
	uint8_t startup;
	/* t_silent */
	timer_t t_silent;
} buf_t;
/* buffer */
static buf_t buf;

/* modulator output */
static bitband_t m = BITBAND_PERIPH(&GPIOA->ODR, 12);
/* sender enable flag */
static bitband_t e = BITBAND_PERIPH(&TIM6->CR1, 0);

/* interrupt routine */
static void MOD_Tim6Isr(void)
{
	/* clear interrupt flag */
	TIM6->SR = 0;

	/* need for new byte? */
	if (buf.cnt == 0) {
		/* eat a byte */
		buf.size--;
		/* still some words buffered */
		if (buf.size > 0) {
			/* set counters */
			buf.ptr++; buf.cnt = buf.size == 1 ? 6 : 24;
		/* no data, not much to do */
		} else {
			/* release line */
			*m = 0;
			 buf.t_silent = time; buf.enable = 0;
			/* end of processing */
			return;
		}
	}

	/* modulate */
	*m = *(buf.ptr) & 1;

//	/* oscillator startup condition */
//	if ((*(buf.ptr) & 0x3) == 0x2) {
//		/* startup condition found */
//		buf.startup = 1;
//		/* adjust auto reload register */
//		TIM6->ARR = 540;
//	/* compensate for startup conddition */
//	} else if (buf.startup == 1) {
//		TIM6->ARR = 940; buf.startup = 2;
//	} else if (buf.startup == 2) {
//		TIM6->ARR = 740; buf.startup = 0;
//	}

	/* next bit */
	*(buf.ptr) = *(buf.ptr) >> 1;
	/* next bit */
	buf.cnt--;

	/* re-enable */
	*e = 1;
}

/* encode byte */
static uint32_t MOD_EncodeByte(uint8_t byte)
{
	/* resulting encoded word */
	uint32_t result = 0x249249, mask = 0x2;
	/* encode bits */
	while (byte) {
		/* bit present */
		if (byte & 1) {
			/* set bit */
			result |= mask;
		}
		/* next bit */
		byte = byte >> 1;
		/* next mask */
		mask = mask << 3;
	}

	/* report result */
	return result;
}

/* encode frame */
void MOD_Encode(void *ptr, uint8_t size)
{
	/* byte-wise pointer */
	uint8_t *s = ptr;
	/* buffer pointer */
	uint32_t *d = buf.buf;

	/* set preamble */
	*(d++) = 0x00555555; buf.cnt = 33;
	/* set initial size */
	buf.size = size + 1;

	/* copy and encode bytes */
	while (size > 0) {
		/* encode */
		*(d++) = MOD_EncodeByte(*(s++));
		/* next byte */
		size--;
	}


	/* set initial pointer value */
	buf.ptr = buf.buf;
}

/* initialize modulator */
int MOD_Init(void)
{
	/* enable gpio */
	RCC->AHBENR |= RCC_AHBENR_GPIOAEN;
	/* enable timer */
	RCC->APB1ENR |= RCC_APB1ENR_TIM6EN;

	/* prepare gpio */
	/* reset moder */
	GPIOA->MODER &= ~GPIO_MODER_MODER12;
	/* set as output */
	GPIOA->MODER |= GPIO_MODER_MODER12_0;

	/* configure anode timer */
	/* auto reload value - 2 kHz */
	TIM6->ARR = 1547;
	/* enable interrupt generation on update event */
	TIM6->DIER |= TIM_DIER_UIE;
	/* one-timer */
	TIM6->CR1 |= TIM_CR1_OPM;

	/* anode interrupt timer */
	NVIC_SetInterruptVector(NVIC_TIM6, MOD_Tim6Isr);
	/* configure led priority */
	NVIC_SetInterruptPriority(NVIC_TIM6, 1);
	/* enable interrupt */
	NVIC_EnableInterrupt(NVIC_TIM6);

	/* report status */
	return EOK;
}

/* send size */
int MOD_Send(void *ptr, size_t size)
{
	/* sender is busy */
	if (*e) {
		return EBUSY;
	}

	/* encode frame */
	MOD_Encode(ptr, size);
	/* start sending */
	*e = 1;

	/* report number of bytes sent */
	return size;
}
