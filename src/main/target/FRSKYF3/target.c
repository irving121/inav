/*
 * This file is part of Cleanflight.
 *
 * Cleanflight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Cleanflight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Cleanflight.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdint.h>

#include <platform.h>
#include "drivers/io.h"
#include "drivers/pwm_mapping.h"
#include "drivers/timer.h"

const timerHardware_t timerHardware[USABLE_TIMER_CHANNEL_COUNT] = {

    { TIM4, IO_TAG(PB9),  TIM_Channel_4, 1, IOCFG_AF_PP, GPIO_AF_2, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR }, // PWM1
    { TIM4, IO_TAG(PB8),  TIM_Channel_3, 1, IOCFG_AF_PP, GPIO_AF_2, TIM_USE_MC_MOTOR | TIM_USE_FW_MOTOR }, // PWM2
    { TIM3, IO_TAG(PB1),  TIM_Channel_4, 1, IOCFG_AF_PP, GPIO_AF_2, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO }, // PWM3
    { TIM2, IO_TAG(PA1),  TIM_Channel_2, 1, IOCFG_AF_PP, GPIO_AF_1, TIM_USE_MC_MOTOR | TIM_USE_FW_SERVO }, // PWM4

    // DEF_TIM(TIM2, CH3, PA2, TIM_USE_MOTOR, TIMER_OUTPUT_STANDARD), // PWM5
    // DEF_TIM(TIM2, CH4, PA3, TIM_USE_MOTOR, TIMER_OUTPUT_STANDARD), // PWM6
    // DEF_TIM(TIM2, CH1, PA0, TIM_USE_MOTOR, TIMER_OUTPUT_STANDARD), // PWM7
    // DEF_TIM(TIM3, CH3, PB0, TIM_USE_MOTOR, TIMER_OUTPUT_STANDARD), // PWM8

    // DEF_TIM(TIM1, CH1, PA8, TIM_USE_LED|TIM_USE_TRANSPONDER, TIMER_OUTPUT_ENABLED), // LED

    // DEF_TIM(TIM8, CH3, PB9, TIM_USE_MOTOR, TIMER_OUTPUT_STANDARD), // PWM1
    // DEF_TIM(TIM8, CH2, PB8, TIM_USE_MOTOR, TIMER_OUTPUT_STANDARD), // PWM2
    // DEF_TIM(TIM3, CH4, PB1, TIM_USE_MOTOR, TIMER_OUTPUT_STANDARD), // PWM3
    // DEF_TIM(TIM2, CH2, PA1, TIM_USE_MOTOR, TIMER_OUTPUT_STANDARD), // PWM4
    // DEF_TIM(TIM2, CH3, PA2, TIM_USE_MOTOR, TIMER_OUTPUT_STANDARD), // PWM5
    // DEF_TIM(TIM2, CH4, PA3, TIM_USE_MOTOR, TIMER_OUTPUT_STANDARD), // PWM6
    // DEF_TIM(TIM2, CH1, PA0, TIM_USE_MOTOR, TIMER_OUTPUT_STANDARD), // PWM7
    // DEF_TIM(TIM3, CH3, PB0, TIM_USE_MOTOR, TIMER_OUTPUT_STANDARD), // PWM8

    // DEF_TIM(TIM1, CH1, PA8, TIM_USE_LED|TIM_USE_TRANSPONDER, TIMER_OUTPUT_ENABLED), // LED
};
