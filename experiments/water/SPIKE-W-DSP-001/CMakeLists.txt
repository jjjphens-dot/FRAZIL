add_library(frazil_water_research STATIC
    dsp/ResearchBaseline.h
    ../../../src/dsp/primitives/RandomSource.cpp
)
target_include_directories(frazil_water_research PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/src")
target_compile_features(frazil_water_research PUBLIC cxx_std_20)

set(water_test_targets)
set(water_test_names)
foreach(module IN ITEMS baseline features modal bubble flow droplet fluid event_pool protect_detector protect)
    set(target "frazil_water_${module}_tests")
    if(module STREQUAL "baseline")
        set(target frazil_water_experiment_tests)
    endif()
    add_executable(${target} "tests/${module}_tests.cpp")
    target_link_libraries(${target} PRIVATE frazil_water_research)
    add_test(NAME "frazil_water_${module}" COMMAND ${target})
    list(APPEND water_test_targets ${target})
    list(APPEND water_test_names "frazil_water_${module}")
endforeach()

juce_add_console_app(frazil_water_experiment_render PRODUCT_NAME "FRAZIL Water Research Render")
target_sources(frazil_water_experiment_render PRIVATE render/render_main.cpp)
target_compile_definitions(frazil_water_experiment_render PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
target_link_libraries(frazil_water_experiment_render PRIVATE frazil_water_research juce::juce_audio_formats)
add_test(NAME frazil_water_experiment_render_cli
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/render_cli_test.py"
            $<TARGET_FILE:frazil_water_experiment_render>)
add_test(NAME frazil_water_protect_listening
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/protect_listening_test.py"
            $<TARGET_FILE:frazil_water_experiment_render>)

add_executable(frazil_water_performance tests/performance.cpp
    ../../../src/app/AudioEngine.cpp ../../../src/dsp/DryWetMixer.cpp
    ../../../src/dsp/primitives/LinearSmoother.cpp)
target_link_libraries(frazil_water_performance PRIVATE frazil_water_research juce::juce_audio_basics)

# Include opt-in research in existing safe presets; never link it into production targets.
add_dependencies(frazil_smoke ${water_test_targets} frazil_water_experiment_render frazil_water_performance)

if(MSVC AND CMAKE_CXX_FLAGS MATCHES "/fsanitize=address")
    get_filename_component(water_msvc_bin "${CMAKE_CXX_COMPILER}" DIRECTORY)
    set_property(TEST ${water_test_names} frazil_water_experiment_render_cli frazil_water_protect_listening
        PROPERTY ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${water_msvc_bin}")
    foreach(target IN LISTS water_test_targets)
        add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${water_msvc_bin}/clang_rt.asan_dynamic-x86_64.dll" "$<TARGET_FILE_DIR:${target}>" VERBATIM)
    endforeach()
    foreach(target IN ITEMS frazil_water_experiment_render frazil_water_performance)
        add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${water_msvc_bin}/clang_rt.asan_dynamic-x86_64.dll" "$<TARGET_FILE_DIR:${target}>" VERBATIM)
    endforeach()
endif()
