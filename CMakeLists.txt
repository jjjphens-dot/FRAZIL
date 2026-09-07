cmake_minimum_required(VERSION 3.25)

project(FRAZIL VERSION 0.1.0 LANGUAGES C CXX)

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

target_sources(FRAZIL
    PRIVATE
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
        src/dsp/DryWetMixer.cpp
        src/dsp/DryWetMixer.h
        src/dsp/primitives/LinearSmoother.cpp
        src/dsp/primitives/LinearSmoother.h
        src/dsp/primitives/RandomSource.cpp
        src/dsp/primitives/RandomSource.h
        src/plugin/ParameterLayout.cpp
        src/plugin/ParameterLayout.h
        src/plugin/PluginEditor.cpp
        src/plugin/PluginEditor.h
        src/plugin/PluginProcessor.cpp
        src/plugin/PluginProcessor.h
        src/plugin/StateAdapter.cpp
        src/plugin/StateAdapter.h
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

add_executable(frazil_smoke tests/smoke/smoke.cpp)
target_compile_features(frazil_smoke PRIVATE cxx_std_20)
add_test(NAME frazil_smoke COMMAND frazil_smoke)

add_executable(frazil_tests
    tests/unit/test_main.cpp
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
    src/dsp/DryWetMixer.cpp
    src/dsp/DryWetMixer.h
    src/dsp/primitives/LinearSmoother.cpp
    src/dsp/primitives/LinearSmoother.h
    src/dsp/primitives/RandomSource.cpp
    src/dsp/primitives/RandomSource.h
    src/plugin/ParameterLayout.cpp
    src/plugin/ParameterLayout.h
    src/plugin/StateAdapter.cpp
    src/plugin/StateAdapter.h
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
    src/dsp/DryWetMixer.cpp
    src/dsp/DryWetMixer.h
    src/dsp/primitives/LinearSmoother.cpp
    src/dsp/primitives/LinearSmoother.h
    src/dsp/primitives/RandomSource.cpp
    src/dsp/primitives/RandomSource.h
    src/plugin/ParameterLayout.cpp
    src/plugin/ParameterLayout.h
    src/plugin/PluginEditor.cpp
    src/plugin/PluginEditor.h
    src/plugin/PluginProcessor.cpp
    src/plugin/PluginProcessor.h
    src/plugin/StateAdapter.cpp
    src/plugin/StateAdapter.h
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

# MSVC ASAN uses a dynamic runtime DLL. Keep test execution independent from the
# shell that launched CTest and make the test artifacts directly runnable.
if(MSVC AND CMAKE_CXX_FLAGS MATCHES "/fsanitize=address")
    get_filename_component(frazil_msvc_tool_bin_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)
    set(frazil_asan_runtime_dll
        "${frazil_msvc_tool_bin_dir}/clang_rt.asan_dynamic-x86_64.dll")

    set_property(
        TEST frazil_smoke frazil_unit frazil_plugin_integration
        PROPERTY ENVIRONMENT_MODIFICATION
                 "PATH=path_list_prepend:${frazil_msvc_tool_bin_dir}"
    )
    foreach(frazil_test_target IN ITEMS frazil_smoke frazil_tests frazil_plugin_integration)
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
