#############################################################################
# Copyright (c) 2026 BootCtrl
#
# SPDX-License-Identifier: EPL-2.0
#############################################################################

target_sources(forte-edgeml PUBLIC
        FILE_SET HEADERS
        FILES
        backend_interface.h
        null_backend.h
)

if (FORTE_EDGEML_BACKEND_TFLITE_ACTIVE)
    target_sources(forte-edgeml PUBLIC
            FILE_SET HEADERS
            FILES
            tflite_backend.h
    )
endif ()
