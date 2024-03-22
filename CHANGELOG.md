# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [v1.1](https://github.com/sigfox-tech-radio/sigfox-ep-addon-ta/releases/tag/v1.1) - 22 Mar 2024

### Fixed

* Fix **error code value** in `SIGFOX_EP_ADDON_TA_API_get_version()` function.
* Fix **compilation issue** on RC zone selection when defining only one FH zone.
* Fix **compilation issue** on `ul_payload` field when defining `UL_PAYLOAD_SIZE` flag.

### Changed

* Rename `RCx` compilation flags into `RCx_ZONE` for **Microchip MCUs compatibility**.

## [v1.0](https://github.com/sigfox-tech-radio/sigfox-ep-addon-ta/releases/tag/v1.0) - 09 Nov 2023

### General

* First version of the Sigfox EP Type Approval addon.

### Added

* **Continuous wave (CW)** test mode.
* **Continuous Sigfox uplink (CSUL)** test mode.
* **Continuous Sigfox downlink (CSDL)** test mode.
