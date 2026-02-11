#############################################################################
# Copyright (c) 2026 BootCtrl
#
# SPDX-License-Identifier: EPL-2.0
#############################################################################

target_sources(forte-edgeml PUBLIC
        FILE_SET HEADERS
        FILES
        ML_FFT_fbt.h
        ML_Filter_fbt.h
        ML_MovingAverage_fbt.h
        ML_Normalize_fbt.h
        ML_PCA_fbt.h
        ML_Scale_fbt.h
)
