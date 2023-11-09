/*!*****************************************************************
 * \file    sigfox_ep_addon_ta_cw.h
 * \brief   Sigfox End-Point type approval add-on continuous wave test.
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

#ifndef __SIGFOX_EP_ADDON_TA_CW_H__
#define __SIGFOX_EP_ADDON_TA_CW_H__

#ifdef USE_SIGFOX_EP_FLAGS_H
#include "sigfox_ep_flags.h"
#endif
#include "sigfox_ep_addon_ta_api.h"
#include "manuf/rf_api.h"
#include "sigfox_types.h"

/*** SIGFOX EP ADDON TA CW structures ***/

/*!******************************************************************
 * \struct SIGFOX_EP_ADDON_TA_CW_config_t
 * \brief CW test mode driver configuration.
 *******************************************************************/
typedef struct {
	const SIGFOX_rc_t *rc;
} SIGFOX_EP_ADDON_TA_CW_config_t;

/*** SIGFOX EP ADDON TA CW functions ***/

/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_CW_open(SIGFOX_EP_ADDON_TA_CW_config_t* cw_test_config)
 * \brief Open continuous wave test mode driver.
 * \param[in]   cw_test_config: Pointer to the test mode configuration.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_CW_open(SIGFOX_EP_ADDON_TA_CW_config_t* cw_test_config);

/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_CW_start(SIGFOX_EP_ADDON_TA_API_cw_test_mode_t* cw_test_mode)
 * \brief Start continuous wave test mode.
 * \param[in]   cw_test_mode: Pointer to the test mode parameters.
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_CW_start(SIGFOX_EP_ADDON_TA_API_cw_test_mode_t* cw_test_mode);

/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_CW_stop(void)
 * \brief Stop continuous wave test mode.
 * \param[in]   none
 * \param[out]  none
 * \retval      Function execution status.
 *******************************************************************/
SIGFOX_EP_ADDON_TA_API_status_t SIGFOX_EP_ADDON_TA_CW_stop(void);

#ifdef ASYNCHRONOUS
/*!******************************************************************
 * \fn SIGFOX_EP_ADDON_TA_API_progress_status_t SIGFOX_EP_ADDON_TA_CW_get_progress_status(void)
 * \brief Get the progress status.
 * \param[in]   none
 * \param[out]  none
 * \retval      Progress status.
 *******************************************************************/
SIGFOX_EP_ADDON_TA_API_progress_status_t SIGFOX_EP_ADDON_TA_CW_get_progress_status(void);
#endif

#endif /* __SIGFOX_EP_ADDON_TA_CW_H__ */
