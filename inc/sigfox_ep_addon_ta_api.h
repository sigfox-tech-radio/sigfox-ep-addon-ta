/*!*****************************************************************
 * \file    sigfox_ep_addon_ta_api.h
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

#ifndef __SIGFOX_EP_ADDON_TA_API_H__
#define __SIGFOX_EP_ADDON_TA_API_H__

#ifndef SIGFOX_EP_DISABLE_FLAGS_FILE
#include "sigfox_ep_flags.h"
#endif
#include "manuf/rf_api.h"
#include "sigfox_types.h"

#ifdef SIGFOX_EP_CERTIFICATION

/*** SIGFOX EP ADDON TA API structures ***/

#ifdef SIGFOX_EP_ERROR_CODES
/*!******************************************************************
 * \enum SIGFOX_EP_ADDON_TA_status_t
 * \brief Sigfox EP ADDON TA error codes.
 *******************************************************************/
typedef enum {
    // Core addon errors.
    SIGFOX_EP_ADDON_TA_API_SUCCESS = 0,
    SIGFOX_EP_ADDON_TA_API_ERROR_NULL_PARAMETER,
    SIGFOX_EP_ADDON_TA_API_ERROR_STATE,
    SIGFOX_EP_ADDON_TA_API_ERROR_UL_BIT_RATE,
    SIGFOX_EP_ADDON_TA_API_ERROR_NUMBER_OF_FRAMES,
    SIGFOX_EP_ADDON_TA_API_ERROR_FREQUENCY_HOPPING_MODE,
    SIGFOX_EP_ADDON_TA_API_ERROR_FREQUENCY_HOPPING_RC,
    // Low level errors.
    // Activate the SIGFOX_EP_ERROR_STACK flag and use the SIGFOX_EP_API_unstack_error() function to get more details.
    SIGFOX_EP_ADDON_TA_API_ERROR_DRIVER_MCU_API,
    SIGFOX_EP_ADDON_TA_API_ERROR_DRIVER_RF_API,
    SIGFOX_EP_ADDON_TA_API_ERROR_DRIVER_SIGFOX_EP_API
} SIGFOX_EP_ADDON_TA_API_status_t;
#else
typedef void SIGFOX_EP_ADDON_TA_API_status_t;
#endif

#ifdef SIGFOX_EP_ASYNCHRONOUS
/*!******************************************************************
 * \brief Sigfox EP ADDON TA addon callback functions.
 * \fn SIGFOX_EP_ADDON_TA_API_process_cb_t:     Will be called each time a low level IRQ is handled by the addon library. Warning: runs in a IRQ context. Should only change variables state, and call as soon as possible @ref SIGFOX_EP_ADDON_TA_API_process.
 * \fn SIGFOX_EP_ADDON_TA_API_test_cplt_cb_t:   Will be called as soon as the test mode is completed. Optional, could be set to NULL.
 *******************************************************************/
typedef void (*SIGFOX_EP_ADDON_TA_API_process_cb_t)(void);
typedef void (*SIGFOX_EP_ADDON_TA_API_test_mode_cplt_cb_t)(void);
#endif

#ifdef SIGFOX_EP_BIDIRECTIONAL
/*!******************************************************************
 * \brief Sigfox EP ADDON TA CSDL test mode callback function.
 * \brief This callback replaces the previous MCU_API_print_dl_payload() function defined in the Sigfox EP library until v3.6.
 * \fn SIGFOX_EP_ADDON_TA_API_downlink_cplt_cb_t:   Will be called as soon as a valid downlink message is received.
 *******************************************************************/
typedef void (*SIGFOX_EP_ADDON_TA_API_downlink_cplt_cb_t)(sfx_u8 *dl_payload, sfx_u8 dl_payload_size, sfx_s16 rssi_dbm);
#endif

/*!******************************************************************
 * \enum SIGFOX_EP_ADDON_TA_API_config_t
 * \brief Sigfox EP ADDON TA configuration structure.
 *******************************************************************/
typedef struct {
    const SIGFOX_rc_t *rc;
#ifdef SIGFOX_EP_ASYNCHRONOUS
    SIGFOX_EP_ADDON_TA_API_process_cb_t process_cb;
#endif
} SIGFOX_EP_ADDON_TA_API_config_t;

#ifdef SIGFOX_EP_SPECTRUM_ACCESS_FH
/*!******************************************************************
 * \enum SIGFOX_EP_ADDON_TA_API_fh_mode_t
 * \brief Sigfox ADDON TA API frequency hopping modes for continuous Sigfox uplink.
 *******************************************************************/
typedef enum {
    SIGFOX_EP_ADDON_TA_API_FH_MODE_SIGFOX_MACRO_CHANNEL_ONLY = 0,
    SIGFOX_EP_ADDON_TA_API_FH_MODE_ALL_MACRO_CHANNELS,
    SIGFOX_EP_ADDON_TA_API_FH_MODE_LAST
} SIGFOX_EP_ADDON_TA_API_fh_mode_t;
#endif

/*!******************************************************************
 * \enum SIGFOX_EP_ADDON_TA_API_state_t
 * \brief Addon states list.
 *******************************************************************/
typedef enum {
    SIGFOX_EP_ADDON_TA_API_STATE_CLOSED = 0,
    SIGFOX_EP_ADDON_TA_API_STATE_READY,
    SIGFOX_EP_ADDON_TA_API_STATE_RUNNING,
    SIGFOX_EP_ADDON_TA_API_STATE_LAST
} SIGFOX_EP_ADDON_TA_API_state_t;

/*!******************************************************************
 * \union SIGFOX_EP_ADDON_TA_API_progress_status_t
 * \brief Progress status.
 *******************************************************************/
typedef union {
    struct {
        sfx_u8 error :1;
        sfx_u8 progress :7;
    };
    sfx_u8 all;
} SIGFOX_EP_ADDON_TA_API_progress_status_t;

/*!******************************************************************
 * \struct SIGFOX_EP_ADDON_TA_API_cw_test_mode_t
 * \brief Sigfox ADDON TA continuous wave test mode parameters.
 *******************************************************************/
typedef struct {
    sfx_u32 tx_frequency_hz; // If non-zero, bypass the uplink frequency of the RC.
    RF_API_modulation_t modulation;
#ifndef SIGFOX_EP_UL_BIT_RATE_BPS
    SIGFOX_ul_bit_rate_t ul_bit_rate;
#endif
#ifndef SIGFOX_EP_TX_POWER_DBM_EIRP
    sfx_s8 tx_power_dbm_eirp;
#endif
} SIGFOX_EP_ADDON_TA_API_cw_test_mode_t;

/*!******************************************************************
 * \struct SIGFOX_EP_ADDON_TA_API_csul_test_mode_t
 * \brief Sigfox ADDON TA continuous Sigfox uplink test mode parameters.
 *******************************************************************/
typedef struct {
    sfx_u32 number_of_frames;
    sfx_u32 tx_frequency_hz; // If non-zero, bypass the random frequency generator of the EP library.
#ifdef SIGFOX_EP_SPECTRUM_ACCESS_FH
    SIGFOX_EP_ADDON_TA_API_fh_mode_t frequency_hopping_mode;
#endif
#ifndef SIGFOX_EP_UL_BIT_RATE_BPS
    SIGFOX_ul_bit_rate_t ul_bit_rate;
#endif
#ifndef SIGFOX_EP_TX_POWER_DBM_EIRP
    sfx_s8 tx_power_dbm_eirp;
#endif
#ifdef SIGFOX_EP_ASYNCHRONOUS
    SIGFOX_EP_ADDON_TA_API_test_mode_cplt_cb_t test_mode_cplt_cb;
#endif
} SIGFOX_EP_ADDON_TA_API_csul_test_mode_t;

#ifdef SIGFOX_EP_BIDIRECTIONAL
/*!******************************************************************
 * \struct SIGFOX_EP_ADDON_TA_API_csdl_test_mode_t
 * \brief Sigfox ADDON TA continuous Sigfox downlink test mode parameters.
 *******************************************************************/
typedef struct {
    sfx_u32 dl_t_rx_ms; // If non-zero, bypass the downlink timeout value (T_RX) of the RC.
    sfx_u32 rx_frequency_hz; // If non-zero, bypass the downlink frequency of the RC.
    SIGFOX_EP_ADDON_TA_API_downlink_cplt_cb_t downlink_cplt_cb; // Called on every successful downlink frame reception.
#ifdef SIGFOX_EP_ASYNCHRONOUS
    SIGFOX_EP_ADDON_TA_API_test_mode_cplt_cb_t test_mode_cplt_cb;
#endif
} SIGFOX_EP_ADDON_TA_API_csdl_test_mode_t;
#endif

/*** SIGFOX EP ADDON TA API functions ***/

/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_API_open(SIGFOX_EP_ADDON_TA_API_config_t *config)
 * \brief Open the TA addon.
 * \param[in]   config: Pointer to the addon configuration.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_API_open(SIGFOX_EP_ADDON_TA_API_config_t *config);

/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_API_close(void)
 * \brief Close the TA addon.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_API_close(void);

/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_API_start_continuous_wave(SIGFOX_EP_ADDON_TA_API_cw_test_mode_t *cw_test_mode)
 * \brief Start the continuous wave test mode.
 * \param[in]   cw_test_mode: Pointer to the CW test mode configuration.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_API_start_continuous_wave(SIGFOX_EP_ADDON_TA_API_cw_test_mode_t *cw_test_mode);

/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_API_stop_continuous_wave(void)
 * \brief Stop the continuous wave test mode.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_API_stop_continuous_wave(void);

/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_API_execute_continuous_sigfox_uplink(SIGFOX_EP_ADDON_TA_API_csul_test_mode_t *csul_est_mode)
 * \brief Execute the continuous Sigfox uplink test mode.
 * \param[in]   csul_test_mode: Pointer to the CSUL test mode configuration.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_API_execute_continuous_sigfox_uplink(SIGFOX_EP_ADDON_TA_API_csul_test_mode_t *csul_test_mode);

#ifdef SIGFOX_EP_BIDIRECTIONAL
/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_API_execute_continuous_sigfox_downlink(SIGFOX_EP_ADDON_TA_API_csdl_test_mode_t *csdl_test_mode)
 * \brief Execute the continuous Sigfox downlink test mode.
 * \param[in]   csdl_test_mode: Pointer to the CSDL test mode configuration.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_API_execute_continuous_sigfox_downlink(SIGFOX_EP_ADDON_TA_API_csdl_test_mode_t *csdl_test_mode);
#endif

/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_TA_API_state_t SIGFOX_EP_ADDON_TA_API_get_state(void)
 * \brief Get the current addon state.
 * \param[in]   none
 * \param[out]  none
 * \retval      Current addon.
 *******************************************************************/
SIGFOX_EP_ADDON_TA_API_state_t SIGFOX_EP_ADDON_TA_API_get_state(void);

#ifdef SIGFOX_EP_ASYNCHRONOUS
/*!******************************************************************
 * \fn void SIGFOX_EP_ADDON_TA_API_process(void)
 * \brief Main process function of the addon. This function should be called as soon as possible when the process callback is triggered.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_API_process(void);
#endif

#ifdef SIGFOX_EP_ASYNCHRONOUS
/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_TA_API_progress_status_t SIGFOX_EP_ADDON_TA_API_get_test_mode_progress_status(void)
 * \brief Get the progress status.
 * \param[in]   none
 * \param[out]  none
 * \retval      Progress status.
 *******************************************************************/
SIGFOX_EP_ADDON_TA_API_progress_status_t SIGFOX_EP_ADDON_TA_API_get_progress_status(void);
#endif

#ifdef SIGFOX_EP_VERBOSE
/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_API_get_version(sfx_u8 **version, sfx_u8 *version_size_char)
 * \brief Get EP library version.
 * \param[out]  version: Version string.
 * \param[out]  version_size_char: Pointer to the string size.
 * \retval      Function execution status.
 *******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_API_get_version(sfx_u8 **version, sfx_u8 *version_size_char);
#endif

#endif /* SIGFOX_EP_CERTIFICATION */

#endif /* __SIGFOX_EP_ADDON_TA_API_H__ */
