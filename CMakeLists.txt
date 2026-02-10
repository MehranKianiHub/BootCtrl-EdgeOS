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
set(FORTE_MODULE_CONVERT ON CACHE BOOL "Enable FORTE convert module")
set(FORTE_MODULE_IEC61131 ON CACHE BOOL "Enable FORTE IEC61131 module")
set(FORTE_MODULE_UTILS ON CACHE BOOL "Enable FORTE utils module")

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
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/edgeml/ML_Scale_tester.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/edgeml/ML_Normalize_tester.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/edgeml/ML_Inference_tester.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/edgeml/ML_ModelManager_tester.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/edgeml/tflite_backend_tester.cpp
                ${CMAKE_CURRENT_SOURCE_DIR}/tests/edgeml/model_registry_tester.cpp)
    endif ()
endif ()
