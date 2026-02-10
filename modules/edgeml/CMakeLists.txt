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

option(FORTE_EDGEML_BACKEND_TFLITE "Enable TensorFlow Lite backend integration for EdgeML" ON)
set(FORTE_EDGEML_TFLITE_INCLUDE_DIR
    ""
    CACHE PATH "Path containing tensorflow/lite headers")
set(FORTE_EDGEML_TFLITE_LIBRARY
    ""
    CACHE FILEPATH "Path to TensorFlow Lite library file (e.g. libtensorflowlite.so)")

add_library(forte-edgeml)
target_link_libraries(forte-edgeml PUBLIC forte-core)
target_link_libraries(forte PUBLIC $<IF:$<BOOL:${BUILD_SHARED_LIBS}>,forte-edgeml,$<LINK_LIBRARY:WHOLE_ARCHIVE,forte-edgeml>>)

# Reset active state every configure run; it is re-enabled only when dependencies are found.
set(FORTE_EDGEML_BACKEND_TFLITE_ACTIVE OFF CACHE INTERNAL "Whether TFLite backend is active for this build" FORCE)

if (FORTE_EDGEML_BACKEND_TFLITE)
    set(_edgeml_tflite_include "")
    set(_edgeml_tflite_library "")

    if (FORTE_EDGEML_TFLITE_INCLUDE_DIR)
        if (EXISTS "${FORTE_EDGEML_TFLITE_INCLUDE_DIR}/tensorflow/lite/interpreter.h")
            set(_edgeml_tflite_include "${FORTE_EDGEML_TFLITE_INCLUDE_DIR}")
        else ()
            message(STATUS
                    "EdgeML: FORTE_EDGEML_TFLITE_INCLUDE_DIR is set but invalid: ${FORTE_EDGEML_TFLITE_INCLUDE_DIR}")
        endif ()
    else ()
        find_path(_edgeml_tflite_include
                  NAMES tensorflow/lite/interpreter.h
                  PATHS /usr/include /usr/local/include)
    endif ()

    if (FORTE_EDGEML_TFLITE_LIBRARY)
        if (EXISTS "${FORTE_EDGEML_TFLITE_LIBRARY}")
            set(_edgeml_tflite_library "${FORTE_EDGEML_TFLITE_LIBRARY}")
        else ()
            message(STATUS "EdgeML: FORTE_EDGEML_TFLITE_LIBRARY is set but invalid: ${FORTE_EDGEML_TFLITE_LIBRARY}")
        endif ()
    else ()
        find_library(_edgeml_tflite_library
                     NAMES tensorflowlite libtensorflowlite
                     PATHS /usr/lib /usr/local/lib /usr/lib/x86_64-linux-gnu /usr/local/lib64)
    endif ()

    if (_edgeml_tflite_include MATCHES "-NOTFOUND$")
        set(_edgeml_tflite_include "")
    endif ()
    if (_edgeml_tflite_library MATCHES "-NOTFOUND$")
        set(_edgeml_tflite_library "")
    endif ()

    if (_edgeml_tflite_include AND _edgeml_tflite_library)
        set(FORTE_EDGEML_BACKEND_TFLITE_ACTIVE ON CACHE INTERNAL "Whether TFLite backend is active for this build" FORCE)
        target_include_directories(forte-edgeml PUBLIC "${_edgeml_tflite_include}")
        target_link_libraries(forte-edgeml PUBLIC "${_edgeml_tflite_library}")
        target_compile_definitions(forte-edgeml PUBLIC FORTE_EDGEML_BACKEND_TFLITE=1)
        message(STATUS "EdgeML: TFLite backend enabled")
    else ()
        message(STATUS
                "EdgeML: TFLite backend requested but dependencies were not found; using NullBackend fallback. "
                "Set FORTE_EDGEML_TFLITE_INCLUDE_DIR and FORTE_EDGEML_TFLITE_LIBRARY to enable TFLite.")
    endif ()
else ()
    message(STATUS "EdgeML: TFLite backend disabled by configuration; using NullBackend.")
endif ()

add_subdirectory(include)
add_subdirectory(src)

install(TARGETS forte-edgeml EXPORT forte-export FILE_SET HEADERS)
