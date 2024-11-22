/*!*****************************************************************
 * \file    sigfox_ep_addon_ta_csdl.c
 * \brief   Sigfox End-Point type approval add-on continuous Sigfox downlink test.
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

#include "test_modes_ta/sigfox_ep_addon_ta_csdl.h"

#ifndef SIGFOX_EP_DISABLE_FLAGS_FILE
#include "sigfox_ep_flags.h"
#endif
#include "manuf/mcu_api.h"
#include "sigfox_ep_addon_ta_api.h"
#include "sigfox_types.h"
#include "sigfox_error.h"
#include "sigfox_ep_api_test.h"

#ifdef SIGFOX_EP_BIDIRECTIONAL

/*** SIGFOX EP ADDON CSDL local structures ***/

/*******************************************************************/
typedef enum {
    SIGFOX_EP_ADDON_TA_CSDL_STATE_READY = 0,
    SIGFOX_EP_ADDON_TA_CSDL_STATE_DL_LISTENING,
    SIGFOX_EP_ADDON_TA_CSDL_STATE_LAST
} SIGFOX_EP_ADDON_TA_CSDL_state_t;

/*******************************************************************/
typedef union {
    struct {
        sfx_u8 synchronous :1;
        sfx_u8 test_mode_request :1;
        sfx_u8 process_running :1;
    } field;
    sfx_u8 all;
} SIGFOX_EP_ADDON_TA_CSDL_internal_flags_t;

/*******************************************************************/
typedef union {
    struct {
        sfx_u8 message_cplt :1;
    } field;
    sfx_u8 all;
} SIGFOX_EP_ADDON_TA_CSDL_irq_flags_t;

/*******************************************************************/
typedef struct {
    const SIGFOX_rc_t *rc;
    SIGFOX_EP_ADDON_TA_CSDL_state_t state;
    volatile SIGFOX_EP_ADDON_TA_CSDL_irq_flags_t irq_flags;
    SIGFOX_EP_ADDON_TA_CSDL_internal_flags_t internal_flags;
    SIGFOX_EP_ADDON_TA_API_csdl_test_mode_t *test_mode_ptr;
#ifdef SIGFOX_EP_ASYNCHRONOUS
    SIGFOX_EP_ADDON_TA_API_csdl_test_mode_t test_mode;
    SIGFOX_EP_ADDON_TA_API_process_cb_t process_cb;
    SIGFOX_EP_ADDON_TA_API_test_mode_cplt_cb_t internal_cplt_cb;
    SIGFOX_EP_ADDON_TA_API_progress_status_t progress_status; // Set to 100% as soon as 1 downlink frame is successfully received.
#endif
} SIGFOX_EP_ADDON_TA_CSDL_context_t;

/*** SIGFOX EP ADDON CSDL local global variables ***/

static SIGFOX_EP_ADDON_TA_CSDL_context_t sigfox_ep_addon_ta_csdl_ctx;

/*** SIGFOX EP ADDON CSDL local functions ***/

/*******************************************************************/
#define _CHECK_STATE(state_condition) { if (sigfox_ep_addon_ta_csdl_ctx.state state_condition) { SIGFOX_EXIT_ERROR(SIGFOX_EP_ADDON_TA_API_ERROR_STATE); } }

#ifdef SIGFOX_EP_ASYNCHRONOUS
/*******************************************************************/
static void _SIGFOX_EP_API_message_completion_callback(void) {
    // Set flag.
    sigfox_ep_addon_ta_csdl_ctx.irq_flags.field.message_cplt = 1;
    // Call process callback.
    if (sigfox_ep_addon_ta_csdl_ctx.process_cb != SIGFOX_NULL) {
        sigfox_ep_addon_ta_csdl_ctx.process_cb();
    }
}
#endif

#if (defined SIGFOX_EP_PARAMETERS_CHECK) && (defined SIGFOX_EP_ERROR_CODES)
/*******************************************************************/
static SIGFOX_EP_ADDON_TA_API_status_t _check_test_mode(SIGFOX_EP_ADDON_TA_API_csdl_test_mode_t *csdl_test_mode) {
    // Local variables.
    SIGFOX_EP_ADDON_TA_API_status_t status = SIGFOX_EP_ADDON_TA_API_SUCCESS;
    // Check test mode.
    if (csdl_test_mode == SIGFOX_NULL) {
        SIGFOX_EXIT_ERROR(SIGFOX_EP_ADDON_TA_API_ERROR_NULL_PARAMETER);
    }
errors:
    SIGFOX_RETURN();
}
#endif

/*******************************************************************/
static SIGFOX_EP_ADDON_TA_API_status_t _store_test_mode(SIGFOX_EP_ADDON_TA_API_csdl_test_mode_t *csdl_test_mode) {
    // Local variables.
#ifdef SIGFOX_EP_ERROR_CODES
    SIGFOX_EP_ADDON_TA_API_status_t status = SIGFOX_EP_ADDON_TA_API_SUCCESS;
#endif
#if (defined SIGFOX_EP_PARAMETERS_CHECK) && (defined SIGFOX_EP_ERROR_CODES)
    // Check parameters.
    status = _check_test_mode(csdl_test_mode);
    SIGFOX_CHECK_STATUS(SIGFOX_EP_ADDON_TA_API_SUCCESS);
#endif
#ifdef SIGFOX_EP_ASYNCHRONOUS
    // In asynchronous mode, all the data has to be stored locally since the client pointer could be removed.
    sigfox_ep_addon_ta_csdl_ctx.test_mode.rx_frequency_hz = (csdl_test_mode->rx_frequency_hz);
    sigfox_ep_addon_ta_csdl_ctx.test_mode.dl_t_rx_ms = (csdl_test_mode->dl_t_rx_ms);
    sigfox_ep_addon_ta_csdl_ctx.test_mode.downlink_cplt_cb = (csdl_test_mode->downlink_cplt_cb);
    sigfox_ep_addon_ta_csdl_ctx.test_mode.test_mode_cplt_cb = (csdl_test_mode->test_mode_cplt_cb);
    // Update pointers to local data.
    sigfox_ep_addon_ta_csdl_ctx.test_mode_ptr = &(sigfox_ep_addon_ta_csdl_ctx.test_mode);
#else /* SIGFOX_EP_ASYNCHRONOUS */
    // In blocking mode, the test mode pointer will directly address the client data since it will be kept during processing.
    sigfox_ep_addon_ta_csdl_ctx.test_mode_ptr = csdl_test_mode;
#endif /* SIGFOX_EP_ASYNCHRONOUS */
#if (defined SIGFOX_EP_PARAMETERS_CHECK) && (defined SIGFOX_EP_ERROR_CODES)
errors:
#endif
    SIGFOX_RETURN();
}

/*******************************************************************/
static SIGFOX_EP_ADDON_TA_API_status_t _start_downlink_window(void) {
    // Local variables.
#ifdef SIGFOX_EP_ERROR_CODES
    SIGFOX_EP_ADDON_TA_API_status_t status = SIGFOX_EP_ADDON_TA_API_SUCCESS;
    SIGFOX_EP_API_status_t sigfox_ep_api_status = SIGFOX_EP_API_SUCCESS;
#endif
    SIGFOX_EP_API_application_message_t message;
    SIGFOX_EP_API_TEST_parameters_t test_params;
#if (defined SIGFOX_EP_APPLICATION_MESSAGES) && (defined SIGFOX_EP_UL_PAYLOAD_SIZE) && (SIGFOX_EP_UL_PAYLOAD_SIZE > 0)
    sfx_u8 ul_payload[SIGFOX_EP_UL_PAYLOAD_SIZE] = {0};
#endif
    // Set test parameters.
    test_params.flags.all = 0;
    test_params.flags.field.dl_enable = 1;
    test_params.tx_frequency_hz = 0;
    test_params.dl_t_w_ms = 0;
    test_params.dl_t_rx_ms = ((sigfox_ep_addon_ta_csdl_ctx.test_mode_ptr)->dl_t_rx_ms);
    test_params.rx_frequency_hz = ((sigfox_ep_addon_ta_csdl_ctx.test_mode_ptr)->rx_frequency_hz);
    // Use RC downlink center frequency if RX frequency is 0.
    if (test_params.rx_frequency_hz == 0) {
        test_params.rx_frequency_hz = (sigfox_ep_addon_ta_csdl_ctx.rc)->f_dl_hz;
    }
#if (defined SIGFOX_EP_REGULATORY) && (defined SIGFOX_EP_SPECTRUM_ACCESS_LBT)
    test_params.lbt_cs_max_duration_first_frame_ms = 0;
#endif
    // Set message parameters.
#ifndef SIGFOX_EP_UL_BIT_RATE_BPS
    message.common_parameters.ul_bit_rate = SIGFOX_UL_BIT_RATE_100BPS;
#endif
#ifndef SIGFOX_EP_TX_POWER_DBM_EIRP
    message.common_parameters.tx_power_dbm_eirp = 14;
#endif
#ifndef SIGFOX_EP_SINGLE_FRAME
    message.common_parameters.number_of_frames = 1;
#ifndef SIGFOX_EP_T_IFU_MS
    message.common_parameters.t_ifu_ms = 0;
#endif
#endif
#ifdef SIGFOX_EP_PUBLIC_KEY_CAPABLE
    message.common_parameters.ep_key_type = SIGFOX_EP_KEY_PRIVATE;
#endif
#ifdef SIGFOX_EP_ASYNCHRONOUS
    message.uplink_cplt_cb = SIGFOX_NULL;
    message.message_cplt_cb = &_SIGFOX_EP_API_message_completion_callback;
#endif
#ifdef SIGFOX_EP_UL_PAYLOAD_SIZE
#if (SIGFOX_EP_UL_PAYLOAD_SIZE == 0)
    message.type = SIGFOX_APPLICATION_MESSAGE_TYPE_EMPTY;
#else
    message.type = SIGFOX_APPLICATION_MESSAGE_TYPE_BYTE_ARRAY;
#endif
#else
    message.type = SIGFOX_APPLICATION_MESSAGE_TYPE_EMPTY;
#endif
#if !(defined SIGFOX_EP_UL_PAYLOAD_SIZE) || (SIGFOX_EP_UL_PAYLOAD_SIZE > 0)
#if (defined SIGFOX_EP_UL_PAYLOAD_SIZE) && (SIGFOX_EP_UL_PAYLOAD_SIZE > 0)
    message.ul_payload = ul_payload;
#else
    message.ul_payload = SIGFOX_NULL;
#endif
#endif
#ifndef SIGFOX_EP_UL_PAYLOAD_SIZE
    message.ul_payload_size_bytes = 0;
#endif
#if (defined SIGFOX_EP_ASYNCHRONOUS) && (defined SIGFOX_EP_BIDIRECTIONAL)
    message.downlink_cplt_cb = SIGFOX_NULL;
#endif
#ifdef SIGFOX_EP_BIDIRECTIONAL
    message.bidirectional_flag = 1;
#ifndef SIGFOX_EP_T_CONF_MS
    message.t_conf_ms = 2000;
#endif
#endif
    // Send message.
#ifdef SIGFOX_EP_ERROR_CODES
    sigfox_ep_api_status = SIGFOX_EP_API_TEST_send_application_message(&message, &test_params);
    SIGFOX_EP_API_check_status(SIGFOX_EP_ADDON_TA_API_ERROR_DRIVER_SIGFOX_EP_API);
#else
    SIGFOX_EP_API_TEST_send_application_message(&message, &test_params);
#endif
#ifdef SIGFOX_EP_ERROR_CODES
errors:
#endif
#ifndef SIGFOX_EP_ASYNCHRONOUS
    sigfox_ep_addon_ta_csdl_ctx.irq_flags.field.message_cplt = 1; // Set flag manually in blocking mode.
#endif
    SIGFOX_RETURN();
}

/*******************************************************************/
static SIGFOX_EP_ADDON_TA_API_status_t _internal_process(void) {
    // Local variables.
#ifdef SIGFOX_EP_ERROR_CODES
    SIGFOX_EP_ADDON_TA_API_status_t status = SIGFOX_EP_ADDON_TA_API_SUCCESS;
    SIGFOX_EP_API_status_t sigfox_ep_api_status = SIGFOX_EP_API_SUCCESS;
#endif
    SIGFOX_EP_API_message_status_t message_status;
    sfx_u8 dl_payload[SIGFOX_DL_PAYLOAD_SIZE_BYTES];
    sfx_s16 dl_rssi_dbm = 0;
    // Perform state machine.
    switch (sigfox_ep_addon_ta_csdl_ctx.state) {
    case SIGFOX_EP_ADDON_TA_CSDL_STATE_READY:
        // Check pending requests.
        if (sigfox_ep_addon_ta_csdl_ctx.internal_flags.field.test_mode_request != 0) {
            // Start first window.
#ifdef SIGFOX_EP_ERROR_CODES
            status = _start_downlink_window();
            SIGFOX_CHECK_STATUS(SIGFOX_EP_ADDON_TA_API_SUCCESS);
#else
            _start_downlink_window();
#endif
            // Clear request.
            sigfox_ep_addon_ta_csdl_ctx.internal_flags.field.test_mode_request = 0;
            // Update state.
            sigfox_ep_addon_ta_csdl_ctx.state = SIGFOX_EP_ADDON_TA_CSDL_STATE_DL_LISTENING;
        }
        break;
    case SIGFOX_EP_ADDON_TA_CSDL_STATE_DL_LISTENING:
        // Check completion or timeout flags.
        if (sigfox_ep_addon_ta_csdl_ctx.irq_flags.field.message_cplt != 0) {
            // Clear flag.
            sigfox_ep_addon_ta_csdl_ctx.irq_flags.field.message_cplt = 0;
            // Read last sequence status.
            message_status = SIGFOX_EP_API_get_message_status();
            // Check downlink status.
            if (message_status.field.dl_frame == 0) {
                // Test mode completed.
                sigfox_ep_addon_ta_csdl_ctx.state = SIGFOX_EP_ADDON_TA_CSDL_STATE_READY;
#ifdef SIGFOX_EP_ASYNCHRONOUS
                // Call internal completion callback.
                if (sigfox_ep_addon_ta_csdl_ctx.internal_cplt_cb != SIGFOX_NULL) {
                    sigfox_ep_addon_ta_csdl_ctx.internal_cplt_cb();
                }
                // Call external completion callback.
                if (((sigfox_ep_addon_ta_csdl_ctx.test_mode_ptr)->test_mode_cplt_cb) != SIGFOX_NULL) {
                    (sigfox_ep_addon_ta_csdl_ctx.test_mode_ptr)->test_mode_cplt_cb();
                }
#endif
            }
            else {
                // Read last downlink payload.
#ifdef SIGFOX_EP_ERROR_CODES
                sigfox_ep_api_status = SIGFOX_EP_API_get_dl_payload((sfx_u8*) dl_payload, SIGFOX_DL_PAYLOAD_SIZE_BYTES, &dl_rssi_dbm);
                SIGFOX_EP_API_check_status(SIGFOX_EP_ADDON_TA_API_ERROR_DRIVER_SIGFOX_EP_API);
#else
                SIGFOX_EP_API_get_dl_payload((sfx_u8*) dl_payload, SIGFOX_DL_PAYLOAD_SIZE_BYTES, &dl_rssi_dbm);
#endif
#ifdef SIGFOX_EP_ASYNCHRONOUS
                sigfox_ep_addon_ta_csdl_ctx.progress_status.progress = 100;
#endif
                // Print last downlink payload.
                if (sigfox_ep_addon_ta_csdl_ctx.test_mode_ptr->downlink_cplt_cb != SIGFOX_NULL) {
                    sigfox_ep_addon_ta_csdl_ctx.test_mode_ptr->downlink_cplt_cb(dl_payload, SIGFOX_DL_PAYLOAD_SIZE_BYTES, dl_rssi_dbm);
                }
                // Start next window.
#ifdef SIGFOX_EP_ERROR_CODES
                status = _start_downlink_window();
                SIGFOX_CHECK_STATUS(SIGFOX_EP_ADDON_TA_API_SUCCESS);
#else
                _start_downlink_window();
#endif
            }
        }
        break;
    default:
        SIGFOX_EXIT_ERROR(SIGFOX_EP_ADDON_TA_API_ERROR_STATE);
    }
    SIGFOX_RETURN();
errors:
    SIGFOX_RETURN();
}

/*** SIGFOX EP ADDON CSDL functions ***/

/*******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_CSDL_open(SIGFOX_EP_ADDON_TA_CSDL_config_t *csdl_test_config) {
    // Local variables.
#ifdef SIGFOX_EP_ERROR_CODES
    SIGFOX_EP_ADDON_TA_API_status_t status = SIGFOX_EP_ADDON_TA_API_SUCCESS;
#endif
#ifdef SIGFOX_EP_PARAMETERS_CHECK
    // Check parameter.
    if (csdl_test_config == SIGFOX_NULL) {
        SIGFOX_EXIT_ERROR(SIGFOX_EP_ADDON_TA_API_ERROR_NULL_PARAMETER);
    }
#endif /* SIGFOX_EP_PARAMETERS_CHECK */
    // Store RC pointer.
    sigfox_ep_addon_ta_csdl_ctx.rc = (csdl_test_config->rc);
    // Init state and flags.
    sigfox_ep_addon_ta_csdl_ctx.state = SIGFOX_EP_ADDON_TA_CSDL_STATE_READY;
    sigfox_ep_addon_ta_csdl_ctx.internal_flags.all = 0;
    sigfox_ep_addon_ta_csdl_ctx.irq_flags.all = 0;
#ifdef SIGFOX_EP_ASYNCHRONOUS
    // Register callbacks.
    sigfox_ep_addon_ta_csdl_ctx.process_cb = (csdl_test_config->process_cb);
    sigfox_ep_addon_ta_csdl_ctx.internal_cplt_cb = (csdl_test_config->internal_cplt_cb);
    sigfox_ep_addon_ta_csdl_ctx.progress_status.all = 0;
    // Update synchronous flag.
    sigfox_ep_addon_ta_csdl_ctx.internal_flags.field.synchronous = (sigfox_ep_addon_ta_csdl_ctx.process_cb == SIGFOX_NULL) ? 1 : 0;
#endif
#ifdef SIGFOX_EP_PARAMETERS_CHECK
errors:
#endif
    SIGFOX_RETURN();
}

/*******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_CSDL_execute_test(SIGFOX_EP_ADDON_TA_API_csdl_test_mode_t *csdl_test_mode) {
    // Local variables.
#ifdef SIGFOX_EP_ERROR_CODES
    SIGFOX_EP_ADDON_TA_API_status_t status = SIGFOX_EP_ADDON_TA_API_SUCCESS;
#endif
    // Check state.
    _CHECK_STATE(!= SIGFOX_EP_ADDON_TA_CSDL_STATE_READY);
    // Reset IRQ flags.
    sigfox_ep_addon_ta_csdl_ctx.irq_flags.all = 0;
#ifdef SIGFOX_EP_ASYNCHRONOUS
    // Reset progress status.
    sigfox_ep_addon_ta_csdl_ctx.progress_status.all = 0;
#endif
    // Store test parameters locally.
#ifdef SIGFOX_EP_ERROR_CODES
    status = _store_test_mode(csdl_test_mode);
    SIGFOX_CHECK_STATUS(SIGFOX_EP_ADDON_TA_API_SUCCESS);
#else
    _store_test_mode(csdl_test_mode);
#endif
    // Set internal request flag.
    sigfox_ep_addon_ta_csdl_ctx.internal_flags.field.test_mode_request = 1;
    // Trigger first frame.
#ifdef SIGFOX_EP_ERROR_CODES
    status = _internal_process();
    SIGFOX_CHECK_STATUS(SIGFOX_EP_ADDON_TA_API_SUCCESS);
#else
    _internal_process();
#endif
#ifdef SIGFOX_EP_ASYNCHRONOUS
    if (sigfox_ep_addon_ta_csdl_ctx.internal_flags.field.synchronous != 0) {
#endif
        // Block until library goes back to READY state.
        while (sigfox_ep_addon_ta_csdl_ctx.state != SIGFOX_EP_ADDON_TA_CSDL_STATE_READY) {
#ifdef SIGFOX_EP_ERROR_CODES
            status = _internal_process();
            SIGFOX_CHECK_STATUS(SIGFOX_EP_ADDON_TA_API_SUCCESS);
#else
            _internal_process();
#endif
        }
#ifdef SIGFOX_EP_ASYNCHRONOUS
    }
    SIGFOX_RETURN();
#endif
errors:
#ifdef SIGFOX_EP_ASYNCHRONOUS
    sigfox_ep_addon_ta_csdl_ctx.progress_status.error = 1;
#endif
    SIGFOX_RETURN();
}

#ifdef SIGFOX_EP_ASYNCHRONOUS
/*******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_CSDL_process(void) {
    // Local variables.
#ifdef SIGFOX_EP_ERROR_CODES
    SIGFOX_EP_ADDON_TA_API_status_t status = SIGFOX_EP_ADDON_TA_API_SUCCESS;
#endif
    // Set process flag.
    sigfox_ep_addon_ta_csdl_ctx.internal_flags.field.process_running = 1;
    // Run the internal process.
    while (sigfox_ep_addon_ta_csdl_ctx.irq_flags.all != 0) {
#ifdef SIGFOX_EP_ERROR_CODES
        status = _internal_process();
        SIGFOX_CHECK_STATUS(SIGFOX_EP_ADDON_TA_API_SUCCESS);
#else
        _internal_process();
#endif
    }
#ifdef SIGFOX_EP_ASYNCHRONOUS
    // Reset process flag.
    sigfox_ep_addon_ta_csdl_ctx.internal_flags.field.process_running = 0;
    SIGFOX_RETURN();
#endif
#ifdef SIGFOX_EP_ERROR_CODES
errors:
#endif
    // Reset process flag.
    sigfox_ep_addon_ta_csdl_ctx.internal_flags.field.process_running = 0;
#ifdef SIGFOX_EP_ASYNCHRONOUS
    sigfox_ep_addon_ta_csdl_ctx.progress_status.error = 1;
#endif
    SIGFOX_RETURN();
}
#endif

#ifdef SIGFOX_EP_ASYNCHRONOUS
/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_TA_API_progress_status_t SIGFOX_EP_ADDON_TA_CSDL_get_progress_status(void)
 * \brief Get the progress status.
 * \param[in]   none
 * \param[out]  none
 * \retval      Progress status.
 *******************************************************************/
SIGFOX_EP_ADDON_TA_API_progress_status_t SIGFOX_EP_ADDON_TA_CSDL_get_progress_status(void) {
    return (sigfox_ep_addon_ta_csdl_ctx.progress_status);
}
#endif

#endif /* SIGFOX_EP_APPLICATION_MESSAGES and SIGFOX_EP_BIDIRECTIONAL */
