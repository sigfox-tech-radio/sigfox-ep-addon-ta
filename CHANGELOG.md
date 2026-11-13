# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [v3.1](https://github.com/sigfox-tech-radio/sigfox-ep-addon-ta/releases/tag/v3.1) - 13 Nov 2026

### Changed

* Changed from the Clear BSD License to the standard **BSD 3-Clause License**. See the `LICENSE` file for details.

## [v3.0](https://github.com/sigfox-tech-radio/sigfox-ep-addon-ta/releases/tag/v3.0) - 07 Nov 2025

### Changed

* Upgrade to **sigfox-ep-lib v4.3**.
* Use **short message and long message** naming in frequency hopping modes enumeration.

## [v2.1](https://github.com/sigfox-tech-radio/sigfox-ep-addon-ta/releases/tag/v2.1) - 25 Jun 2025

### Changed

* Remove **#error directive** when `SIGFOX_EP_CERTIFICATION` is not defined, so that the addon can be removed via compilation flags when not used (switch between test and production firmware for example).

## [v2.0](https://github.com/sigfox-tech-radio/sigfox-ep-addon-ta/releases/tag/v2.0) - 22 Nov 2024

### Added

* Add **new** `downlink_cplt_cb` **callback to print DL payload** when the CSDL test mode receives a valid downlink frame (`MCU_API_print_dl_payload()` function replacement).

### Fixed

* Remove `dl_t_rx_ms` **field check** in CSDL test mode since value 0 is valid (taking RC default value).
* Use **RC default downlink frequency** when `rx_frequency_hz` is set to 0 in CSDL test mode.
* Fix **bit rate setting** issue (missing conversion from enumeration to bps value).
* Remove **unifdef dependency** in all cmake with linked target.

### Changed

* Upgrade to **sigfox-ep-lib v4.0**.
* Add **_t** suffix to all functions typedef.

## [v1.1](https://github.com/sigfox-tech-radio/sigfox-ep-addon-ta/releases/tag/v1.1) - 22 Mar 2024

### Fixed

* Fix **error code value** in `SIGFOX_EP_ADDON_TA_API_get_version()` function.
* Fix **compilation issue** on RC zone selection when defining only one FH zone.
* Fix **compilation issue** on `ul_payload` field when defining `UL_PAYLOAD_SIZE` flag.

### Changed

* Rename `RCx` compilation flags to `RCx_ZONE` for **Microchip MCUs compatibility**.

## [v1.0](https://github.com/sigfox-tech-radio/sigfox-ep-addon-ta/releases/tag/v1.0) - 09 Nov 2023

### General

* First version of the Sigfox EP Type Approval addon.

### Added

* **Continuous wave (CW)** test mode.
* **Continuous Sigfox uplink (CSUL)** test mode.
* **Continuous Sigfox downlink (CSDL)** test mode.
