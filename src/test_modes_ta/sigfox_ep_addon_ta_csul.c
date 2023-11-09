/*!*****************************************************************
 * \file    sigfox_ep_addon_ta_csul.c
 * \brief   Sigfox End-Point type approval add_on continuous Sigfox uplink test.
 *******************************************************************
 * \copyright
 *
 * Copyright (c) 2022, UnaBiz SAS
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1 Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *  2 Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *  3 Neither the name of UnaBiz SAS nor the names of its contributors may be
 *    used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 * BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *******************************************************************/

#include "test_modes_ta/sigfox_ep_addon_ta_csul.h"

#ifdef USE_SIGFOX_EP_FLAGS_H
#include "sigfox_ep_flags.h"
#endif
#include "manuf/mcu_api.h"
#include "sigfox_ep_addon_ta_api.h"
#include "sigfox_rc.h"
#include "sigfox_types.h"
#include "sigfox_error.h"
#include "sigfox_ep_api_test.h"

/*** SIGFOX EP ADDON CSUL local macros ***/

#define SIGFOX_EP_ADDON_TA_CSUL_INTERFRAME_TIMER_MS			200 // In order to comply with the 20s FCC specification.

#define SIGFOX_EP_ADDON_TA_CSUL_PN7_TAP						6
#define SIGFOX_EP_ADDON_TA_CSUL_PN7_LENGTH					7
#define SIGFOX_EP_ADDON_TA_CSUL_PN7_MASK					0x007F
#define SIGFOX_EP_ADDON_TA_CSUL_PN7_OFFSET					((SIGFOX_EP_ADDON_TA_CSUL_PN7_MASK - SIGFOX_FH_MICRO_CHANNEL_NUMBER) / 2)

#define SIGFOX_EP_ADDON_TA_CSUL_RANDOM_VALUE_SIZE_BITS		7
#define SIGFOX_EP_ADDON_TA_CSUL_RANDOM_VALUE_MASK			((1 << SIGFOX_EP_ADDON_TA_CSUL_RANDOM_VALUE_SIZE_BITS) - 1)
#define SIGFOX_EP_ADDON_TA_CSUL_RANDOM_VALUE_MAX			((sfx_u32) SIGFOX_EP_ADDON_TA_CSUL_RANDOM_VALUE_MASK)
#define SIGFOX_EP_ADDON_TA_CSUL_RANDOM_MULTIPLIER			5
#define SIGFOX_EP_ADDON_TA_CSUL_RANDOM_OFFSET				31

/*** SIGFOX EP ADDON CSUL local structures ***/

/*******************************************************************/
typedef enum {
	SIGFOX_EP_ADDON_TA_CSUL_STATE_READY = 0,
	SIGFOX_EP_ADDON_TA_CSUL_STATE_UL_MODULATION_PENDING,
	SIGFOX_EP_ADDON_TA_CSUL_STATE_UL_INTER_FRAME_TIMER,
	SIGFOX_EP_ADDON_TA_CSUL_STATE_LAST
} SIGFOX_EP_ADDON_TA_CSUL_state_t;

/*******************************************************************/
typedef union  {
	struct  {
		sfx_u8 synchronous : 1;
		sfx_u8 test_mode_request : 1;
		sfx_u8 process_running : 1;
	} field;
	sfx_u8 all;
} SIGFOX_EP_ADDON_TA_CSUL_internal_flags_t;

/*******************************************************************/
typedef union {
	struct {
		sfx_u8 message_cplt : 1;
		sfx_u8 timer_cplt : 1;
	} field;
	sfx_u8 all;
} SIGFOX_EP_ADDON_TA_CSUL_irq_flags_t;

/*******************************************************************/
typedef struct {
	const SIGFOX_rc_t *rc;
	SIGFOX_EP_ADDON_TA_CSUL_state_t state;
	volatile SIGFOX_EP_ADDON_TA_CSUL_irq_flags_t irq_flags;
	SIGFOX_EP_ADDON_TA_CSUL_internal_flags_t internal_flags;
	sfx_u32 ul_frame_count;
	SIGFOX_EP_ADDON_TA_API_csul_test_mode_t* test_mode_ptr;
#ifdef ASYNCHRONOUS
	SIGFOX_EP_ADDON_TA_API_csul_test_mode_t test_mode;
	SIGFOX_EP_ADDON_TA_API_process_cb_t process_cb;
	SIGFOX_EP_ADDON_TA_API_test_mode_cplt_cb internal_cplt_cb;
	SIGFOX_EP_ADDON_TA_API_progress_status_t progress_status;
#endif
#ifdef SPECTRUM_ACCESS_FH
	sfx_u8 operated_macro_channel_index;
	sfx_u32 first_micro_channel_frequency_hz;
	sfx_u8 micro_channel_index;
	sfx_u8 random_value[SIGFOX_FH_MICRO_CHANNEL_NUMBER];
	sfx_u32 baseband_frequency_hz;
#endif
} SIGFOX_EP_ADDON_TA_CSUL_context_t;

/*** SIGFOX EP ADDON CSUL local global variables ***/

static SIGFOX_EP_ADDON_TA_CSUL_context_t sigfox_ep_addon_ta_csul_ctx;

/*** SIGFOX EP ADDON CSUL local functions ***/

/*******************************************************************/
#define _CHECK_STATE(state_condition) { if (sigfox_ep_addon_ta_csul_ctx.state state_condition) { EXIT_ERROR(SIGFOX_EP_ADDON_TA_API_ERROR_STATE); } }

#ifdef ASYNCHRONOUS
/*******************************************************************/
static void _SIGFOX_EP_API_message_completion_callback(void) {
	// Set flag.
	sigfox_ep_addon_ta_csul_ctx.irq_flags.field.message_cplt = 1;
	// Call process callback.
	if (sigfox_ep_addon_ta_csul_ctx.process_cb != SFX_NULL) {
		sigfox_ep_addon_ta_csul_ctx.process_cb();
	}
}
#endif

#ifdef ASYNCHRONOUS
/*******************************************************************/
static void _MCU_API_timer_completion_callback(void) {
	// Set flag.
	sigfox_ep_addon_ta_csul_ctx.irq_flags.field.timer_cplt = 1;
	// Call process callback.
	if (sigfox_ep_addon_ta_csul_ctx.process_cb != SFX_NULL) {
		sigfox_ep_addon_ta_csul_ctx.process_cb();
	}
}
#endif

#if (defined PARAMETERS_CHECK) && (defined ERROR_CODES)
/*******************************************************************/
static SIGFOX_EP_ADDON_TA_API_status_t _check_test_mode(SIGFOX_EP_ADDON_TA_API_csul_test_mode_t* csul_test_mode) {
	// Local variables.
	SIGFOX_EP_ADDON_TA_API_status_t status = SIGFOX_EP_ADDON_TA_API_SUCCESS;
	// Check test mode.
	if (csul_test_mode == SFX_NULL) {
		EXIT_ERROR(SIGFOX_EP_ADDON_TA_API_ERROR_NULL_PARAMETER);
	}
	if ((csul_test_mode -> number_of_frames) == 0) {
		EXIT_ERROR(SIGFOX_EP_ADDON_TA_API_ERROR_NUMBER_OF_FRAMES);
	}
#ifdef SPECTRUM_ACCESS_FH
	if ((csul_test_mode -> frequency_hopping_mode) >= SIGFOX_EP_ADDON_TA_API_FH_MODE_LAST) {
		EXIT_ERROR(SIGFOX_EP_ADDON_TA_API_ERROR_FREQUENCY_HOPPING_MODE);
	}
#endif
errors:
	RETURN();
}
#endif

/*******************************************************************/
static SIGFOX_EP_ADDON_TA_API_status_t _store_test_mode(SIGFOX_EP_ADDON_TA_API_csul_test_mode_t *csul_test_mode)  {
	// Local variables.
#ifdef ERROR_CODES
	SIGFOX_EP_ADDON_TA_API_status_t status = SIGFOX_EP_ADDON_TA_API_SUCCESS;
#endif
#if (defined PARAMETERS_CHECK) && (defined ERROR_CODES)
	// Check parameters.
	status = _check_test_mode(csul_test_mode);
	CHECK_STATUS(SIGFOX_EP_ADDON_TA_API_SUCCESS);
#endif
#ifdef ASYNCHRONOUS
	// In asynchronous mode, all the data has to be stored locally since the client pointer could be removed.
	sigfox_ep_addon_ta_csul_ctx.test_mode.number_of_frames = (csul_test_mode -> number_of_frames);
	sigfox_ep_addon_ta_csul_ctx.test_mode.tx_frequency_hz = (csul_test_mode -> tx_frequency_hz);
#ifdef SPECTRUM_ACCESS_FH
	sigfox_ep_addon_ta_csul_ctx.test_mode.frequency_hopping_mode = (csul_test_mode -> frequency_hopping_mode);
#endif
#ifndef UL_BIT_RATE_BPS
	sigfox_ep_addon_ta_csul_ctx.test_mode.ul_bit_rate = (csul_test_mode -> ul_bit_rate);
#endif
#ifndef TX_POWER_DBM_EIRP
	sigfox_ep_addon_ta_csul_ctx.test_mode.tx_power_dbm_eirp = (csul_test_mode -> tx_power_dbm_eirp);
#endif
	sigfox_ep_addon_ta_csul_ctx.test_mode.test_mode_cplt_cb = (csul_test_mode -> test_mode_cplt_cb);
	// Update pointers to local data.
	sigfox_ep_addon_ta_csul_ctx.test_mode_ptr = &(sigfox_ep_addon_ta_csul_ctx.test_mode);
#else /* ASYNCHRONOUS */
	// In blocking mode, the test mode pointer will directly address the client data since it will be kept during processing.
	sigfox_ep_addon_ta_csul_ctx.test_mode_ptr = csul_test_mode;
#endif /* ASYNCHRONOUS */
#if (defined PARAMETERS_CHECK) && (defined ERROR_CODES)
errors:
#endif
	RETURN();
}

#ifdef SPECTRUM_ACCESS_FH
/*******************************************************************/
static void _compute_new_random_micro_channel(void) {
	// Local variables.
	sfx_u8 m = 0;
	sfx_u8 n = 0;
	sfx_u16 min = 0;
	sfx_u16 max = 0;
	sfx_u16 shift_reg = (sfx_u16) (sigfox_ep_addon_ta_csul_ctx.micro_channel_index + SIGFOX_EP_ADDON_TA_CSUL_PN7_OFFSET);
	// Compute minimum and maximum values.
	if (((sigfox_ep_addon_ta_csul_ctx.test_mode_ptr) -> frequency_hopping_mode) == SIGFOX_EP_ADDON_TA_API_FH_MODE_ALL_MACRO_CHANNELS) {
		min = SIGFOX_EP_ADDON_TA_CSUL_PN7_OFFSET;
		max = (min + SIGFOX_FH_MICRO_CHANNEL_NUMBER - 1);
	}
	else {
		min = SIGFOX_EP_ADDON_TA_CSUL_PN7_OFFSET + (SIGFOX_FH_MICRO_CHANNEL_PER_MACRO_CHANNEL_OPERATED * sigfox_ep_addon_ta_csul_ctx.operated_macro_channel_index);
		max = (min + SIGFOX_FH_MICRO_CHANNEL_PER_MACRO_CHANNEL_OPERATED - 1);
	}
	// Loop until value is in the range.
	do {
		// Avoid null value.
		if (shift_reg == 0) {
			shift_reg = SIGFOX_EP_ADDON_TA_CSUL_PN7_MASK;
		}
		m = (shift_reg >> (SIGFOX_EP_ADDON_TA_CSUL_PN7_LENGTH - 1)) & 0x01;
		n = (shift_reg >> (SIGFOX_EP_ADDON_TA_CSUL_PN7_TAP - 1)) & 0x01;
		shift_reg = (shift_reg << 1);
		if ((m ^ n) != SFX_FALSE) {
			shift_reg |= 0x1;
		}
		// Apply mask.
		shift_reg &= SIGFOX_EP_ADDON_TA_CSUL_PN7_MASK;
	}
	while ((shift_reg < min) || (shift_reg > max));
	// Update new index.
	sigfox_ep_addon_ta_csul_ctx.micro_channel_index = (sfx_u8) (shift_reg - SIGFOX_EP_ADDON_TA_CSUL_PN7_OFFSET);
}
#endif

#ifdef SPECTRUM_ACCESS_FH
/*******************************************************************/
static void _compute_new_random_baseband_frequency(void) {
	// Local variables.
	sfx_u32 random_value_temp = 0;
	sfx_u32 baseband_frequency_temp = 0;
	// Compute new random value.
	random_value_temp = (sfx_u32) sigfox_ep_addon_ta_csul_ctx.random_value[sigfox_ep_addon_ta_csul_ctx.micro_channel_index];
	random_value_temp = (SIGFOX_EP_ADDON_TA_CSUL_RANDOM_MULTIPLIER * random_value_temp) + SIGFOX_EP_ADDON_TA_CSUL_RANDOM_OFFSET;
	random_value_temp = (random_value_temp & SIGFOX_EP_ADDON_TA_CSUL_RANDOM_VALUE_MASK);
	// Compute baseband frequency.
	baseband_frequency_temp = ((random_value_temp * SIGFOX_FH_MICRO_CHANNEL_OPERATED_WIDTH_HZ) / (SIGFOX_EP_ADDON_TA_CSUL_RANDOM_VALUE_MAX));
	// Update context.
	sigfox_ep_addon_ta_csul_ctx.random_value[sigfox_ep_addon_ta_csul_ctx.micro_channel_index] = (sfx_u8) (random_value_temp);
	sigfox_ep_addon_ta_csul_ctx.baseband_frequency_hz = baseband_frequency_temp;
}
#endif

/*******************************************************************/
static SIGFOX_EP_ADDON_TA_API_status_t _send_uplink_frame(void) {
	// Local variables.
#ifdef ERROR_CODES
	SIGFOX_EP_ADDON_TA_API_status_t status = SIGFOX_EP_ADDON_TA_API_SUCCESS;
	SIGFOX_EP_API_status_t sigfox_ep_api_status = SIGFOX_EP_API_SUCCESS;
#endif
#ifdef APPLICATION_MESSAGES
	SIGFOX_EP_API_application_message_t message;
#else
	SIGFOX_EP_API_control_message_t message;
#endif
	SIGFOX_EP_API_TEST_parameters_t test_params;
#if (defined UL_PAYLOAD_SIZE) && (UL_PAYLOAD_SIZE > 0)
	sfx_u8 ul_payload[UL_PAYLOAD_SIZE] = {0};
#endif
#ifdef SPECTRUM_ACCESS_FH
	sfx_u32 micro_channel_center_frequency_hz = 0;
#endif
	// Set test parameters.
	test_params.flags.all = 0;
	test_params.flags.field.ul_enable = 1;
	test_params.tx_frequency_hz = ((sigfox_ep_addon_ta_csul_ctx.test_mode_ptr) -> tx_frequency_hz);
#ifdef SPECTRUM_ACCESS_FH
	// Overwrite EP library frequency generator when using FH spectrum access.
	if ((((sigfox_ep_addon_ta_csul_ctx.rc) -> spectrum_access -> type) == SIGFOX_SPECTRUM_ACCESS_TYPE_FH) && (((sigfox_ep_addon_ta_csul_ctx.test_mode_ptr) -> tx_frequency_hz) == 0)) {
		// Compute new micro-channel index.
		_compute_new_random_micro_channel();
		_compute_new_random_baseband_frequency();
		// Compute micro-channel center frequency.
		micro_channel_center_frequency_hz = sigfox_ep_addon_ta_csul_ctx.first_micro_channel_frequency_hz;
		micro_channel_center_frequency_hz += (SIGFOX_FH_MICRO_CHANNEL_WIDTH_HZ * sigfox_ep_addon_ta_csul_ctx.micro_channel_index);
		micro_channel_center_frequency_hz += (sigfox_ep_addon_ta_csul_ctx.micro_channel_index / SIGFOX_FH_MICRO_CHANNEL_PER_MACRO_CHANNEL_OPERATED) * SIGFOX_FH_MICRO_CHANNEL_HOP;
		// Compute absolute signal frequency.
		test_params.tx_frequency_hz = (micro_channel_center_frequency_hz - (SIGFOX_FH_MICRO_CHANNEL_OPERATED_WIDTH_HZ / 2) + sigfox_ep_addon_ta_csul_ctx.baseband_frequency_hz);
	}
#endif
#ifdef BIDIRECTIONAL
	test_params.dl_t_rx_ms = 0;
	test_params.dl_t_w_ms = 0;
	test_params.rx_frequency_hz = 0;
#endif
#if (defined REGULATORY) && (defined SPECTRUM_ACCESS_LBT)
	test_params.lbt_cs_max_duration_first_frame_ms = 0;
#endif
	// Set message parameters.
#ifndef UL_BIT_RATE_BPS
	message.common_parameters.ul_bit_rate = ((sigfox_ep_addon_ta_csul_ctx.test_mode_ptr) -> ul_bit_rate);
#endif
#ifndef TX_POWER_DBM_EIRP
	message.common_parameters.tx_power_dbm_eirp = ((sigfox_ep_addon_ta_csul_ctx.test_mode_ptr) -> tx_power_dbm_eirp);
#endif
#ifndef SINGLE_FRAME
	message.common_parameters.number_of_frames = 1;
#ifndef T_IFU_MS
	message.common_parameters.t_ifu_ms = 0;
#endif
#endif
#ifdef PUBLIC_KEY_CAPABLE
	message.common_parameters.ep_key_type = SIGFOX_EP_KEY_PRIVATE;
#endif
#ifdef ASYNCHRONOUS
	message.uplink_cplt_cb = SFX_NULL;
	message.message_cplt_cb = &_SIGFOX_EP_API_message_completion_callback;
#endif
#ifdef APPLICATION_MESSAGES
#if (defined UL_PAYLOAD_SIZE) && (UL_PAYLOAD_SIZE > 0)
	message.type = SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY;
	message.ul_payload = ul_payload;
#else
	message.type = SIGFOX_APPLICATION_MESSAGE_TYPE_EMPTY;
	message.ul_payload = SFX_NULL;
#endif
#ifndef UL_PAYLOAD_SIZE
	message.ul_payload_size_bytes = 0;
#endif
#if (defined ASYNCHRONOUS) && (defined BIDIRECTIONAL)
	message.downlink_cplt_cb = SFX_NULL;
#endif
#ifdef BIDIRECTIONAL
	message.bidirectional_flag = 0;
#ifndef T_CONF_MS
	message.t_conf_ms = 2000;
#endif
#endif
#else /* APPLICATION_MESSAGES */
	message.type = SIGFOX_CONTROL_MESSAGE_TYPE_KEEP_ALIVE;
#endif  /* APPLICATION_MESSAGES */
	// Send message.
#ifdef APPLICATION_MESSAGES
#ifdef ERROR_CODES
	sigfox_ep_api_status = SIGFOX_EP_API_TEST_send_application_message(&message, &test_params);
	SIGFOX_EP_API_check_status(SIGFOX_EP_ADDON_TA_API_ERROR_DRIVER_SIGFOX_EP_API);
#else
	SIGFOX_EP_API_TEST_send_application_message(&message, &test_params);
#endif
#else /* APPLICATION_MESSAGES */
#ifdef ERROR_CODES
	sigfox_ep_api_status = SIGFOX_EP_API_TEST_send_control_message(&message, &test_params);
	SIGFOX_EP_API_check_status(SIGFOX_EP_ADDON_TA_API_ERROR_DRIVER_SIGFOX_EP_API);
#else
	SIGFOX_EP_API_TEST_send_control_message(&message, &test_params);
#endif
#endif  /* APPLICATION_MESSAGES */
#ifdef ERROR_CODES
errors:
#endif
	// Increment messages count.
	sigfox_ep_addon_ta_csul_ctx.ul_frame_count++;
#ifndef ASYNCHRONOUS
	sigfox_ep_addon_ta_csul_ctx.irq_flags.field.message_cplt = 1; // Set flag manually in blocking mode.
#endif
	RETURN();
}

/*******************************************************************/
static SIGFOX_EP_ADDON_TA_API_status_t _start_interframe_timer(void) {
	// Local variables.
#ifdef ERROR_CODES
	SIGFOX_EP_ADDON_TA_API_status_t status = SIGFOX_EP_ADDON_TA_API_SUCCESS;
	MCU_API_status_t mcu_api_status = MCU_API_SUCCESS;
#endif
	MCU_API_timer_t mcu_timer;
	// Configure timer.
	mcu_timer.instance = MCU_API_TIMER_INSTANCE_ADDON_TA;
	mcu_timer.duration_ms = SIGFOX_EP_ADDON_TA_CSUL_INTERFRAME_TIMER_MS;
	mcu_timer.reason = MCU_API_TIMER_REASON_ADDON_TA;
#ifdef ASYNCHRONOUS
	mcu_timer.cplt_cb = &_MCU_API_timer_completion_callback;
#endif
#ifdef ERROR_CODES
	mcu_api_status = MCU_API_timer_start(&mcu_timer);
	MCU_API_check_status(SIGFOX_EP_ADDON_TA_API_ERROR_DRIVER_MCU_API);
#else
	MCU_API_timer_start(&mcu_timer);
#endif
	// Wait for timer completion in case of blocking or synchronous operation.
#ifdef ASYNCHRONOUS
	if (sigfox_ep_addon_ta_csul_ctx.internal_flags.field.synchronous != 0) {
		// Wait flag.
		while (sigfox_ep_addon_ta_csul_ctx.irq_flags.field.timer_cplt == 0);
	}
#else /* ASYNCHRONOUS */
#ifdef ERROR_CODES
	mcu_api_status = MCU_API_timer_wait_cplt(MCU_API_TIMER_INSTANCE_ADDON_TA);
	MCU_API_check_status(SIGFOX_EP_ADDON_TA_API_ERROR_DRIVER_MCU_API);
#else
	MCU_API_timer_wait_cplt(MCU_API_TIMER_INSTANCE_ADDON_TA);
#endif
#endif /* ASYNCHRONOUS */
#ifdef ERROR_CODES
errors:
#endif
#ifndef ASYNCHRONOUS
	sigfox_ep_addon_ta_csul_ctx.irq_flags.field.timer_cplt = 1; // Set flag manually in blocking mode.
#endif
	RETURN();
}

/*******************************************************************/
static SIGFOX_EP_ADDON_TA_API_status_t _internal_process(void) {
	// Local variables.
#ifdef ERROR_CODES
	SIGFOX_EP_ADDON_TA_API_status_t status = SIGFOX_EP_ADDON_TA_API_SUCCESS;
#endif
	// Perform state machine.
	switch (sigfox_ep_addon_ta_csul_ctx.state) {
	case SIGFOX_EP_ADDON_TA_CSUL_STATE_READY:
		// Check pending requests.
		if (sigfox_ep_addon_ta_csul_ctx.internal_flags.field.test_mode_request != 0) {
			// Reset frame count.
			sigfox_ep_addon_ta_csul_ctx.ul_frame_count = 0;
			// Send first frame.
#ifdef ERROR_CODES
			status = _send_uplink_frame();
			CHECK_STATUS(SIGFOX_EP_ADDON_TA_API_SUCCESS);
#else
			_send_uplink_frame();
#endif
			// Clear request.
			sigfox_ep_addon_ta_csul_ctx.internal_flags.field.test_mode_request = 0;
			// Update state.
			sigfox_ep_addon_ta_csul_ctx.state = SIGFOX_EP_ADDON_TA_CSUL_STATE_UL_MODULATION_PENDING;
		}
		break;
	case SIGFOX_EP_ADDON_TA_CSUL_STATE_UL_MODULATION_PENDING:
		// Check completion flag.
		if (sigfox_ep_addon_ta_csul_ctx.irq_flags.field.message_cplt != 0) {
			// Clear flag.
			sigfox_ep_addon_ta_csul_ctx.irq_flags.field.message_cplt = 0;
			// Start inter-frame timer.
#ifdef ERROR_CODES
			status = _start_interframe_timer();
			CHECK_STATUS(SIGFOX_EP_ADDON_TA_API_SUCCESS);
#else
			_start_interframe_timer();
#endif
			// Update state.
			sigfox_ep_addon_ta_csul_ctx.state = SIGFOX_EP_ADDON_TA_CSUL_STATE_UL_INTER_FRAME_TIMER;
		}
		break;
	case SIGFOX_EP_ADDON_TA_CSUL_STATE_UL_INTER_FRAME_TIMER:
		// Check completion flag.
		if (sigfox_ep_addon_ta_csul_ctx.irq_flags.field.timer_cplt != 0) {
			// Clear flag.
			sigfox_ep_addon_ta_csul_ctx.irq_flags.field.timer_cplt = 0;
			// Check frame count.
			if (sigfox_ep_addon_ta_csul_ctx.ul_frame_count >= ((sigfox_ep_addon_ta_csul_ctx.test_mode_ptr) -> number_of_frames)) {
				// Test mode completed.
				sigfox_ep_addon_ta_csul_ctx.state = SIGFOX_EP_ADDON_TA_CSUL_STATE_READY;
#ifdef ASYNCHRONOUS
				// Call internal completion callback.
				if (sigfox_ep_addon_ta_csul_ctx.internal_cplt_cb != SFX_NULL) {
					sigfox_ep_addon_ta_csul_ctx.internal_cplt_cb();
				}
				// Call external completion callback.
				if (((sigfox_ep_addon_ta_csul_ctx.test_mode_ptr) -> test_mode_cplt_cb) != SFX_NULL) {
					(sigfox_ep_addon_ta_csul_ctx.test_mode_ptr) -> test_mode_cplt_cb();
				}
#endif
			}
			else {
				// Send next frame.
#ifdef ERROR_CODES
				status = _send_uplink_frame();
				CHECK_STATUS(SIGFOX_EP_ADDON_TA_API_SUCCESS);
#else
				_send_uplink_frame();
#endif
				// Update state.
				sigfox_ep_addon_ta_csul_ctx.state = SIGFOX_EP_ADDON_TA_CSUL_STATE_UL_MODULATION_PENDING;
			}
		}
		break;
	default:
		EXIT_ERROR(SIGFOX_EP_ADDON_TA_API_ERROR_STATE);
	}
	RETURN();
errors:
	RETURN();
}

/*** SIGFOX EP ADDON CSUL functions ***/

/*******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_CSUL_open(SIGFOX_EP_ADDON_TA_CSUL_config_t* csul_test_config) {
	// Local variables.
#ifdef ERROR_CODES
	SIGFOX_EP_ADDON_TA_API_status_t status = SIGFOX_EP_ADDON_TA_API_SUCCESS;
#endif
#ifdef SPECTRUM_ACCESS_FH
	sfx_u32 initial_random_value = 0;
	sfx_u32 idx = 0;
#endif
#ifdef PARAMETERS_CHECK
	// Check parameter.
	if (csul_test_config == SFX_NULL) {
		EXIT_ERROR(SIGFOX_EP_ADDON_TA_API_ERROR_NULL_PARAMETER);
	}
#endif /* PARAMETERS_CHECK */
	// Store RC pointer.
	sigfox_ep_addon_ta_csul_ctx.rc = (csul_test_config -> rc);
	// Init state and flags.
	sigfox_ep_addon_ta_csul_ctx.state = SIGFOX_EP_ADDON_TA_CSUL_STATE_READY;
	sigfox_ep_addon_ta_csul_ctx.internal_flags.all = 0;
	sigfox_ep_addon_ta_csul_ctx.irq_flags.all = 0;
#ifdef ASYNCHRONOUS
	// Register callbacks.
	sigfox_ep_addon_ta_csul_ctx.process_cb = (csul_test_config -> process_cb);
	sigfox_ep_addon_ta_csul_ctx.internal_cplt_cb = (csul_test_config -> internal_cplt_cb);
	sigfox_ep_addon_ta_csul_ctx.progress_status.all = 0;
	// Update synchronous flag.
	sigfox_ep_addon_ta_csul_ctx.internal_flags.field.synchronous = (sigfox_ep_addon_ta_csul_ctx.process_cb == SFX_NULL) ? 1 : 0;
#endif
#ifdef SPECTRUM_ACCESS_FH
	sigfox_ep_addon_ta_csul_ctx.operated_macro_channel_index = 0;
	sigfox_ep_addon_ta_csul_ctx.first_micro_channel_frequency_hz = 0;
	sigfox_ep_addon_ta_csul_ctx.micro_channel_index = 0;
	for (idx=0 ; idx<SIGFOX_FH_MICRO_CHANNEL_NUMBER ; idx++) {
		// Compute pseudo-random initial value.
		initial_random_value = (SIGFOX_EP_ADDON_TA_CSUL_RANDOM_MULTIPLIER * (idx << 3)) + SIGFOX_EP_ADDON_TA_CSUL_RANDOM_OFFSET;
		initial_random_value = (initial_random_value & SIGFOX_EP_ADDON_TA_CSUL_RANDOM_VALUE_MASK);
		sigfox_ep_addon_ta_csul_ctx.random_value[idx] = (sfx_u8) initial_random_value;
	}
	sigfox_ep_addon_ta_csul_ctx.baseband_frequency_hz = 0;
	// Check RC pointer in case of FH.
	if ((csul_test_config -> rc -> spectrum_access -> type) == SIGFOX_SPECTRUM_ACCESS_TYPE_FH) {
		// Frequency hopping mode only support pre-defined RC2 and RC4 zones.
		if ((csul_test_config -> rc -> f_ul_hz) == SIGFOX_RC2.f_ul_hz) {
			sigfox_ep_addon_ta_csul_ctx.operated_macro_channel_index = SIGFOX_FH_RC2_OPERATED_MACRO_CHANNEL_INDEX;
			sigfox_ep_addon_ta_csul_ctx.first_micro_channel_frequency_hz = SIGFOX_FH_RC2_FIRST_MICRO_CHANNEL_FREQUENCY_HZ;
		}
		else if ((csul_test_config -> rc -> f_ul_hz) == SIGFOX_RC4.f_ul_hz) {
			sigfox_ep_addon_ta_csul_ctx.operated_macro_channel_index = SIGFOX_FH_RC4_OPERATED_MACRO_CHANNEL_INDEX;
			sigfox_ep_addon_ta_csul_ctx.first_micro_channel_frequency_hz = SIGFOX_FH_RC4_FIRST_MICRO_CHANNEL_FREQUENCY_HZ;
		}
#ifdef PARAMETERS_CHECK
		else {
			EXIT_ERROR(SIGFOX_EP_ADDON_TA_API_ERROR_FREQUENCY_HOPPING_RC);
		}
#endif
	}
#endif /* SPECTRUM_ACCESS_FH */
#ifdef PARAMETERS_CHECK
errors:
#endif
	RETURN();
}

/*******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_CSUL_execute_test(SIGFOX_EP_ADDON_TA_API_csul_test_mode_t* csul_test_mode) {
	// Local variables.
#ifdef ERROR_CODES
	SIGFOX_EP_ADDON_TA_API_status_t status = SIGFOX_EP_ADDON_TA_API_SUCCESS;
#endif
	// Check state.
	_CHECK_STATE(!= SIGFOX_EP_ADDON_TA_CSUL_STATE_READY);
	// Reset IRQ flags.
	sigfox_ep_addon_ta_csul_ctx.irq_flags.all = 0;
#ifdef ASYNCHRONOUS
	// Reset progress status.
	sigfox_ep_addon_ta_csul_ctx.progress_status.all = 0;
#endif
	// Store test parameters locally.
#ifdef ERROR_CODES
	status = _store_test_mode(csul_test_mode);
	CHECK_STATUS(SIGFOX_EP_ADDON_TA_API_SUCCESS);
#else
	_store_test_mode(csul_test_mode);
#endif
	// Set internal request flag.
	sigfox_ep_addon_ta_csul_ctx.internal_flags.field.test_mode_request = 1;
	// Trigger first frame.
#ifdef ERROR_CODES
	status = _internal_process();
	CHECK_STATUS(SIGFOX_EP_ADDON_TA_API_SUCCESS);
#else
	_internal_process();
#endif
#ifdef ASYNCHRONOUS
	if (sigfox_ep_addon_ta_csul_ctx.internal_flags.field.synchronous != 0) {
#endif
	// Block until library goes back to READY state.
	while (sigfox_ep_addon_ta_csul_ctx.state != SIGFOX_EP_ADDON_TA_CSUL_STATE_READY) {
#ifdef ERROR_CODES
		status = _internal_process();
		CHECK_STATUS(SIGFOX_EP_ADDON_TA_API_SUCCESS);
#else
		_internal_process();
#endif
	}
#ifdef ASYNCHRONOUS
	}
	RETURN();
#endif
errors:
#ifdef ASYNCHRONOUS
	sigfox_ep_addon_ta_csul_ctx.progress_status.error = 1;
#endif
	RETURN();
}

#ifdef ASYNCHRONOUS
/*******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_CSUL_process(void) {
	// Local variables.
#ifdef ERROR_CODES
	SIGFOX_EP_ADDON_TA_API_status_t status = SIGFOX_EP_ADDON_TA_API_SUCCESS;
#endif
	// Set process flag.
	sigfox_ep_addon_ta_csul_ctx.internal_flags.field.process_running = 1;
	// Run the internal process.
	while (sigfox_ep_addon_ta_csul_ctx.irq_flags.all != 0) {
#ifdef ERROR_CODES
		status = _internal_process();
		CHECK_STATUS(SIGFOX_EP_ADDON_TA_API_SUCCESS);
#else
		_internal_process();
#endif
	}
#ifdef ASYNCHRONOUS
	// Compute progress status.
	sigfox_ep_addon_ta_csul_ctx.progress_status.progress = ((sigfox_ep_addon_ta_csul_ctx.ul_frame_count * 100) / ((sigfox_ep_addon_ta_csul_ctx.test_mode_ptr) -> number_of_frames));
	// Reset process flag.
	sigfox_ep_addon_ta_csul_ctx.internal_flags.field.process_running = 0;
	RETURN();
#endif
#ifdef ERROR_CODES
errors:
#endif
	// Reset process flag.
	sigfox_ep_addon_ta_csul_ctx.internal_flags.field.process_running = 0;
#ifdef ASYNCHRONOUS
	sigfox_ep_addon_ta_csul_ctx.progress_status.error = 1;
#endif
	RETURN();
}
#endif

#ifdef ASYNCHRONOUS
/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_TA_API_progress_status_t SIGFOX_EP_ADDON_TA_CSUL_get_progress_status(void)
 * \brief Get the progress status.
 * \param[in]   none
 * \param[out]  none
 * \retval      Progress status.
 *******************************************************************/
SIGFOX_EP_ADDON_TA_API_progress_status_t SIGFOX_EP_ADDON_TA_CSUL_get_progress_status(void) {
	return (sigfox_ep_addon_ta_csul_ctx.progress_status);
}
#endif
