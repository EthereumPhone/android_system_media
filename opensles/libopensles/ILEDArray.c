/*
 * Copyright (C) 2010 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* LEDArray implementation */

#include "sles_allinclusive.h"

static SLresult ILEDArray_ActivateLEDArray(SLLEDArrayItf self, SLuint32 lightMask)
{
    ILEDArray *this = (ILEDArray *) self;
    interface_lock_poke(this);
    this->mLightMask = lightMask;
    interface_unlock_poke(this);
    return SL_RESULT_SUCCESS;
}

static SLresult ILEDArray_IsLEDArrayActivated(SLLEDArrayItf self, SLuint32 *pLightMask)
{
    if (NULL == pLightMask)
        return SL_RESULT_PARAMETER_INVALID;
    ILEDArray *this = (ILEDArray *) self;
    interface_lock_peek(this);
    SLuint32 lightMask = this->mLightMask;
    interface_unlock_peek(this);
    *pLightMask = lightMask;
    return SL_RESULT_SUCCESS;
}

static SLresult ILEDArray_SetColor(SLLEDArrayItf self, SLuint8 index, const SLHSL *pColor)
{
    if (NULL == pColor)
        return SL_RESULT_PARAMETER_INVALID;
    SLHSL color = *pColor;
    if (!(0 <= color.hue && color.hue <= 360000))
        return SL_RESULT_PARAMETER_INVALID;
    if (!(0 <= color.saturation && color.saturation <= 1000))
        return SL_RESULT_PARAMETER_INVALID;
    if (!(0 <= color.lightness && color.lightness <= 1000))
        return SL_RESULT_PARAMETER_INVALID;
    ILEDArray *this = (ILEDArray *) self;
    // can't use poke because struct copy might not be atomic
    interface_lock_exclusive(this);
    this->mColors[index] = color;
    interface_unlock_exclusive(this);
    return SL_RESULT_SUCCESS;
}

static SLresult ILEDArray_GetColor(SLLEDArrayItf self, SLuint8 index,
    SLHSL *pColor) {
    if (NULL == pColor)
        return SL_RESULT_PARAMETER_INVALID;
    ILEDArray *this = (ILEDArray *) self;
    // can't use peek because struct copy might not be atomic
    interface_lock_shared(this);
    SLHSL color = this->mColors[index];
    interface_unlock_shared(this);
    *pColor = color;
    return SL_RESULT_SUCCESS;
}

static const struct SLLEDArrayItf_ ILEDArray_Itf = {
    ILEDArray_ActivateLEDArray,
    ILEDArray_IsLEDArrayActivated,
    ILEDArray_SetColor,
    ILEDArray_GetColor,
};

void ILEDArray_init(void *self)
{
    ILEDArray *this = (ILEDArray *) self;
    this->mItf = &ILEDArray_Itf;
    this->mLightMask = 0;
    SLHSL *color = this->mColors;
    SLuint8 index;
    for (index = 0; index < MAX_LED_COUNT; ++index, ++color) {
        color->hue = 0; // red, but per specification 1.0.1 pg. 259: "Default color is undefined."
        color->saturation = 1000;
        color->lightness = 1000;
    }
    // const
    this->mCount = MAX_LED_COUNT;
}
