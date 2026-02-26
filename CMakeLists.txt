cmake_minimum_required(VERSION 3.30)

project(BootCtrl_EdgeOS
        DESCRIPTION "BootCtrl EdgeOS superproject with 4diac submodules and out-of-tree EdgeML modules"
        LANGUAGES C CXX)

option(BOOTCTRL_ENABLE_EDGEML "Build EdgeML module" ON)

# 4diac FORTE cache defaults can be overridden from command line.
set(FORTE_ARCHITECTURE "Posix" CACHE STRING "FORTE architecture")
set(FORTE_COM_ETH ON CACHE BOOL "Enable FORTE ETH communication")
set(FORTE_COM_FBDK ON CACHE BOOL "Enable FORTE FBDK communication")
set(FORTE_COM_LOCAL ON CACHE BOOL "Enable FORTE local communication")
set(FORTE_COM_PAHOMQTT ON CACHE BOOL "Enable FORTE Paho MQTT communication")
set(FORTE_MODULE_CONVERT ON CACHE BOOL "Enable FORTE convert module")
set(FORTE_MODULE_IEC61131 ON CACHE BOOL "Enable FORTE IEC61131 module")
set(FORTE_MODULE_UTILS ON CACHE BOOL "Enable FORTE utils module")

if (FORTE_COM_PAHOMQTT)
    if (NOT FORTE_COM_PAHOMQTT_INCLUDE_DIR)
        find_path(_BOOTCTRL_PAHO_INCLUDE_DIR
                NAMES MQTTAsync.h
                PATH_SUFFIXES include)
        if (_BOOTCTRL_PAHO_INCLUDE_DIR)
            set(FORTE_COM_PAHOMQTT_INCLUDE_DIR
                    "${_BOOTCTRL_PAHO_INCLUDE_DIR}"
                    CACHE PATH "ABSOLUTE path to Paho MQTT src directory" FORCE)
        endif ()
    endif ()

    if (NOT FORTE_COM_PAHOMQTT_LIB)
        find_library(_BOOTCTRL_PAHO_LIBRARY
                NAMES paho-mqtt3a paho-mqtt3as libpaho-mqtt3a libpaho-mqtt3as)
        if (_BOOTCTRL_PAHO_LIBRARY)
            set(FORTE_COM_PAHOMQTT_LIB
                    "${_BOOTCTRL_PAHO_LIBRARY}"
                    CACHE STRING "MQTT Library name to be linked to forte" FORCE)
        endif ()
    endif ()

    if (NOT FORTE_COM_PAHOMQTT_LIB_DIR)
        set(_BOOTCTRL_PAHO_LIBRARY_FOR_DIR "${FORTE_COM_PAHOMQTT_LIB}")
        if (_BOOTCTRL_PAHO_LIBRARY_FOR_DIR)
            if (NOT IS_ABSOLUTE "${_BOOTCTRL_PAHO_LIBRARY_FOR_DIR}")
                find_library(_BOOTCTRL_PAHO_LIBRARY_ABS
                        NAMES ${_BOOTCTRL_PAHO_LIBRARY_FOR_DIR})
                if (_BOOTCTRL_PAHO_LIBRARY_ABS)
                    set(_BOOTCTRL_PAHO_LIBRARY_FOR_DIR "${_BOOTCTRL_PAHO_LIBRARY_ABS}")
                endif ()
            endif ()
            if (IS_ABSOLUTE "${_BOOTCTRL_PAHO_LIBRARY_FOR_DIR}")
                get_filename_component(_BOOTCTRL_PAHO_LIB_DIR "${_BOOTCTRL_PAHO_LIBRARY_FOR_DIR}" DIRECTORY)
                set(FORTE_COM_PAHOMQTT_LIB_DIR
                        "${_BOOTCTRL_PAHO_LIB_DIR}"
                        CACHE PATH "ABSOLUTE path to Paho MQTT library" FORCE)
            endif ()
        endif ()
    endif ()

    if (NOT FORTE_COM_PAHOMQTT_INCLUDE_DIR)
        message(FATAL_ERROR "FORTE_COM_PAHOMQTT=ON but Paho MQTT headers were not found (missing MQTTAsync.h). Install libpaho-mqtt-dev or pass FORTE_COM_PAHOMQTT_INCLUDE_DIR.")
    endif ()
    if (NOT FORTE_COM_PAHOMQTT_LIB)
        message(FATAL_ERROR "FORTE_COM_PAHOMQTT=ON but Paho MQTT library was not found. Install libpaho-mqtt-dev or pass FORTE_COM_PAHOMQTT_LIB.")
    endif ()
endif ()

if (BOOTCTRL_ENABLE_EDGEML)
    set(FORTE_MODULE_EDGEML ON CACHE BOOL "Enable EdgeML module" FORCE)
endif ()

# Upstream runtimes as submodules.
add_subdirectory(4diacFORTE)

# Out-of-tree project code.
if (BOOTCTRL_ENABLE_EDGEML)
    add_subdirectory(modules)

    # FORTE tests discover linked libs before this module is added,
    # so explicitly link the module when tests are enabled.
    if (TARGET forte_test)
        target_link_libraries(forte_test
                $<IF:$<BOOL:${BUILD_SHARED_LIBS}>,forte-edgeml,$<LINK_LIBRARY:WHOLE_ARCHIVE,forte-edgeml>>)
        target_sources(forte_test PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/edgeml/ML_Threshold_tester.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/edgeml/ML_ArgMax_tester.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/edgeml/ML_Softmax_tester.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/edgeml/ML_AnomalyScore_tester.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/edgeml/ML_Scale_tester.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/edgeml/ML_Normalize_tester.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/edgeml/ML_Filter_tester.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/edgeml/ML_MovingAverage_tester.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/edgeml/ML_FFT_tester.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/edgeml/ML_PCA_tester.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/edgeml/ML_Inference_tester.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/edgeml/ML_ModelManager_tester.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/edgeml/ML_OTAUpdate_tester.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/edgeml/ML_Monitoring_tester.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/edgeml/ML_SHAPValues_tester.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/edgeml/tflite_backend_tester.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/edgeml/model_registry_tester.cpp)
    endif ()
endif ()
