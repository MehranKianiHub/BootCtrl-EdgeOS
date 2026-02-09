# EdgeML Module (Out-of-Tree)

This directory contains the incremental implementation of EdgeML-4diac as an out-of-tree module in the BootCtrl superproject.

## Build Switch
- CMake option: `FORTE_MODULE_EDGEML=ON`

## Current Architecture
- `core/`: shared status and model abstractions
- `backend/`: backend interface and placeholder backend
- `preprocessing/`: input transformation FBs
- `postprocessing/`: output decision FBs

## Implemented Function Blocks

### 1. `ML_Scale`
Purpose: linearly map input from one range to another.

Inputs:
- `IN : REAL`
- `IN_MIN : REAL`
- `IN_MAX : REAL`
- `OUT_MIN : REAL`
- `OUT_MAX : REAL`
- `CLAMP : BOOL`

Outputs:
- `OUT : REAL`
- `VALID : BOOL`
- `ERROR : BOOL`
- `ERROR_CODE : USINT`

Error codes:
- `0`: ok
- `1`: non-finite input detected
- `2`: invalid input range (`IN_MIN >= IN_MAX`)

### 2. `ML_Normalize`
Purpose: normalize input using Min-Max or Z-Score mode.

Inputs:
- `IN : REAL`
- `METHOD : USINT` (`0=MinMax`, `1=ZScore`)
- `MIN : REAL`
- `MAX : REAL`
- `MEAN : REAL`
- `STDDEV : REAL`
- `CLAMP : BOOL` (used for Min-Max)

Outputs:
- `OUT : REAL`
- `VALID : BOOL`
- `ERROR : BOOL`
- `ERROR_CODE : USINT`

Error codes:
- `0`: ok
- `1`: non-finite input detected
- `2`: invalid Min-Max range (`MIN >= MAX`)
- `3`: invalid standard deviation (`STDDEV <= 0`)
- `4`: unknown method

### 3. `ML_Threshold`
Purpose: threshold comparison with inclusive/exclusive mode.

## Tests
- `tests/edgeml/ML_Scale_tester.cpp`
- `tests/edgeml/ML_Normalize_tester.cpp`
- `tests/edgeml/ML_Threshold_tester.cpp`
- `tests/edgeml/model_registry_tester.cpp`

## Notes
- Upstream runtimes are submodules in this repository:
  - `4diacFORTE/`
  - `4diacFBE/`
- Use `git submodule update --init --recursive` after cloning.
