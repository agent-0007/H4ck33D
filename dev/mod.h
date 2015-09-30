/*
 * mod.h
 *
 *  Created on: 12-07-2014
 *      Author: Tomek
 */

#ifndef DEV_MOD_H_
#define DEV_MOD_H_

#include <util/stdlib.h>

/* initialize modulator */
int MOD_Init(void);
/* send size */
int MOD_Send(void *ptr, size_t size);


#endif /* MOD_H_ */
