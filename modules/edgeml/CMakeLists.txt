#############################################################################
# Copyright (c) 2026 BootCtrl
#
# This program and the accompanying materials are made available under the
# terms of the Eclipse Public License 2.0 which is available at
# http://www.eclipse.org/legal/epl-2.0.
#
# SPDX-License-Identifier: EPL-2.0
#############################################################################

if (NOT DEFINED FORTE_MODULE_EDGEML)
    option(FORTE_MODULE_EDGEML "EdgeML Function Blocks and runtime support" ON)
endif ()

if (NOT FORTE_MODULE_EDGEML)
    return()
endif ()

if (NOT TARGET forte OR NOT TARGET forte-core)
    message(FATAL_ERROR "The edgeml module requires 4diac FORTE targets (forte, forte-core). Add 4diacFORTE first.")
endif ()

add_library(forte-edgeml)
target_link_libraries(forte-edgeml PUBLIC forte-core)
target_link_libraries(forte PUBLIC $<IF:$<BOOL:${BUILD_SHARED_LIBS}>,forte-edgeml,$<LINK_LIBRARY:WHOLE_ARCHIVE,forte-edgeml>>)

add_subdirectory(include)
add_subdirectory(src)

install(TARGETS forte-edgeml EXPORT forte-export FILE_SET HEADERS)
