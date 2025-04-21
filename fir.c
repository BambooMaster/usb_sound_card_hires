/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 BambooMaster
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

/**
 * @file fir.c
 * @author BambooMaster (https://misskey.hakoniwa-project.com/@BambooMaster)
 * @brief usb_sound_card_hires
 * @version 0.4-interpolation
 * @date 2025-04-22
 * 
 */

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "hardware/dma.h"
#include "arm_math.h"

#include "i2s.h"
#include "fir.h"

static int doorbell_dsp;
static uint32_t sample_r;
static q31_t buff_r[49 * 8];
static float32_t dsp_buff_r_1[49 * 8];

//48*8
static arm_fir_interpolate_instance_f32 s_r_1st;
static arm_fir_interpolate_instance_f32 s_r_2nd;
static float32_t state_r_1st[(FIR_1ST_140DB_TAPS / 2) + FIR_1ST_BLOCK_SIZE - 1] = {0.0f};
static float32_t state_r_2nd[(FIR_2ND_140DB_TAPS / 4) + FIR_2ND_BLOCK_SIZE - 1] = {0.0f};

//96*4
static arm_fir_interpolate_instance_f32 s_r_1st_96;
static arm_fir_interpolate_instance_f32 s_r_2nd_96;
static float32_t state_r_1st_96[(FIR_1ST_140DB_96_TAPS / 2) + FIR_1ST_96_BLOCK_SIZE - 1] = {0.0f};
static float32_t state_r_2nd_96[(FIR_2ND_140DB_96_TAPS / 2) + FIR_2ND_96_BLOCK_SIZE - 1] = {0.0f};

void dsp_init(void){
    //doorbell init
    doorbell_dsp = multicore_doorbell_claim_unused((1 << NUM_CORES) - 1, true);
    multicore_doorbell_clear_current_core(doorbell_dsp);

    //48*8
    arm_fir_interpolate_init_f32(&s_r_1st, 2, FIR_1ST_140DB_TAPS, fir_1st_140db, state_r_1st, FIR_1ST_BLOCK_SIZE);
    arm_fir_interpolate_init_f32(&s_r_2nd, 4, FIR_2ND_140DB_TAPS, fir_2nd_140db, state_r_2nd, FIR_2ND_BLOCK_SIZE);

    //96*4
    arm_fir_interpolate_init_f32(&s_r_1st_96, 2, FIR_1ST_140DB_96_TAPS, fir_1st_140db_96, state_r_1st_96, FIR_1ST_96_BLOCK_SIZE);
    arm_fir_interpolate_init_f32(&s_r_2nd_96, 2, FIR_2ND_140DB_96_TAPS, fir_2nd_140db_96, state_r_2nd_96, FIR_2ND_96_BLOCK_SIZE);
}

void __not_in_flash_func(core0_task)(void){
    if (multicore_doorbell_is_set_current_core(doorbell_dsp)){
        float32_t dsp_buff_r_2[49 * 8];
        #if 1
        if (sample_r <= 49){
            arm_scale_f32(dsp_buff_r_1, 8.0, dsp_buff_r_1, sample_r);

            arm_fir_interpolate_f32(&s_r_1st, dsp_buff_r_1, dsp_buff_r_2, sample_r);
            sample_r *= 2;
            arm_fir_interpolate_f32(&s_r_2nd, dsp_buff_r_2, dsp_buff_r_1, sample_r);
            sample_r *= 4;
        }
        else{
            arm_scale_f32(dsp_buff_r_1, 4.0, dsp_buff_r_1, sample_r);

            arm_fir_interpolate_f32(&s_r_1st_96, dsp_buff_r_1, dsp_buff_r_2, sample_r);
            sample_r *= 2;
            arm_fir_interpolate_f32(&s_r_2nd_96, dsp_buff_r_2, dsp_buff_r_1, sample_r);
            sample_r *= 2;
        }
        arm_float_to_q31(dsp_buff_r_1, buff_r, sample_r);
        #else
        if (sample_r <= 49){
            for (int i = 0, j = 0; i < sample_r; i++){
                dsp_buff_r_2[j++] = dsp_buff_r_1[i];
                dsp_buff_r_2[j++] = dsp_buff_r_1[i];
                dsp_buff_r_2[j++] = dsp_buff_r_1[i];
                dsp_buff_r_2[j++] = dsp_buff_r_1[i];
                dsp_buff_r_2[j++] = dsp_buff_r_1[i];
                dsp_buff_r_2[j++] = dsp_buff_r_1[i];
                dsp_buff_r_2[j++] = dsp_buff_r_1[i];
                dsp_buff_r_2[j++] = dsp_buff_r_1[i];
            }
            sample_r *= 8;
        }
        else {
            for (int i = 0, j = 0; i < sample_r; i++){
                dsp_buff_r_2[j++] = dsp_buff_r_1[i];
                dsp_buff_r_2[j++] = dsp_buff_r_1[i];
                dsp_buff_r_2[j++] = dsp_buff_r_1[i];
                dsp_buff_r_2[j++] = dsp_buff_r_1[i];
            }
            sample_r *= 4;
        }
        arm_float_to_q31(dsp_buff_r_2, buff_r, sample_r);
        #endif
        multicore_doorbell_clear_current_core(doorbell_dsp);
    }
}

void __not_in_flash_func(core1_main)(void){
    int32_t* buff;
    int dma_sample[2], sample;
    bool mute = false;
    int32_t mute_buff[96 * 2] = {0};
    uint32_t mute_len = sizeof(mute_buff) / sizeof(int32_t);
    int8_t buf_length;
    int32_t dma_buff[2][49 * 8 * 2];
    uint8_t dma_use = 0;
    int dma_ch = 0;
    uint32_t previous_length = 44 * 2;

    //48*8
    arm_fir_interpolate_instance_f32 s_l_1st;
    arm_fir_interpolate_instance_f32 s_l_2nd;
    float32_t state_l_1st[(FIR_1ST_140DB_TAPS / 2) + FIR_1ST_BLOCK_SIZE - 1] = {0.0f};
    float32_t state_l_2nd[(FIR_2ND_140DB_TAPS / 4) + FIR_2ND_BLOCK_SIZE - 1] = {0.0f};
    arm_fir_interpolate_init_f32(&s_l_1st, 2, FIR_1ST_140DB_TAPS, fir_1st_140db, state_l_1st, FIR_1ST_BLOCK_SIZE);
    arm_fir_interpolate_init_f32(&s_l_2nd, 4, FIR_2ND_140DB_TAPS, fir_2nd_140db, state_l_2nd, FIR_2ND_BLOCK_SIZE);

    //96*4
    arm_fir_interpolate_instance_f32 s_l_1st_96;
    arm_fir_interpolate_instance_f32 s_l_2nd_96;
    float32_t state_l_1st_96[(FIR_1ST_140DB_96_TAPS / 2) + FIR_1ST_96_BLOCK_SIZE - 1] = {0.0f};
    float32_t state_l_2nd_96[(FIR_2ND_140DB_96_TAPS / 2) + FIR_2ND_96_BLOCK_SIZE - 1] = {0.0f};
    arm_fir_interpolate_init_f32(&s_l_1st_96, 2, FIR_1ST_140DB_96_TAPS, fir_1st_140db_96, state_l_1st_96, FIR_1ST_96_BLOCK_SIZE);
    arm_fir_interpolate_init_f32(&s_l_2nd_96, 2, FIR_2ND_140DB_96_TAPS, fir_2nd_140db_96, state_l_2nd_96, FIR_2ND_96_BLOCK_SIZE);

    //gpio_init(15);
    //gpio_set_dir(15, GPIO_OUT);

    while (1){
        static float32_t dsp_buff_l_1[49 * 8];
        static float32_t dsp_buff_l_2[49 * 8];
        static q31_t buff_l[49 * 8];

        //gpio_put(15, 1);
        buf_length = i2s_get_buf_length();

        if (buf_length == 0){
            mute = true;
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
            ///set_playback_state(false);
        }
        else if (buf_length >= I2S_START_LEVEL && mute == true){
            mute = false;
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
            //set_playback_state(true);
        }

        if (mute == true){
            buff = mute_buff;
            sample = previous_length;
        }
        else if (i2s_dequeue(&buff, &sample) == false){
            buff = mute_buff;
            sample = previous_length;
        }
        else{
            previous_length = sample;
        }

        //int32_tをfloat32_tに変換
        arm_q31_to_float(buff, dsp_buff_l_2, sample);
        
        //左右に分割
        sample /= 2;
        for (int i = 0, j = 0; i < sample; i++){
            dsp_buff_l_1[i] = dsp_buff_l_2[j++];
            dsp_buff_r_1[i] = dsp_buff_l_2[j++];
        }
        sample_r = sample;

        //core0_task開始
        multicore_doorbell_set_other_core(doorbell_dsp);

        if (sample <= 49){
            arm_scale_f32(dsp_buff_l_1, 8.0, dsp_buff_l_1, sample);

            arm_fir_interpolate_f32(&s_l_1st, dsp_buff_l_1, dsp_buff_l_2, sample);
            sample *= 2;
            arm_fir_interpolate_f32(&s_l_2nd, dsp_buff_l_2, dsp_buff_l_1, sample);
            sample *= 4;
        }
        else{
            arm_scale_f32(dsp_buff_l_1, 4.0, dsp_buff_l_1, sample);

            arm_fir_interpolate_f32(&s_l_1st_96, dsp_buff_l_1, dsp_buff_l_2, sample);
            sample *= 2;
            arm_fir_interpolate_f32(&s_l_2nd_96, dsp_buff_l_2, dsp_buff_l_1, sample);
            sample *= 2;
        }
        arm_float_to_q31(dsp_buff_l_1, buff_l, sample);

        //core0_taskが終わるまで待機
        while(multicore_doorbell_is_set_other_core(doorbell_dsp)){
            tight_loop_contents();
        }

        //i2sバッファに格納
        for (int i = 0, j = 0; i < sample; i++){
            dma_buff[dma_use][j++] = buff_l[i];
            dma_buff[dma_use][j++] = buff_r[i];
        }
        dma_sample[dma_use] = sample * 2;
        //gpio_put(15, 0);

        dma_channel_wait_for_finish_blocking(dma_ch);
        dma_channel_transfer_from_buffer_now(dma_ch, dma_buff[dma_use], dma_sample[dma_use]);
        dma_use ^= 1;
    }
}