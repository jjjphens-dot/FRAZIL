cmake_minimum_required(VERSION 3.25)

project(FRAZIL VERSION 0.1.0 LANGUAGES C CXX)

execute_process(
    COMMAND git rev-parse HEAD
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE FRAZIL_GIT_COMMIT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)
if(NOT FRAZIL_GIT_COMMIT)
    set(FRAZIL_GIT_COMMIT "unknown")
endif()

execute_process(
    COMMAND git status --porcelain --untracked-files=normal
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    OUTPUT_VARIABLE FRAZIL_GIT_STATUS
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE FRAZIL_GIT_STATUS_RESULT
    ERROR_QUIET
)
if(NOT FRAZIL_GIT_STATUS_RESULT EQUAL 0)
    set(FRAZIL_GIT_STATE "unknown")
elseif(FRAZIL_GIT_STATUS)
    set(FRAZIL_GIT_STATE "dirty")
else()
    set(FRAZIL_GIT_STATE "clean")
endif()

set(FRAZIL_COMPILER_FLAGS "${CMAKE_CXX_FLAGS}")
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    string(APPEND FRAZIL_COMPILER_FLAGS " ${CMAKE_CXX_FLAGS_DEBUG}")
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    string(APPEND FRAZIL_COMPILER_FLAGS " ${CMAKE_CXX_FLAGS_RELEASE}")
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    string(APPEND FRAZIL_COMPILER_FLAGS " ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    string(APPEND FRAZIL_COMPILER_FLAGS " ${CMAKE_CXX_FLAGS_MINSIZEREL}")
endif()
string(STRIP "${FRAZIL_COMPILER_FLAGS}" FRAZIL_COMPILER_FLAGS)
if(NOT FRAZIL_COMPILER_FLAGS)
    set(FRAZIL_COMPILER_FLAGS "none reported by CMake configure")
endif()

if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/JUCE/CMakeLists.txt")
    message(FATAL_ERROR
        "JUCE 9.0.1 is missing. Expected it at external/JUCE."
    )
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(external/JUCE)

juce_add_plugin(FRAZIL
    COMPANY_NAME "FRAZIL"
    COMPANY_WEBSITE "https://example.invalid/frazil"
    COMPANY_EMAIL "devnull@example.invalid"
    BUNDLE_ID "com.frazil.audio.FRAZIL"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Frzl
    PLUGIN_CODE Frz1
    FORMATS VST3 Standalone
    PRODUCT_NAME "FRAZIL"
    VST3_CATEGORIES Fx
)

set(FRAZIL_APP_SOURCES
    src/app/AudioEngine.cpp
    src/app/AudioEngine.h
    src/app/EngineParameters.h
    src/app/ParameterMapper.cpp
    src/app/ParameterMapper.h
    src/app/ParameterSnapshot.cpp
    src/app/ParameterSnapshot.h
    src/app/ProcessSpec.h
    src/app/StateModel.cpp
    src/app/StateModel.h
)

set(FRAZIL_DSP_SOURCES
    src/dsp/DryWetMixer.cpp
    src/dsp/DryWetMixer.h
    src/dsp/primitives/LinearSmoother.cpp
    src/dsp/primitives/LinearSmoother.h
    src/dsp/primitives/RandomSource.cpp
    src/dsp/primitives/RandomSource.h
)

set(FRAZIL_PLUGIN_ADAPTER_SOURCES
    src/plugin/ParameterLayout.cpp
    src/plugin/ParameterLayout.h
    src/plugin/StateAdapter.cpp
    src/plugin/StateAdapter.h
)

set(FRAZIL_PLUGIN_SOURCES
    ${FRAZIL_APP_SOURCES}
    ${FRAZIL_DSP_SOURCES}
    ${FRAZIL_PLUGIN_ADAPTER_SOURCES}
    src/plugin/PluginEditor.cpp
    src/plugin/PluginEditor.h
    src/plugin/PluginProcessor.cpp
    src/plugin/PluginProcessor.h
)

target_sources(FRAZIL
    PRIVATE
        ${FRAZIL_PLUGIN_SOURCES}
)

target_include_directories(FRAZIL PRIVATE src)

target_compile_features(FRAZIL PRIVATE cxx_std_20)

target_compile_definitions(FRAZIL
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(FRAZIL
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
)

juce_generate_juce_header(FRAZIL)

enable_testing()
find_package(Python3 COMPONENTS Interpreter REQUIRED)

add_executable(frazil_smoke tests/smoke/smoke.cpp)
target_compile_features(frazil_smoke PRIVATE cxx_std_20)
add_test(NAME frazil_smoke COMMAND frazil_smoke)

juce_add_console_app(frazil_render
    PRODUCT_NAME "FRAZIL Offline Render"
    NEEDS_WEB_BROWSER FALSE
    NEEDS_CURL FALSE
)
target_sources(frazil_render
    PRIVATE
    tests/render/offline_render.cpp
    src/app/AudioEngine.cpp
    src/app/AudioEngine.h
    src/app/EngineParameters.h
    src/app/ProcessSpec.h
    src/dsp/DryWetMixer.cpp
    src/dsp/DryWetMixer.h
    src/dsp/primitives/LinearSmoother.cpp
    src/dsp/primitives/LinearSmoother.h
)
target_compile_features(frazil_render PRIVATE cxx_std_20)
target_include_directories(frazil_render PRIVATE src)
target_compile_definitions(frazil_render
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
)
target_link_libraries(frazil_render
    PRIVATE
        juce::juce_audio_formats
        juce::juce_audio_basics
)

add_executable(frazil_tests
    tests/unit/test_main.cpp
    ${FRAZIL_APP_SOURCES}
    ${FRAZIL_DSP_SOURCES}
    ${FRAZIL_PLUGIN_ADAPTER_SOURCES}
)
target_compile_features(frazil_tests PRIVATE cxx_std_20)
target_include_directories(frazil_tests PRIVATE src)
target_link_libraries(frazil_tests PRIVATE juce::juce_audio_basics juce::juce_audio_processors)
add_test(NAME frazil_unit COMMAND frazil_tests)

juce_add_console_app(frazil_plugin_integration
    PRODUCT_NAME "FRAZIL Plugin Integration"
    NEEDS_WEB_BROWSER FALSE
    NEEDS_CURL FALSE
)
target_sources(frazil_plugin_integration
    PRIVATE
    tests/integration/plugin_integration.cpp
    ${FRAZIL_PLUGIN_SOURCES}
)
target_compile_features(frazil_plugin_integration PRIVATE cxx_std_20)
target_include_directories(frazil_plugin_integration PRIVATE src)
target_compile_definitions(frazil_plugin_integration
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)
target_link_libraries(frazil_plugin_integration
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
)
juce_generate_juce_header(frazil_plugin_integration)
add_test(NAME frazil_plugin_integration COMMAND frazil_plugin_integration)

juce_add_console_app(frazil_processor_property
    PRODUCT_NAME "FRAZIL Processor Property Harness"
    NEEDS_WEB_BROWSER FALSE
    NEEDS_CURL FALSE
)
target_sources(frazil_processor_property
    PRIVATE
    tests/integration/processor_property.cpp
    ${FRAZIL_PLUGIN_SOURCES}
)
target_compile_features(frazil_processor_property PRIVATE cxx_std_20)
target_include_directories(frazil_processor_property PRIVATE src)
target_compile_definitions(frazil_processor_property
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)
target_link_libraries(frazil_processor_property
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
)
juce_generate_juce_header(frazil_processor_property)
add_test(NAME frazil_processor_property COMMAND frazil_processor_property)

juce_add_console_app(frazil_latency_contract
    PRODUCT_NAME "FRAZIL Latency Contract"
    NEEDS_WEB_BROWSER FALSE
    NEEDS_CURL FALSE
)
target_sources(frazil_latency_contract
    PRIVATE
    tests/integration/latency_contract.cpp
    ${FRAZIL_PLUGIN_SOURCES}
)
target_compile_features(frazil_latency_contract PRIVATE cxx_std_20)
target_include_directories(frazil_latency_contract PRIVATE src)
target_compile_definitions(frazil_latency_contract
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)
target_link_libraries(frazil_latency_contract
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_formats
        juce::juce_audio_processors
)
juce_generate_juce_header(frazil_latency_contract)
add_test(
    NAME frazil_latency_contract
    COMMAND frazil_latency_contract
            --input "${CMAKE_CURRENT_SOURCE_DIR}/testdata/input/zero_state_response__impulse.wav"
)

juce_add_console_app(frazil_performance_baseline
    PRODUCT_NAME "FRAZIL Performance Baseline"
    NEEDS_WEB_BROWSER FALSE
    NEEDS_CURL FALSE
)
target_sources(frazil_performance_baseline
    PRIVATE
    tests/performance/performance_baseline.cpp
    ${FRAZIL_PLUGIN_SOURCES}
)
target_compile_features(frazil_performance_baseline PRIVATE cxx_std_20)
target_include_directories(frazil_performance_baseline PRIVATE src)
target_compile_definitions(frazil_performance_baseline
    PRIVATE
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        FRAZIL_GIT_COMMIT="${FRAZIL_GIT_COMMIT}"
        FRAZIL_GIT_STATE="${FRAZIL_GIT_STATE}"
        FRAZIL_BUILD_TYPE="${CMAKE_BUILD_TYPE}"
        FRAZIL_COMPILER_FLAGS="${FRAZIL_COMPILER_FLAGS}"
)
target_link_libraries(frazil_performance_baseline
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
)
if(WIN32)
    target_link_libraries(frazil_performance_baseline PRIVATE Psapi)
endif()
juce_generate_juce_header(frazil_performance_baseline)
add_test(
    NAME frazil_performance_baseline
    COMMAND frazil_performance_baseline
            --output "${CMAKE_CURRENT_BINARY_DIR}/performance/perf_base_001.json"
)
add_test(
    NAME frazil_render
    COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tools/render_testdata.py
            --renderer $<TARGET_FILE:frazil_render>
            --output "${CMAKE_CURRENT_BINARY_DIR}/rendered/zero_state_response__impulse.wav"
            --manifest "${CMAKE_CURRENT_BINARY_DIR}/rendered/zero_state_response__impulse.manifest.json"
            --build-type ${CMAKE_BUILD_TYPE}
)
add_test(
    NAME frazil_render_cli
    COMMAND ${Python3_EXECUTABLE}
            ${CMAKE_CURRENT_SOURCE_DIR}/tools/test_render_cli.py
            --renderer $<TARGET_FILE:frazil_render>
)

# MSVC ASAN uses a dynamic runtime DLL. Keep test execution independent from the
# shell that launched CTest and make the test artifacts directly runnable.
if(MSVC AND CMAKE_CXX_FLAGS MATCHES "/fsanitize=address")
    get_filename_component(frazil_msvc_tool_bin_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)
    set(frazil_asan_runtime_dll
        "${frazil_msvc_tool_bin_dir}/clang_rt.asan_dynamic-x86_64.dll")

    set_property(
        TEST frazil_smoke frazil_unit frazil_plugin_integration frazil_processor_property
             frazil_latency_contract frazil_performance_baseline frazil_render frazil_render_cli
        PROPERTY ENVIRONMENT_MODIFICATION
                 "PATH=path_list_prepend:${frazil_msvc_tool_bin_dir}"
    )
    foreach(
        frazil_test_target
        IN ITEMS frazil_smoke frazil_tests frazil_plugin_integration frazil_processor_property
                   frazil_latency_contract frazil_performance_baseline frazil_render
    )
        if(EXISTS "${frazil_asan_runtime_dll}")
            add_custom_command(
                TARGET ${frazil_test_target}
                POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                        "${frazil_asan_runtime_dll}"
                        "$<TARGET_FILE_DIR:${frazil_test_target}>"
                VERBATIM
            )
        endif()
    endforeach()

    if(NOT EXISTS "${frazil_asan_runtime_dll}")
        message(WARNING
            "MSVC ASAN runtime was not found at ${frazil_asan_runtime_dll}; "
            "ASAN test executables may not run outside a Developer environment."
        )
    endif()
endif()
