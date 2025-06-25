/*!*****************************************************************
 * \file    sigfox_ep_addon_ta_csdl.h
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

#ifndef __SIGFOX_EP_ADDON_TA_CSDL_H__
#define __SIGFOX_EP_ADDON_TA_CSDL_H__

#ifndef SIGFOX_EP_DISABLE_FLAGS_FILE
#include "sigfox_ep_flags.h"
#endif
#include "sigfox_ep_addon_ta_api.h"
#include "sigfox_types.h"

#if ((defined SIGFOX_EP_CERTIFICATION) && (defined SIGFOX_EP_BIDIRECTIONAL))

/*** SIGFOX EP ADDON CSDL structures ***/

/*!******************************************************************
 * \struct SIGFOX_EP_ADDON_TA_CSDL_config_t
 * \brief CW test mode driver configuration.
 *******************************************************************/
typedef struct {
    const SIGFOX_rc_t *rc;
#ifdef SIGFOX_EP_ASYNCHRONOUS
    SIGFOX_EP_ADDON_TA_API_process_cb_t process_cb;
    SIGFOX_EP_ADDON_TA_API_test_mode_cplt_cb_t internal_cplt_cb;
#endif
} SIGFOX_EP_ADDON_TA_CSDL_config_t;

/*** SIGFOX EP ADDON CSDL functions ***/

/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_CSDL_open(SIGFOX_EP_ADDON_TA_CSDL_config_t* csdl_test_config)
 * \brief Open continuous Sigfox downlink test mode driver.
 * \param[in]   csdl_test_config: Pointer to the test mode configuration.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_CSDL_open(SIGFOX_EP_ADDON_TA_CSDL_config_t *csdl_test_config);

/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_CSDL_execute_test(SIGFOX_EP_ADDON_TA_API_csdl_test_mode_t* csdl_test_mode)
 * \brief Execute (in blocking mode) or start (in asynchronous mode) Sigfox continuous downlink test mode.
 * \param[in]   csdl_test_config: Pointer to the test mode configuration.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_CSDL_execute_test(SIGFOX_EP_ADDON_TA_API_csdl_test_mode_t *csdl_test_mode);

#ifdef SIGFOX_EP_ASYNCHRONOUS
/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_CSDL_process(void)
 * \brief Process continuous Sigfox downlink test mode.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_CSDL_process(void);
#endif

#ifdef SIGFOX_EP_ASYNCHRONOUS
/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_TA_API_progress_status_t SIGFOX_EP_ADDON_TA_CSDL_get_progress_status(void)
 * \brief Get the progress status.
 * \param[in]   none
 * \param[out]  none
 * \retval      Progress status.
 *******************************************************************/
SIGFOX_EP_ADDON_TA_API_progress_status_t SIGFOX_EP_ADDON_TA_CSDL_get_progress_status(void);
#endif

#endif /* SIGFOX_EP_CERTIFICATION and SIGFOX_EP_BIDIRECTIONAL */

#endif /* __SIGFOX_EP_ADDON_TA_CSDL_H__ */
