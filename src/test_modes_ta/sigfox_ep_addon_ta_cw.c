/*!*****************************************************************
 * \file    sigfox_ep_addon_ta_cw.c
 * \brief   Sigfox End-Point type approval add-on continuous test.
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

#include "test_modes_ta/sigfox_ep_addon_ta_cw.h"

#ifndef SIGFOX_EP_DISABLE_FLAGS_FILE
#include "sigfox_ep_flags.h"
#endif
#include "sigfox_ep_addon_ta_api.h"
#include "manuf/rf_api.h"
#include "sigfox_types.h"
#include "sigfox_error.h"

/*** SIGFOX EP ADDON TA CW local structures ***/

typedef struct {
    const SIGFOX_rc_t *rc;
#ifdef SIGFOX_EP_ASYNCHRONOUS
    SIGFOX_EP_ADDON_TA_API_progress_status_t progress_status; // Set to 100% when CW is successfully started.
#endif
} SIGFOX_EP_ADDON_TA_CW_context_t;

/*** SIGFOX EP ADDON TA CW local variables ***/

static SIGFOX_EP_ADDON_TA_CW_context_t sigfox_ep_addon_ta_cw_ctx;

/*** SIGFOX EP ADDON TA CW functions ***/

/*******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_CW_open(SIGFOX_EP_ADDON_TA_CW_config_t *cw_test_config) {
    // Local variables.
#ifdef SIGFOX_EP_ERROR_CODES
    SIGFOX_EP_ADDON_TA_API_status_t status = SIGFOX_EP_ADDON_TA_API_SUCCESS;
#endif
#ifdef SIGFOX_EP_PARAMETERS_CHECK
    if (cw_test_config == SIGFOX_NULL) {
        SIGFOX_EXIT_ERROR(SIGFOX_EP_ADDON_TA_API_ERROR_NULL_PARAMETER);
    }
    if ((cw_test_config->rc) == SIGFOX_NULL) {
        SIGFOX_EXIT_ERROR(SIGFOX_EP_ADDON_TA_API_ERROR_NULL_PARAMETER);
    }
#endif
    // Update local context.
    sigfox_ep_addon_ta_cw_ctx.rc = (cw_test_config->rc);
#ifdef SIGFOX_EP_ASYNCHRONOUS
    sigfox_ep_addon_ta_cw_ctx.progress_status.all = 0;
#endif
#ifdef SIGFOX_EP_PARAMETERS_CHECK
errors:
#endif
    SIGFOX_RETURN();
}

/*******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_CW_start(SIGFOX_EP_ADDON_TA_API_cw_test_mode_t *cw_test_mode) {
    // Local variables.
#ifdef SIGFOX_EP_ERROR_CODES
    SIGFOX_EP_ADDON_TA_API_status_t status = SIGFOX_EP_ADDON_TA_API_SUCCESS;
    RF_API_status_t rf_api_status = RF_API_SUCCESS;
#endif
    RF_API_radio_parameters_t radio_params;
#ifdef SIGFOX_EP_ASYNCHRONOUS
    // Reset progress status.
    sigfox_ep_addon_ta_cw_ctx.progress_status.all = 0;
#endif
#ifdef SIGFOX_EP_PARAMETERS_CHECK
    if (cw_test_mode == SIGFOX_NULL) {
        SIGFOX_EXIT_ERROR(SIGFOX_EP_ADDON_TA_API_ERROR_NULL_PARAMETER);
    }
#ifndef SIGFOX_EP_UL_BIT_RATE_BPS
    if ((cw_test_mode -> ul_bit_rate) >= SIGFOX_UL_BIT_RATE_LAST) {
        SIGFOX_EXIT_ERROR(SIGFOX_EP_ADDON_TA_API_ERROR_UL_BIT_RATE);
    }
#endif
#endif
    // Build radio configuration structure.
    radio_params.rf_mode = RF_API_MODE_TX;
    radio_params.frequency_hz = ((cw_test_mode->tx_frequency_hz) != 0) ? (cw_test_mode->tx_frequency_hz) : ((sigfox_ep_addon_ta_cw_ctx.rc)->f_ul_hz);
    radio_params.modulation = (cw_test_mode->modulation);
#ifdef SIGFOX_EP_UL_BIT_RATE_BPS
    radio_params.bit_rate_bps = SIGFOX_EP_UL_BIT_RATE_BPS;
#else
    radio_params.bit_rate_bps = SIGFOX_UL_BIT_RATE_BPS_LIST[cw_test_mode -> ul_bit_rate];
#endif
#ifdef SIGFOX_EP_TX_POWER_DBM_EIRP
    radio_params.tx_power_dbm_eirp = SIGFOX_EP_TX_POWER_DBM_EIRP;
#else
    radio_params.tx_power_dbm_eirp = (cw_test_mode->tx_power_dbm_eirp);
#endif
#ifdef SIGFOX_EP_BIDIRECTIONAL
    radio_params.deviation_hz = 0;
#endif
    // Wake-up radio.
#ifdef SIGFOX_EP_ERROR_CODES
    rf_api_status = RF_API_wake_up();
    RF_API_check_status(SIGFOX_EP_ADDON_TA_API_ERROR_DRIVER_RF_API);
#else
    RF_API_wake_up();
#endif
    // Init radio.
#ifdef SIGFOX_EP_ERROR_CODES
    rf_api_status = RF_API_init(&radio_params);
    RF_API_check_status(SIGFOX_EP_ADDON_TA_API_ERROR_DRIVER_RF_API);
#else
    RF_API_init(&radio_params);
#endif
    // Start continuous wave.
#ifdef SIGFOX_EP_ERROR_CODES
    rf_api_status = RF_API_start_continuous_wave();
    RF_API_check_status(SIGFOX_EP_ADDON_TA_API_ERROR_DRIVER_RF_API);
#else
    RF_API_start_continuous_wave();
#endif
#ifdef SIGFOX_EP_ASYNCHRONOUS
    sigfox_ep_addon_ta_cw_ctx.progress_status.progress = 100;
    SIGFOX_RETURN();
#endif
#if (defined SIGFOX_EP_PARAMETERS_CHECK) || (defined SIGFOX_EP_ERROR_CODES)
errors:
#endif
#ifdef SIGFOX_EP_ASYNCHRONOUS
    sigfox_ep_addon_ta_cw_ctx.progress_status.error = 1;
#endif
    SIGFOX_RETURN();
}

/*******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_CW_stop(void) {
    // Local variables.
#ifdef SIGFOX_EP_ERROR_CODES
    SIGFOX_EP_ADDON_TA_API_status_t status = SIGFOX_EP_ADDON_TA_API_SUCCESS;
    RF_API_status_t rf_api_status = RF_API_SUCCESS;
#endif
#ifdef SIGFOX_EP_ASYNCHRONOUS
    // Reset progress status.
    sigfox_ep_addon_ta_cw_ctx.progress_status.all = 0;
#endif
    // Stop continuous wave.
#ifdef SIGFOX_EP_ERROR_CODES
    rf_api_status = RF_API_de_init();
    RF_API_check_status(SIGFOX_EP_ADDON_TA_API_ERROR_DRIVER_RF_API);
#else
    RF_API_de_init();
#endif
    // Turn radio off.
#ifdef SIGFOX_EP_ERROR_CODES
    rf_api_status = RF_API_sleep();
    RF_API_check_status(SIGFOX_EP_ADDON_TA_API_ERROR_DRIVER_RF_API);
#else
    RF_API_sleep();
#endif
#ifdef SIGFOX_EP_ASYNCHRONOUS
    SIGFOX_RETURN();
#endif
#ifdef SIGFOX_EP_ERROR_CODES
errors:
#endif
#ifdef SIGFOX_EP_ASYNCHRONOUS
    sigfox_ep_addon_ta_cw_ctx.progress_status.error = 1;
#endif
    SIGFOX_RETURN();
}

#ifdef SIGFOX_EP_ASYNCHRONOUS
/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_TA_API_progress_status_t SIGFOX_EP_ADDON_TA_CW_get_progress_status(void)
 * \brief Get the progress status.
 * \param[in]   none
 * \param[out]  none
 * \retval      Progress status.
 *******************************************************************/
SIGFOX_EP_ADDON_TA_API_progress_status_t SIGFOX_EP_ADDON_TA_CW_get_progress_status(void) {
    return (sigfox_ep_addon_ta_cw_ctx.progress_status);
}
#endif
