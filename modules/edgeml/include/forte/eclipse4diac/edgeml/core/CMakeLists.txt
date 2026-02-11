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
        monitoring_pipeline.h
        ota_persistence.h
        ota_security.h
        ota_trust_store.h
        runtime_context.h
)
