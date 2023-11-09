/*!*****************************************************************
 * \file    sigfox_ep_addon_ta_api.c
 * \brief   Sigfox End-Point type approval add-on API.
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

#include "sigfox_ep_addon_ta_api.h"

#ifdef USE_SIGFOX_EP_FLAGS_H
#include "sigfox_ep_flags.h"
#endif
#include "manuf/rf_api.h"
#include "test_modes_ta/sigfox_ep_addon_ta_cw.h"
#include "test_modes_ta/sigfox_ep_addon_ta_csul.h"
#if (defined APPLICATION_MESSAGES) && (defined BIDIRECTIONAL)
#include "test_modes_ta/sigfox_ep_addon_ta_csdl.h"
#endif
#include "sigfox_ep_addon_ta_version.h"
#include "sigfox_ep_api.h"
#include "sigfox_types.h"
#include "sigfox_error.h"

#ifdef CERTIFICATION

/*** SIGFOX EP ADDON TA API local structures ***/

/*******************************************************************/
typedef enum {
	SIGFOX_EP_ADDON_TA_API_TEST_MODE_CW = 0,
	SIGFOX_EP_ADDON_TA_API_TEST_MODE_CSUL,
#if (defined APPLICATION_MESSAGES) && (defined BIDIRECTIONAL)
	SIGFOX_EP_ADDON_TA_API_TEST_MODE_CSDL,
#endif
	SIGFOX_EP_ADDON_TA_API_TEST_MODE_LAST
} SIGFOX_EP_API_test_mode_reference_t;

/*******************************************************************/
typedef union {
	struct {
		sfx_u8 synchronous : 1;
		sfx_u8 process_running : 1;
	};
	sfx_u8 all;
} SIGFOX_EP_API_internal_flags_t;

/*******************************************************************/
typedef union {
	struct {
		sfx_u8 sigfox_ep_api_process : 1;
		sfx_u8 test_mode_process : 1;
	};
	sfx_u8 all;
} SIGFOX_EP_API_irq_flags_t;

#ifdef ASYNCHRONOUS
/*******************************************************************/
typedef SIGFOX_EP_ADDON_TA_API_status_t (*SIGFOX_EP_ADDON_TA_API_test_mode_process_function_t)(void);
typedef SIGFOX_EP_ADDON_TA_API_progress_status_t (*SIGFOX_EP_ADDON_TA_API_get_progress_status_function_t)(void);
#endif

/*******************************************************************/
typedef struct {
	const SIGFOX_rc_t *rc_ptr;
	SIGFOX_EP_ADDON_TA_API_state_t state;
	SIGFOX_EP_API_internal_flags_t internal_flags;
	volatile SIGFOX_EP_API_irq_flags_t irq_flags;
#ifdef ASYNCHRONOUS
	SIGFOX_EP_ADDON_TA_API_process_cb_t process_cb;
	SIGFOX_EP_ADDON_TA_API_test_mode_process_function_t test_mode_process_function;
	SIGFOX_EP_ADDON_TA_API_get_progress_status_function_t get_progress_status_function;
#endif
} SIGFOX_EP_ADDON_TA_API_context_t;

/*** SIGFOX EP ADDON TA API local global variables ***/

static SIGFOX_EP_ADDON_TA_API_context_t sigfox_ep_addon_ta_api_ctx = {
	.rc_ptr = SFX_NULL,
	.state = SIGFOX_EP_ADDON_TA_API_STATE_CLOSED,
	.internal_flags.all = 0,
	.irq_flags.all = 0,
#ifdef ASYNCHRONOUS
	.process_cb = SFX_NULL,
	.test_mode_process_function = SFX_NULL,
	.get_progress_status_function = SFX_NULL,
#endif
};

/*** SIGFOX EP ADOON TA API local functions ***/

/*******************************************************************/
#define _CHECK_ADDON_STATE(state_condition) { if (sigfox_ep_addon_ta_api_ctx.state state_condition) { EXIT_ERROR(SIGFOX_EP_ADDON_TA_API_ERROR_STATE) } }

#ifdef ASYNCHRONOUS
/*******************************************************************/
#define _PROCESS_CALLBACK(void) { \
	if ((sigfox_ep_addon_ta_api_ctx.process_cb != SFX_NULL) && (sigfox_ep_addon_ta_api_ctx.internal_flags.process_running == 0)) { \
		sigfox_ep_addon_ta_api_ctx.process_cb(); \
	} \
}
#endif

#ifdef ASYNCHRONOUS
/*******************************************************************/
static void _SIGFOX_EP_API_process_callback(void) {
    sigfox_ep_addon_ta_api_ctx.irq_flags.sigfox_ep_api_process = 1;
    _PROCESS_CALLBACK();
}
#endif

#ifdef ASYNCHRONOUS
/*******************************************************************/
static void _SIGFOX_EP_ADDON_TA_test_mode_process_callback(void) {
    sigfox_ep_addon_ta_api_ctx.irq_flags.test_mode_process = 1;
    _PROCESS_CALLBACK();
}
#endif

#ifdef ASYNCHRONOUS
/*******************************************************************/
static void _SIGFOX_EP_ADDON_TA_test_mode_completion_callback(void) {
	// Update state.
	sigfox_ep_addon_ta_api_ctx.state = SIGFOX_EP_ADDON_TA_API_STATE_READY;
}
#endif

#ifdef ASYNCHRONOUS
/*******************************************************************/
static SIGFOX_EP_ADDON_TA_API_status_t _internal_process(void) {
	// Local variables.
#ifdef ERROR_CODES
	SIGFOX_EP_ADDON_TA_API_status_t status = SIGFOX_EP_ADDON_TA_API_SUCCESS;
	SIGFOX_EP_API_status_t sigfox_ep_api_status = SIGFOX_EP_API_SUCCESS;
#endif
	// Check addon is opened.
	_CHECK_ADDON_STATE(== SIGFOX_EP_ADDON_TA_API_STATE_CLOSED);
	// Check low level process flags.
	if (sigfox_ep_addon_ta_api_ctx.irq_flags.sigfox_ep_api_process != 0) {
		// Clear flag.
		sigfox_ep_addon_ta_api_ctx.irq_flags.sigfox_ep_api_process = 0;
		// Process EP library.
#ifdef ERROR_CODES
		sigfox_ep_api_status = SIGFOX_EP_API_process();
		SIGFOX_EP_API_check_status(SIGFOX_EP_ADDON_TA_API_ERROR_DRIVER_SIGFOX_EP_API);
#else
		SIGFOX_EP_API_process();
#endif
	}
	// Check process flag.
	if (sigfox_ep_addon_ta_api_ctx.irq_flags.test_mode_process != 0) {
		// Clear flag.
		sigfox_ep_addon_ta_api_ctx.irq_flags.test_mode_process = 0;
		// Call process function.
		if (sigfox_ep_addon_ta_api_ctx.test_mode_process_function != SFX_NULL) {
#ifdef ERROR_CODES
			status = sigfox_ep_addon_ta_api_ctx.test_mode_process_function();
			CHECK_STATUS(SIGFOX_EP_ADDON_TA_API_SUCCESS);
#else
			sigfox_ep_addon_ta_api_ctx.test_mode_process_function();
#endif
		}
	}
errors:
	RETURN();
}
#endif

/*** SIGFOX EP ADDON TA API functions ***/

/*******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_API_open(SIGFOX_EP_ADDON_TA_API_config_t *config) {
	// Local variables.
#ifdef ERROR_CODES
	SIGFOX_EP_ADDON_TA_API_status_t status = SIGFOX_EP_ADDON_TA_API_SUCCESS;
	SIGFOX_EP_API_status_t sigfox_ep_api_status = SIGFOX_EP_API_SUCCESS;
#endif
	SIGFOX_EP_API_config_t ep_api_config;
	SIGFOX_EP_ADDON_TA_CW_config_t cw_config;
	SIGFOX_EP_ADDON_TA_CSUL_config_t csul_config;
#if (defined APPLICATION_MESSAGES) && (defined BIDIRECTIONAL)
	SIGFOX_EP_ADDON_TA_CSDL_config_t csdl_config;
#endif
#ifdef PARAMETERS_CHECK
	// Check parameters.
	if (config == SFX_NULL) {
		EXIT_ERROR(SIGFOX_EP_ADDON_TA_API_ERROR_NULL_PARAMETER);
	}
	if ((config -> rc) == SFX_NULL) {
		EXIT_ERROR(SIGFOX_EP_ADDON_TA_API_ERROR_NULL_PARAMETER);
	}
#endif /* PARAMETERS_CHECK */
	// Check state.
	_CHECK_ADDON_STATE(!= SIGFOX_EP_ADDON_TA_API_STATE_CLOSED);
	// Store parameters locally.
	sigfox_ep_addon_ta_api_ctx.rc_ptr = (config -> rc);
#ifdef ASYNCHRONOUS
	sigfox_ep_addon_ta_api_ctx.process_cb = (config -> process_cb);
	// Update synchronous flag.
	sigfox_ep_addon_ta_api_ctx.internal_flags.synchronous = (sigfox_ep_addon_ta_api_ctx.process_cb == SFX_NULL) ? 1 : 0;
#endif /* ASYNCHRONOUS */
	// Open CW test modes driver.
	cw_config.rc = (config -> rc);
#ifdef ERROR_CODES
	status = SIGFOX_EP_ADDON_TA_CW_open(&cw_config);
	CHECK_STATUS(SIGFOX_EP_ADDON_TA_API_SUCCESS);
#else
	SIGFOX_EP_ADDON_TA_CW_open(&cw_config);
#endif
	// Open CSUL test mode driver.
	csul_config.rc = (config -> rc);
#ifdef ASYNCHRONOUS
	csul_config.process_cb = (sigfox_ep_addon_ta_api_ctx.internal_flags.synchronous == 0) ? &_SIGFOX_EP_ADDON_TA_test_mode_process_callback : SFX_NULL;
	csul_config.internal_cplt_cb = &_SIGFOX_EP_ADDON_TA_test_mode_completion_callback;
#endif
#ifdef ERROR_CODES
	status = SIGFOX_EP_ADDON_TA_CSUL_open(&csul_config);
	CHECK_STATUS(SIGFOX_EP_ADDON_TA_API_SUCCESS);
#else
	SIGFOX_EP_ADDON_TA_CSUL_open(&csul_config);
#endif
#if (defined APPLICATION_MESSAGES) && (defined BIDIRECTIONAL)
	// Open CSDL test mode driver.
	csdl_config.rc = (config -> rc);
#ifdef ASYNCHRONOUS
	csdl_config.process_cb = (sigfox_ep_addon_ta_api_ctx.internal_flags.synchronous == 0) ? &_SIGFOX_EP_ADDON_TA_test_mode_process_callback : SFX_NULL;
	csdl_config.internal_cplt_cb = &_SIGFOX_EP_ADDON_TA_test_mode_completion_callback;
#endif
#ifdef ERROR_CODES
	status = SIGFOX_EP_ADDON_TA_CSDL_open(&csdl_config);
	CHECK_STATUS(SIGFOX_EP_ADDON_TA_API_SUCCESS);
#else
	SIGFOX_EP_ADDON_TA_CSDL_open(&csdl_config);
#endif
#endif
	// Configure library.
	ep_api_config.rc = (config -> rc);
#ifdef ASYNCHRONOUS
	ep_api_config.process_cb = (sigfox_ep_addon_ta_api_ctx.internal_flags.synchronous == 0) ? &_SIGFOX_EP_API_process_callback : SFX_NULL;
#endif
#ifndef MESSAGE_COUNTER_ROLLOVER
	ep_api_config.message_counter_rollover = SIGFOX_MESSAGE_COUNTER_ROLLOVER_4096;
#endif
	// Open library.
#ifdef ERROR_CODES
	sigfox_ep_api_status = SIGFOX_EP_API_open(&ep_api_config);
	SIGFOX_EP_API_check_status(SIGFOX_EP_ADDON_TA_API_ERROR_DRIVER_SIGFOX_EP_API);
#else
	SIGFOX_EP_API_open(&ep_api_config);
#endif
	// Update addon state if no error occurred.
	sigfox_ep_addon_ta_api_ctx.state = SIGFOX_EP_ADDON_TA_API_STATE_READY;
errors:
	RETURN();
}

/*******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_API_close(void) {
	// Local variables.
#ifdef ERROR_CODES
	SIGFOX_EP_ADDON_TA_API_status_t status = SIGFOX_EP_ADDON_TA_API_SUCCESS;
	SIGFOX_EP_API_status_t sigfox_ep_api_status = SIGFOX_EP_API_SUCCESS;
#endif
	// Check addon state.
	_CHECK_ADDON_STATE(!= SIGFOX_EP_ADDON_TA_API_STATE_READY);
	// Close library.
#ifdef ERROR_CODES
	sigfox_ep_api_status = SIGFOX_EP_API_close();
	SIGFOX_EP_API_check_status(SIGFOX_EP_ADDON_TA_API_ERROR_DRIVER_SIGFOX_EP_API);
#else
	SIGFOX_EP_API_close();
#endif
	// Update library state if no error occurred.
	sigfox_ep_addon_ta_api_ctx.state = SIGFOX_EP_ADDON_TA_API_STATE_CLOSED;
errors:
	RETURN();
}

/*******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_API_start_continuous_wave(SIGFOX_EP_ADDON_TA_API_cw_test_mode_t *cw_test_mode) {
	// Local variables.
#ifdef ERROR_CODES
	SIGFOX_EP_ADDON_TA_API_status_t status = SIGFOX_EP_ADDON_TA_API_SUCCESS;
#endif
	// Check state.
	_CHECK_ADDON_STATE(!= SIGFOX_EP_ADDON_TA_API_STATE_READY);
#ifdef ASYNCHRONOUS
	// Update context for asynchronous operation.
	sigfox_ep_addon_ta_api_ctx.test_mode_process_function = SFX_NULL;
	sigfox_ep_addon_ta_api_ctx.get_progress_status_function = (SIGFOX_EP_ADDON_TA_API_get_progress_status_function_t) &SIGFOX_EP_ADDON_TA_CW_get_progress_status;
#endif
	// Update state.
	sigfox_ep_addon_ta_api_ctx.state = SIGFOX_EP_ADDON_TA_API_STATE_RUNNING;
	// Start CW.
#ifdef ERROR_CODES
	status = SIGFOX_EP_ADDON_TA_CW_start(cw_test_mode);
	CHECK_STATUS(SIGFOX_EP_ADDON_TA_API_SUCCESS);
#else
	SIGFOX_EP_ADDON_TA_CW_start(cw_test_mode);
#endif
errors:
	RETURN();
}

/*******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_API_stop_continuous_wave(void) {
	// Local variables.
#ifdef ERROR_CODES
	SIGFOX_EP_ADDON_TA_API_status_t status = SIGFOX_EP_ADDON_TA_API_SUCCESS;
#endif
	// Check state.
	_CHECK_ADDON_STATE(!= SIGFOX_EP_ADDON_TA_API_STATE_RUNNING);
	// Start CW.
#ifdef ERROR_CODES
	status = SIGFOX_EP_ADDON_TA_CW_stop();
	CHECK_STATUS(SIGFOX_EP_ADDON_TA_API_SUCCESS);
#else
	SIGFOX_EP_ADDON_TA_CW_stop();
#endif
errors:
	// Update state.
	sigfox_ep_addon_ta_api_ctx.state = SIGFOX_EP_ADDON_TA_API_STATE_READY;
	RETURN();
}

/*******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_API_execute_continuous_sigfox_uplink(SIGFOX_EP_ADDON_TA_API_csul_test_mode_t *csul_test_mode) {
	// Local variables.
#ifdef ERROR_CODES
	SIGFOX_EP_ADDON_TA_API_status_t status = SIGFOX_EP_ADDON_TA_API_SUCCESS;
#endif
	// Check addon state.
	_CHECK_ADDON_STATE(!= SIGFOX_EP_ADDON_TA_API_STATE_READY);
#ifdef ASYNCHRONOUS
	// Update context for asynchronous operation.
	sigfox_ep_addon_ta_api_ctx.test_mode_process_function = (SIGFOX_EP_ADDON_TA_API_test_mode_process_function_t) &SIGFOX_EP_ADDON_TA_CSUL_process;
	sigfox_ep_addon_ta_api_ctx.get_progress_status_function = (SIGFOX_EP_ADDON_TA_API_get_progress_status_function_t) &SIGFOX_EP_ADDON_TA_CSUL_get_progress_status;
	sigfox_ep_addon_ta_api_ctx.irq_flags.all = 0;
#endif
	// Update state.
	sigfox_ep_addon_ta_api_ctx.state = SIGFOX_EP_ADDON_TA_API_STATE_RUNNING;
#ifdef ERROR_CODES
	status = SIGFOX_EP_ADDON_TA_CSUL_execute_test(csul_test_mode);
	CHECK_STATUS(SIGFOX_EP_ADDON_TA_API_SUCCESS);
#else
	SIGFOX_EP_ADDON_TA_CSUL_execute_test(csul_test_mode);
#endif
#ifndef ASYNCHRONOUS
	// Update state.
	sigfox_ep_addon_ta_api_ctx.state = SIGFOX_EP_ADDON_TA_API_STATE_READY;
#endif
	RETURN();
errors:
	// Force state to ready after error.
	sigfox_ep_addon_ta_api_ctx.state = SIGFOX_EP_ADDON_TA_API_STATE_READY;
	RETURN();
}

#if (defined APPLICATION_MESSAGES) && (defined BIDIRECTIONAL)
/*******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_API_execute_continuous_sigfox_downlink(SIGFOX_EP_ADDON_TA_API_csdl_test_mode_t *csdl_test_mode) {
	// Local variables.
#ifdef ERROR_CODES
	SIGFOX_EP_ADDON_TA_API_status_t status = SIGFOX_EP_ADDON_TA_API_SUCCESS;
#endif
	// Check addon state.
	_CHECK_ADDON_STATE(!= SIGFOX_EP_ADDON_TA_API_STATE_READY);
#ifdef ASYNCHRONOUS
	// Update context for asynchronous operation.
	sigfox_ep_addon_ta_api_ctx.test_mode_process_function = (SIGFOX_EP_ADDON_TA_API_test_mode_process_function_t) &SIGFOX_EP_ADDON_TA_CSDL_process;
	sigfox_ep_addon_ta_api_ctx.get_progress_status_function = (SIGFOX_EP_ADDON_TA_API_get_progress_status_function_t) &SIGFOX_EP_ADDON_TA_CSDL_get_progress_status;
	sigfox_ep_addon_ta_api_ctx.irq_flags.all = 0;
#endif
	// Update state.
	sigfox_ep_addon_ta_api_ctx.state = SIGFOX_EP_ADDON_TA_API_STATE_RUNNING;
#ifdef ERROR_CODES
	status = SIGFOX_EP_ADDON_TA_CSDL_execute_test(csdl_test_mode);
	CHECK_STATUS(SIGFOX_EP_ADDON_TA_API_SUCCESS);
#else
	SIGFOX_EP_ADDON_TA_CSDL_execute_test(csdl_test_mode);
#endif
#ifndef ASYNCHRONOUS
	// Update state.
	sigfox_ep_addon_ta_api_ctx.state = SIGFOX_EP_ADDON_TA_API_STATE_READY;
#endif
	RETURN();
errors:
	// Force state to ready after error.
	sigfox_ep_addon_ta_api_ctx.state = SIGFOX_EP_ADDON_TA_API_STATE_READY;
	RETURN();
}
#endif

/*******************************************************************/
SIGFOX_EP_ADDON_TA_API_state_t SIGFOX_EP_ADDON_TA_API_get_state(void) {
	return sigfox_ep_addon_ta_api_ctx.state;
}

#ifdef ASYNCHRONOUS
/*******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_API_process(void) {
#ifdef ERROR_CODES
	// Local variables.
	SIGFOX_EP_ADDON_TA_API_status_t status = SIGFOX_EP_ADDON_TA_API_SUCCESS;
#endif
	// Set process flag.
	sigfox_ep_addon_ta_api_ctx.internal_flags.process_running = 1;
	// Run the internal process.
	while (sigfox_ep_addon_ta_api_ctx.irq_flags.all != 0) {
#ifdef ERROR_CODES
		status = _internal_process();
		CHECK_STATUS(SIGFOX_EP_ADDON_TA_API_SUCCESS);
#else
		_internal_process();
#endif
	}
#ifdef ERROR_CODES
errors:
#endif
	// Reset process flag.
	sigfox_ep_addon_ta_api_ctx.internal_flags.process_running = 0;
	RETURN();
}
#endif

#ifdef ASYNCHRONOUS
/*******************************************************************/
SIGFOX_EP_ADDON_TA_API_progress_status_t SIGFOX_EP_ADDON_TA_API_get_progress_status(void) {
	// Local variables.
	SIGFOX_EP_ADDON_TA_API_progress_status_t progress_status;
	// Reset status.
	progress_status.all = 0;
	// Check function pointer.
	if (sigfox_ep_addon_ta_api_ctx.get_progress_status_function != SFX_NULL) {
		progress_status = sigfox_ep_addon_ta_api_ctx.get_progress_status_function();
	}
	return progress_status;
}
#endif

#ifdef VERBOSE
/*******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_API_get_version(sfx_u8 **version, sfx_u8 *version_size_char) {
	// Local variables.
#ifdef ERROR_CODES
	SIGFOX_EP_ADDON_TA_API_status_t status = SIGFOX_EP_ADDON_TA_API_SUCCESS;
#endif
#ifdef PARAMETERS_CHECK
	if ((version == SFX_NULL) || (version_size_char == SFX_NULL)) {
		EXIT_ERROR(SIGFOX_EP_API_ERROR_NULL_PARAMETER);
	}
#endif
	// Return version.
	(*version) = (sfx_u8*) SIGFOX_EP_ADDON_TA_VERSION;
	(*version_size_char) = (sfx_u8) sizeof(SIGFOX_EP_ADDON_TA_VERSION);
#ifdef PARAMETERS_CHECK
errors:
#endif
	RETURN();
}
#endif

#endif /* CERTIFICATION */
