#############################################################################
# Copyright (c) 2026 BootCtrl
#
# SPDX-License-Identifier: EPL-2.0
#############################################################################

target_sources(forte-edgeml PRIVATE
        ML_ArgMax_fbt.cpp
        ML_AnomalyScore_fbt.cpp
        ML_SHAPValues_fbt.cpp
        ML_Softmax_fbt.cpp
        ML_Threshold_fbt.cpp
)
