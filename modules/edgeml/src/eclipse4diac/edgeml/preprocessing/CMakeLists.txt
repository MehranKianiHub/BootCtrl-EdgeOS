#############################################################################
# Copyright (c) 2026 BootCtrl
#
# SPDX-License-Identifier: EPL-2.0
#############################################################################

target_sources(forte-edgeml PRIVATE
        ML_FFT_fbt.cpp
        ML_Filter_fbt.cpp
        ML_MovingAverage_fbt.cpp
        ML_Normalize_fbt.cpp
        ML_PCA_fbt.cpp
        ML_Scale_fbt.cpp
)
