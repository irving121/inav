/*
 * This file is part of INAV.
 *
 * INAV is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * INAV is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with INAV.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Authors:
 * Dominic Clifton - Cleanflight implementation
 * John Ihlein - Initial FF32 code
 * Konstantin Sharlaimov - busDevice refactoring
*/

#include <stdbool.h>
#include <stdint.h>

#include "platform.h"
#include "build/debug.h"

#include "common/axis.h"
#include "common/maths.h"

#include "drivers/system.h"
#include "drivers/time.h"
#include "drivers/io.h"
#include "drivers/exti.h"
#include "drivers/bus.h"

#include "drivers/gyro_sync.h"

#include "drivers/sensor.h"
#include "drivers/accgyro/accgyro.h"
#include "drivers/accgyro/accgyro_mpu.h"

#if defined(USE_GYRO_MPU6050) || defined(USE_ACC_MPU6050)
#include "drivers/accgyro/accgyro_bus_mpu6050.h"

#define BIT_H_RESET                 0x80
#define MPU_CLK_SEL_PLLGYROZ        0x03
#define MPU_INQUIRY_MASK            0x7E

typedef enum {
    MPU6050_NONE,
    MPU6050_HALF_RESOLUTION,
    MPU6050_FULL_RESOLUTION
} mpuDetectionResult_e;

static bool mpu6050InitDone = false;

static void mpu6050AccAndGyroInit(gyroDev_t *gyro)
{
    busDevice_t * dev = gyro->dev;
    mpuIntExtiInit(gyro);

    busSetSpeed(dev, BUS_SPEED_INITIALIZATION);

    if (!mpu6050InitDone) {
        // Device Reset
        busWrite(dev, MPU_RA_PWR_MGMT_1, BIT_H_RESET);
        delay(150);

        // Clock Source PPL with Z axis gyro reference
        busWrite(dev, MPU_RA_PWR_MGMT_1, MPU_CLK_SEL_PLLGYROZ);
        delayMicroseconds(15);

        // Accel Sample Rate 1kHz
        // Gyroscope Output Rate =  1kHz when the DLPF is enabled
        busWrite(dev, MPU_RA_SMPLRT_DIV, gyroMPU6xxxGetDividerDrops(gyro));
        delayMicroseconds(15);

        // Accel and Gyro DLPF Setting
        busWrite(dev, MPU_RA_CONFIG, gyro->lpf);
        delayMicroseconds(1);

        // Gyro +/- 2000 DPS Full Scale
        busWrite(dev, MPU_RA_GYRO_CONFIG, INV_FSR_2000DPS << 3);
        delayMicroseconds(15);

        // Accel +/- 8 G Full Scale
        busWrite(dev, MPU_RA_ACCEL_CONFIG, INV_FSR_8G << 3);
        delayMicroseconds(15);

        busWrite(dev, MPU_RA_INT_PIN_CFG, 0 << 7 | 0 << 6 | 0 << 5 | 0 << 4 | 0 << 3 | 0 << 2 | 1 << 1 | 0 << 0); // INT_PIN_CFG   -- INT_LEVEL_HIGH, INT_OPEN_DIS, LATCH_INT_DIS, INT_RD_CLEAR_DIS, FSYNC_INT_LEVEL_HIGH, FSYNC_INT_DIS, I2C_BYPASS_EN, CLOCK_DIS
        delayMicroseconds(15);

#ifdef USE_MPU_DATA_READY_SIGNAL
        busWrite(dev, MPU_RA_INT_ENABLE, MPU_RF_DATA_RDY_EN);
        delayMicroseconds(15);
#endif

        mpu6050InitDone = true;
    }

    busSetSpeed(dev, BUS_SPEED_FAST);
}

static void mpu6050AccInit(accDev_t *acc)
{
    uint32_t magic = busDeviceReadScratchpad(acc->dev);

    if (magic == 0xFFFF6050) {
        acc->acc_1G = 512 * 8;
    }
    else {
        acc->acc_1G = 256 * 8;
    }
}

bool mpu6050AccDetect(accDev_t *acc)
{
    acc->dev = busDeviceOpen(BUSTYPE_ANY, DEVHW_MPU6050);
    if (acc->dev == NULL) {
        return false;
    }

    uint32_t magic = busDeviceReadScratchpad(acc->dev);
    if (magic == 0x00006050 || magic == 0xFFFF6050) {
        acc->initFn = mpu6050AccInit;
        acc->readFn = mpuAccRead;
        return true;
    }

    return false;
}

static bool mpu6050DeviceDetect(busDevice_t * dev)
{
    uint8_t in;
    uint8_t readBuffer[6];
    uint8_t attemptsRemaining = 5;

    busSetSpeed(dev, BUS_SPEED_INITIALIZATION);

    busWrite(dev, MPU_RA_PWR_MGMT_1, BIT_H_RESET);

    do {
        delay(150);

        busRead(dev, MPU_RA_WHO_AM_I, &in);
        in &= MPU_INQUIRY_MASK;
        if (in == MPUx0x0_WHO_AM_I_CONST || in == MPU6500_WHO_AM_I_CONST) {
            break;
        }
        if (!attemptsRemaining) {
            return false;
        }
    } while (attemptsRemaining--);

    if (in == MPU6500_WHO_AM_I_CONST) {
        return MPU6050_FULL_RESOLUTION;
    }
    else {
        // There is a map of revision contained in the android source tree which is quite comprehensive and may help to understand this code
        // See https://android.googlesource.com/kernel/msm.git/+/eaf36994a3992b8f918c18e4f7411e8b2320a35f/drivers/misc/mpu6050/mldl_cfg.c
        // determine product ID and accel revision
        busReadBuf(dev, MPU_RA_XA_OFFS_H, readBuffer, 6);
        uint8_t revision = ((readBuffer[5] & 0x01) << 2) | ((readBuffer[3] & 0x01) << 1) | (readBuffer[1] & 0x01);

        if (revision) {
            /* Congrats, these parts are better. */
            if (revision == 1) {
                return MPU6050_HALF_RESOLUTION;
            } else if (revision == 2) {
                return MPU6050_FULL_RESOLUTION;
            } else if ((revision == 3) || (revision == 7)) {
                return MPU6050_FULL_RESOLUTION;
            } else {
                return MPU6050_NONE;
            }
        } else {
            uint8_t productId;

            busRead(dev, MPU_RA_PRODUCT_ID, &productId);
            revision = productId & 0x0F;

            if (!revision) {
                return MPU6050_NONE;
            } else if (revision == 4) {
                return MPU6050_HALF_RESOLUTION;
            } else {
                return MPU6050_FULL_RESOLUTION;
            }
        }

        return MPU6050_NONE;
    }
}

bool mpu6050GyroDetect(gyroDev_t *gyro)
{
    gyro->dev = busDeviceInit(BUSTYPE_ANY, DEVHW_MPU6050, OWNER_MPU);
    if (gyro->dev == NULL) {
        return false;
    }

    mpuDetectionResult_e res = mpu6050DeviceDetect(gyro->dev);
    if (res == MPU6050_NONE) {
        busDeviceDeInit(gyro->dev);
        return false;
    }

    busDeviceWriteScratchpad(gyro->dev, res == MPU6050_FULL_RESOLUTION ? 0xFFFF6050 : 0x00006050);

    gyro->devConfig.mpu.gyroReadXRegister = MPU_RA_GYRO_XOUT_H;
    gyro->initFn = mpu6050AccAndGyroInit;
    gyro->readFn = mpuGyroRead;
    gyro->intStatusFn = mpuCheckDataReady;
    gyro->scale = 1.0f / 16.4f;     // 16.4 dps/lsb scalefactor

    return true;
}

#endif
