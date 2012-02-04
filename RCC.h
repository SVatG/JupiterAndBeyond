#ifndef __RCC_H__
#define __RCC_H__

#include <stdint.h>
#include <stm32f4xx.h>

static inline void EnableAHB1PeripheralClock(uint32_t peripherals) { RCC->AHB1ENR|=peripherals; }
static inline void EnableAHB2PeripheralClock(uint32_t peripherals) { RCC->AHB2ENR|=peripherals; }
static inline void EnableAHB3PeripheralClock(uint32_t peripherals) { RCC->AHB3ENR|=peripherals; }
static inline void EnableAPB1PeripheralClock(uint32_t peripherals) { RCC->APB1ENR|=peripherals; }
static inline void EnableAPB2PeripheralClock(uint32_t peripherals) { RCC->APB2ENR|=peripherals; }

static inline void DisableAHB1PeripheralClock(uint32_t peripherals) { RCC->AHB1ENR&=~peripherals; }
static inline void DisableAHB2PeripheralClock(uint32_t peripherals) { RCC->AHB2ENR&=~peripherals; }
static inline void DisableAHB3PeripheralClock(uint32_t peripherals) { RCC->AHB3ENR&=~peripherals; }
static inline void DisableAPB1PeripheralClock(uint32_t peripherals) { RCC->APB1ENR&=~peripherals; }
static inline void DisableAPB2PeripheralClock(uint32_t peripherals) { RCC->APB2ENR&=~peripherals; }

static inline void SetAHB1PeripheralReset(uint32_t peripherals) { RCC->AHB1RSTR|=peripherals; }
static inline void SetAHB2PeripheralReset(uint32_t peripherals) { RCC->AHB2RSTR|=peripherals; }
static inline void SetAHB3PeripheralReset(uint32_t peripherals) { RCC->AHB3RSTR|=peripherals; }
static inline void SetAPB1PeripheralReset(uint32_t peripherals) { RCC->APB1RSTR|=peripherals; }
static inline void SetAPB2PeripheralReset(uint32_t peripherals) { RCC->APB2RSTR|=peripherals; }

static inline void ClearAHB1PeripheralReset(uint32_t peripherals) { RCC->AHB1RSTR&=~peripherals; }
static inline void ClearAHB2PeripheralReset(uint32_t peripherals) { RCC->AHB2RSTR&=~peripherals; }
static inline void ClearAHB3PeripheralReset(uint32_t peripherals) { RCC->AHB3RSTR&=~peripherals; }
static inline void ClearAPB1PeripheralReset(uint32_t peripherals) { RCC->APB1RSTR&=~peripherals; }
static inline void ClearAPB2PeripheralReset(uint32_t peripherals) { RCC->APB2RSTR&=~peripherals; }

static inline void EnableAHB1PeripheralLowPowerClock(uint32_t peripherals) { RCC->AHB1LPENR|=peripherals; }
static inline void EnableAHB2PeripheralLowPowerClock(uint32_t peripherals) { RCC->AHB2LPENR|=peripherals; }
static inline void EnableAHB3PeripheralLowPowerClock(uint32_t peripherals) { RCC->AHB3LPENR|=peripherals; }
static inline void EnableAPB1PeripheralLowPowerClock(uint32_t peripherals) { RCC->APB1LPENR|=peripherals; }
static inline void EnableAPB2PeripheralLowPowerClock(uint32_t peripherals) { RCC->APB2LPENR|=peripherals; }

static inline void DisableAHB1PeripheralLowPowerClock(uint32_t peripherals) { RCC->AHB1LPENR&=~peripherals; }
static inline void DisableAHB2PeripheralLowPowerClock(uint32_t peripherals) { RCC->AHB2LPENR&=~peripherals; }
static inline void DisableAHB3PeripheralLowPowerClock(uint32_t peripherals) { RCC->AHB3LPENR&=~peripherals; }
static inline void DisableAPB1PeripheralLowPowerClock(uint32_t peripherals) { RCC->APB1LPENR&=~peripherals; }
static inline void DisableAPB2PeripheralLowPowerClock(uint32_t peripherals) { RCC->APB2LPENR&=~peripherals; }

#endif
