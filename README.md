# Sigfox End-Point Type Approval addon (EP_ADDON_TA)

## Description

The **Sigfox End-Point Type Approval addon** provides utility functions to pass regulatory certifications with the new [Sigfox end point library](https://github.com/sigfox-tech-radio/sigfox-ep-lib). 

The table below shows the versions **compatibility** between this addon and the Sigfox End-Point library.

| **EP_ADDON_TA** | **EP_LIB** |
|:---:|:---:|
| [v1.1](https://github.com/sigfox-tech-radio/sigfox-ep-addon-ta/releases/tag/v1.1) | >= [v3.5](https://github.com/sigfox-tech-radio/sigfox-ep-lib/releases/tag/v3.5) |
| [v1.0](https://github.com/sigfox-tech-radio/sigfox-ep-addon-ta/releases/tag/v1.0) | [v3.4](https://github.com/sigfox-tech-radio/sigfox-ep-lib/releases/tag/v3.4) |

The application is divided into three modes detailed thereafter.

### Continuous wave (CW)

After implementing the corresponding RF_API functions, this mode generates a **continuous wave** (modulated or not) which can be used for output power measurement and TX spurious tests.

### Continuous Sigfox uplink (CSUL)

This mode continuously sends a configurable number of **Sigfox uplink frames**. In the particular case of FCC certification in RC2 and RC4 zones, the `SIGFOX_EP_ADDON_TA_API_FH_MODE_ALL_MACRO_CHANNELS` option (equivalent to the **LONG_MESSAGE** configuration of the old Sigfox library) is helpful to send Sigfox messages in all FCC macro channels and perform the related tests.

### Continuous Sigfox downlink (CSDL)

This mode puts the device in **continuous reception** mode with configurable timeout, in order to perform RX spurious tests and resiliency tests.

## Stack architecture

<p align="center">
<img src="https://github.com/sigfox-tech-radio/sigfox-ep-addon-ta/wiki/images/sigfox_ep_addon_ta_architecture.drawio.png" width="600"/>
</p>

## Compilation flags for optimization

This addon inherits all the [Sigfox End-Point library flags](https://github.com/sigfox-tech-radio/sigfox-ep-lib/wiki/compilation-flags-for-optimization) and can be optimized accordingly.

The `CERTIFICATION` flag must be enabled to use this addon.

## How to add Sigfox Type Approval addon to your project

### Dependencies

The only dependency of this addon is the [Sigfox End-Point library](https://github.com/sigfox-tech-radio/sigfox-ep-lib) source code.

### Submodule

The best way to embed the Sigfox End-Point Type Approval addon into your project is to use a [Git submodule](https://git-scm.com/book/en/v2/Git-Tools-Submodules), in a similar way to the library. The addon will be seen as a sub-repository with independant history. It will be much easier to **upgrade the addon** or to **switch between versions** when necessary, by using the common `git pull` and `git checkout` commands within the `sigfox-ep-addon-ta` folder.

To add the Sigfox type approval addon submodule, go to your project location and run the following commands:

```bash
mkdir lib
cd lib/
git submodule add https://github.com/sigfox-tech-radio/sigfox-ep-addon-ta.git
```

This will clone the Sigfox End-Point Type Approval addon repository. At project level, you can commit the submodule creation with the following commands:

```bash
git commit --message "Add Sigfox End Point Type Approval addon submodule."
git push
```

With the submodule, you can easily:

* Update the addon to the **latest version**:

```bash
cd lib/sigfox-ep-addon-ta/
git pull
git checkout master
```

* Use a **specific release**:

```bash
cd lib/sigfox-ep-addon-ta/
git pull
git checkout <tag>
```

### Raw source code

You can [download](https://github.com/sigfox-tech-radio/sigfox-ep-addon-ta/releases) or clone any release of the Sigfox End-Point Type Approval addon and copy all files into your project.

```bash
git clone https://github.com/sigfox-tech-radio/sigfox-ep-addon-ta.git
```

### Precompiled source code

You can [download](https://github.com/sigfox-tech-radio/sigfox-ep-addon-ta/releases) or clone any release of the Sigfox End-Point Type Approval addon and copy all files into your project. If you do not plan to change your compilation flags in the future, you can perform a **precompilation step** before copying the file in your project. The precompilation will **remove all preprocessor directives** according to your flags selection, in order to produce a more **readable code**. Then you can copy the new files into your project.

```bash
git clone https://github.com/sigfox-tech-radio/sigfox-ep-addon-ta.git
```

To perform the precompilation, you have to install `cmake` and `unifdef` tools, and run the following commands:

```bash
cd sigfox-ep-addon-ta/
mkdir build
cd build/
```

* Precompiling by reading the `sigfox_ep_flags.h` file:

```bash
cmake -DSIGFOX_EP_LIB_DIR=<sigfox-ep-lib path> \
      -DUSE_SIGFOX_EP_FLAGS_H=ON ..
make precompil_sigfox_ep_addon_ta
```

* Precompiling by entering the flags selection on command line:

```bash
cmake -DSIGFOX_EP_LIB_DIR=<sigfox-ep-lib path> \
      -DUSE_SIGFOX_EP_FLAGS_H=OFF \
      -DRC1_ZONE=ON \
      -DRC2_ZONE=ON \
      -DRC3C_ZONE=ON \
      -DRC3D_ZONE=ON \
      -DRC4_ZONE=ON \
      -DRC5_ZONE=ON \
      -DRC6_ZONE=ON \
      -DRC7_ZONE=ON \
      -DAPPLICATION_MESSAGES=ON \
      -DCONTROL_KEEP_ALIVE_MESSAGE=ON \
      -DBIDIRECTIONAL=ON \
      -DASYNCHRONOUS=ON \
      -DLOW_LEVEL_OPEN_CLOSE=ON \
      -DREGULATORY=ON \
      -DLATENCY_COMPENSATION=ON \
      -DSINGLE_FRAME=ON \
      -DUL_BIT_RATE_BPS=OFF \
      -DTX_POWER_DBM_EIRP=OFF \
      -DT_IFU_MS=OFF \
      -DT_CONF_MS=OFF \
      -DUL_PAYLOAD_SIZE=OFF \
      -DCRC_HW=OFF \
      -DMESSAGE_COUNTER_ROLLOVER=OFF \
      -DPARAMETERS_CHECK=ON \
      -DCERTIFICATION=ON \
      -DPUBLIC_KEY_CAPABLE=ON \
      -DVERBOSE=ON \
      -DERROR_CODES=ON \
      -DERROR_STACK=12 ..
make precompil_sigfox_ep_addon_ta
```

The new files will be generated in the `build/precompil` folder.

### Static library

You can also [download](https://github.com/sigfox-tech-radio/sigfox-ep-addon-ta/releases) or clone any release of the Sigfox End-Point Type Approval addon and build a **static library**.

```bash
git clone https://github.com/sigfox-tech-radio/sigfox-ep-addon-ta.git
```

To build a static library, you have to install `cmake` tool and run the following commands:

```bash
cd sigfox-ep-addon-ta/
mkdir build
cd build/
```

* Building by reading the `sigfox_ep_flags.h` file:

```bash
cmake -DSIGFOX_EP_LIB_DIR=<sigfox-ep-lib path> \
      -DUSE_SIGFOX_EP_FLAGS_H=ON ..
make sigfox_ep_addon_ta
```

* Building by entering the flags selection on command line:

```bash
cmake -DSIGFOX_EP_LIB_DIR=<sigfox-ep-lib path> \
      -DUSE_SIGFOX_EP_FLAGS_H=OFF \
      -DRC1_ZONE=ON \
      -DRC2_ZONE=ON \
      -DRC3C_ZONE=ON \
      -DRC3D_ZONE=ON \
      -DRC4_ZONE=ON \
      -DRC5_ZONE=ON \
      -DRC6_ZONE=ON \
      -DRC7_ZONE=ON \
      -DAPPLICATION_MESSAGES=ON \
      -DCONTROL_KEEP_ALIVE_MESSAGE=ON \
      -DBIDIRECTIONAL=ON \
      -DASYNCHRONOUS=ON \
      -DLOW_LEVEL_OPEN_CLOSE=ON \
      -DREGULATORY=ON \
      -DLATENCY_COMPENSATION=ON \
      -DSINGLE_FRAME=ON \
      -DPARAMETERS_CHECK=ON \
      -DCERTIFICATION=ON \
      -DPUBLIC_KEY_CAPABLE=ON \
      -DVERBOSE=ON \
      -DCRC_HW=OFF \
      -DERROR_CODES=ON \
      -DUL_BIT_RATE_BPS=OFF \
      -DT_IFU_MS=OFF \
      -DT_CONF_MS=OFF \
      -DUL_PAYLOAD_SIZE=OFF \
      -DMESSAGE_COUNTER_ROLLOVER=OFF \
      -DERROR_STACK=12 ..
make sigfox_ep_addon_ta
```

The archive will be generated in the `build/lib` folder.
