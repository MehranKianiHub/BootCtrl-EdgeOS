#############################################################################
# Copyright (c) 2026 BootCtrl
#
# SPDX-License-Identifier: EPL-2.0
#############################################################################

target_sources(forte-edgeml PUBLIC
        FILE_SET HEADERS
        FILES
        edgeml_status.h
        model_metadata.h
        model_registry.h
        runtime_context.h
)
