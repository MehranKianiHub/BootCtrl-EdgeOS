#############################################################################
# Copyright (c) 2026 BootCtrl
#
# SPDX-License-Identifier: EPL-2.0
#############################################################################

target_sources(forte-edgeml PRIVATE
        null_backend.cpp
)

if (FORTE_EDGEML_BACKEND_TFLITE_ACTIVE)
    target_sources(forte-edgeml PRIVATE
            tflite_backend.cpp
    )
endif ()
