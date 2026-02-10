#############################################################################
# Copyright (c) 2026 BootCtrl
#
# SPDX-License-Identifier: EPL-2.0
#############################################################################

target_sources(forte-edgeml PUBLIC
        FILE_SET HEADERS
        FILES
        ML_ArgMax_fbt.h
        ML_AnomalyScore_fbt.h
        ML_Softmax_fbt.h
        ML_Threshold_fbt.h
)
