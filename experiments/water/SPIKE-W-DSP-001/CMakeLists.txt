add_library(frazil_water_research STATIC
    dsp/ResearchBaseline.h
    ../../../src/dsp/primitives/RandomSource.cpp
)
target_include_directories(frazil_water_research PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/src")
target_compile_features(frazil_water_research PUBLIC cxx_std_20)

set(water_test_targets)
set(water_test_names)
foreach(module IN ITEMS baseline features modal excitation normalization motion bubble bubble_a1 bubble_a1_cull_gate droplet_b1_physics droplet_b1_onset droplet_b1 droplet_b1_allocation droplet_b2 flow flow_d1 droplet activity fluid event_pool protect_detector protect mapping)
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
target_sources(frazil_water_bubble_a1_tests PRIVATE tests/AllocationObserver.cpp)
target_sources(frazil_water_droplet_b1_allocation_tests PRIVATE tests/AllocationObserver.cpp)
target_sources(frazil_water_droplet_b2_tests PRIVATE tests/AllocationObserver.cpp)

add_executable(frazil_water_flow_d1_latency_native tests/flow_d1_latency_native.cpp
    tests/AllocationObserver.cpp)
target_link_libraries(frazil_water_flow_d1_latency_native PRIVATE frazil_water_research)
list(APPEND water_test_targets frazil_water_flow_d1_latency_native)
add_test(NAME frazil_water_flow_d1_latency_native
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/flow_d1_latency_native_test.py"
            $<TARGET_FILE:frazil_water_flow_d1_latency_native>)
list(APPEND water_test_names frazil_water_flow_d1_latency_native)
add_test(NAME frazil_water_evidence_tools
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/evidence_tools_test.py")
list(APPEND water_test_names frazil_water_evidence_tools)

juce_add_console_app(frazil_water_experiment_render PRODUCT_NAME "FRAZIL Water Research Render")
target_sources(frazil_water_experiment_render PRIVATE render/render_main.cpp)
target_compile_definitions(frazil_water_experiment_render PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
target_link_libraries(frazil_water_experiment_render PRIVATE frazil_water_research juce::juce_audio_formats)
juce_add_console_app(frazil_water_research_cases PRODUCT_NAME "FRAZIL Water Research Cases")
target_sources(frazil_water_research_cases PRIVATE render/research_cases.cpp)
target_compile_definitions(frazil_water_research_cases PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
target_link_libraries(frazil_water_research_cases PRIVATE frazil_water_research juce::juce_core)
add_test(NAME frazil_water_experiment_render_cli
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/render_cli_test.py"
            $<TARGET_FILE:frazil_water_experiment_render>)
add_test(NAME frazil_water_protect_listening
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/protect_listening_test.py"
            $<TARGET_FILE:frazil_water_experiment_render>)
add_test(NAME frazil_water_bubble_a1_cli
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/bubble_a1_cli_test.py"
            $<TARGET_FILE:frazil_water_experiment_render>)

add_test(NAME frazil_water_droplet_b2_cli
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/droplet_b2_cli_test.py"
            $<TARGET_FILE:frazil_water_experiment_render>)

add_test(NAME frazil_water_droplet_b1_cli
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/droplet_b1_cli_test.py"
            $<TARGET_FILE:frazil_water_experiment_render>)

add_test(NAME frazil_water_flow_d1_cli
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/flow_d1_cli_test.py"
            $<TARGET_FILE:frazil_water_experiment_render>
            $<TARGET_FILE:frazil_water_flow_d1_source_probe>)
add_test(NAME frazil_water_flow_d1_remediation
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/flow_d1_remediation_test.py"
            $<TARGET_FILE:frazil_water_flow_d1_source_probe>)
add_test(NAME frazil_water_flow_d1_convergence
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/flow_d1_convergence_test.py"
            --probe $<TARGET_FILE:frazil_water_flow_d1_source_probe>)

add_test(NAME frazil_water_flow_d1_latency
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/flow_d1_latency_test.py")

add_executable(frazil_water_performance tests/performance.cpp
    ../../../src/app/AudioEngine.cpp ../../../src/dsp/DryWetMixer.cpp
    ../../../src/dsp/primitives/LinearSmoother.cpp)
target_link_libraries(frazil_water_performance PRIVATE frazil_water_research juce::juce_audio_basics)
add_executable(frazil_water_bubble_a1_performance tests/bubble_a1_performance.cpp)
target_link_libraries(frazil_water_bubble_a1_performance PRIVATE frazil_water_research)
add_executable(frazil_water_droplet_b1_performance tests/droplet_b1_performance.cpp)
target_link_libraries(frazil_water_droplet_b1_performance PRIVATE frazil_water_research)

add_executable(frazil_water_flow_d1_performance tests/flow_d1_performance.cpp)
target_link_libraries(frazil_water_flow_d1_performance PRIVATE frazil_water_research)
add_executable(frazil_water_flow_d1_source_probe tests/flow_d1_source_probe.cpp)
target_link_libraries(frazil_water_flow_d1_source_probe PRIVATE frazil_water_research)

# Separate research application. It is never linked into FRAZIL or registered as a Host plugin.
if(FRAZIL_BUILD_WATER_PREVIEW)
    set(FRAZIL_PREVIEW_VARIANT "${CMAKE_BUILD_TYPE}")
    if(CMAKE_CXX_FLAGS MATCHES "/fsanitize=address")
        string(APPEND FRAZIL_PREVIEW_VARIANT "+ASAN")
    endif()
    configure_file(preview/PreviewBuildInfo.h.in generated/PreviewBuildInfo.h @ONLY)
    juce_add_gui_app(frazil_water_preview PRODUCT_NAME "FRAZIL Water Research Preview")
    target_sources(frazil_water_preview PRIVATE preview/PreviewMain.cpp preview/PreviewController.cpp
        ../../../src/ui/DeveloperDiagnosticsView.cpp ../../../src/ui/DeveloperLevelMeter.cpp
        ../../../src/dsp/primitives/LinearSmoother.cpp)
    target_compile_definitions(frazil_water_preview PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0
        FRAZIL_ENABLE_DEVELOPER_UI=1)
    target_link_libraries(frazil_water_preview PRIVATE frazil_water_research juce::juce_audio_utils)

    juce_add_console_app(frazil_water_preview_tests PRODUCT_NAME "FRAZIL Water Preview Tests")
    target_sources(frazil_water_preview_tests PRIVATE tests/preview_tests.cpp tests/preview_time_tests.cpp
        tests/preview_descriptor_tests.cpp tests/preview_session_tests.cpp tests/preview_session_codec_tests.cpp
        tests/preview_protect_tests.cpp tests/preview_diagnostics_tests.cpp tests/preview_workflow_tests.cpp
        tests/preview_operation_tests.cpp tests/preview_audition_tests.cpp tests/preview_audition_workflow_tests.cpp
        tests/preview_reworked_core_tests.cpp tests/preview_reworked_parameter_tests.cpp
        tests/preview_monitor_rate_tests.cpp
        preview/PreviewController.cpp
        ../../../src/dsp/primitives/LinearSmoother.cpp)
    target_compile_definitions(frazil_water_preview_tests PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0
        FRAZIL_ENABLE_DEVELOPER_UI=1)
    target_link_libraries(frazil_water_preview_tests PRIVATE frazil_water_research juce::juce_audio_utils)
    foreach(target IN ITEMS frazil_water_preview frazil_water_preview_tests)
        target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/generated")
    endforeach()
    add_test(NAME frazil_water_preview COMMAND frazil_water_preview_tests)
    list(APPEND water_test_targets frazil_water_preview frazil_water_preview_tests)
    list(APPEND water_test_names frazil_water_preview)
endif()


# P0 preserves every existing registration and assertion. A1 and B2 correctness
# executables are bounded regression workloads; retain their complete bodies in Fast.
# B2 prints diagnostic timing but never gates on it (separation remains P2).
# Mixed CLI/study suites and the extended B1 matrix remain Slow.
set_tests_properties(frazil_water_baseline PROPERTIES LABELS "water-common;fast;property")
set_tests_properties(frazil_water_features PROPERTIES LABELS "water-common;fast;property")
set_tests_properties(frazil_water_modal PROPERTIES LABELS "water-common;fast;property")
set_tests_properties(frazil_water_excitation PROPERTIES LABELS "water-common;fast;property")
set_tests_properties(frazil_water_normalization PROPERTIES LABELS "water-common;fast;property")
set_tests_properties(frazil_water_motion PROPERTIES LABELS "water-common;fast;property")
set_tests_properties(frazil_water_bubble PROPERTIES LABELS "water-common;fast;property")
set_tests_properties(frazil_water_flow PROPERTIES LABELS "water-common;fast;property")
set_tests_properties(frazil_water_droplet PROPERTIES LABELS "water-common;fast;property")
set_tests_properties(frazil_water_activity PROPERTIES LABELS "water-common;fast;property")
set_tests_properties(frazil_water_fluid PROPERTIES LABELS "water-common;fast;property")
set_tests_properties(frazil_water_event_pool PROPERTIES LABELS "water-common;fast;property")
set_tests_properties(frazil_water_mapping PROPERTIES LABELS "water-common;fast;property")
set_tests_properties(frazil_water_bubble_a1 PROPERTIES LABELS "water-a1;fast;property")
set_tests_properties(frazil_water_bubble_a1_cull_gate PROPERTIES LABELS "water-a1;slow;property;evidence;research")
set_tests_properties(frazil_water_droplet_b1_physics PROPERTIES LABELS "water-b1;fast;unit")
set_tests_properties(frazil_water_droplet_b1_onset PROPERTIES LABELS "water-b1;fast;unit")
set_tests_properties(frazil_water_droplet_b1 PROPERTIES LABELS "water-b1;slow;property;research")
set_tests_properties(frazil_water_droplet_b1_allocation PROPERTIES LABELS "water-common;water-b1;water-d1;fast;property")
set_tests_properties(frazil_water_droplet_b2 PROPERTIES LABELS "water-b2;fast;property")
set_tests_properties(frazil_water_flow_d1 PROPERTIES LABELS "water-d1;fast;property")
set_tests_properties(frazil_water_protect_detector PROPERTIES LABELS "water-protect;fast;unit")
set_tests_properties(frazil_water_protect PROPERTIES LABELS "water-protect;fast;property")
set_tests_properties(frazil_water_flow_d1_latency_native PROPERTIES LABELS "water-d1;slow;native;research")
set_tests_properties(frazil_water_evidence_tools PROPERTIES LABELS "water-common;research-validation;slow;unit;evidence;research")
set_tests_properties(frazil_water_experiment_render_cli PROPERTIES LABELS "water-common;water-protect;slow;cli;render;research")
set_tests_properties(frazil_water_protect_listening PROPERTIES LABELS "water-protect;slow;listening;research")
set_tests_properties(frazil_water_bubble_a1_cli PROPERTIES LABELS "water-a1;slow;cli;render;research")
set_tests_properties(frazil_water_droplet_b2_cli PROPERTIES LABELS "water-b2;slow;cli;render;research")
set_tests_properties(frazil_water_droplet_b1_cli PROPERTIES LABELS "water-b1;slow;cli;render;listening;research")
set_tests_properties(frazil_water_flow_d1_cli PROPERTIES LABELS "water-d1;slow;cli;native;research")
set_tests_properties(frazil_water_flow_d1_remediation PROPERTIES LABELS "water-d1;slow;native;research")
set_tests_properties(frazil_water_flow_d1_convergence PROPERTIES LABELS "water-d1;slow;native;research")
set_tests_properties(frazil_water_flow_d1_latency PROPERTIES LABELS "water-d1;slow;unit;research")
if(FRAZIL_BUILD_WATER_PREVIEW)
    set_tests_properties(frazil_water_preview PROPERTIES LABELS "water-preview;fast;integration")
endif()

# Derived labels encode module AND tier for CTest presets (whose label field is
# one regex). Keep module/tier/kind authoritative; never maintain aliases by hand.
get_property(water_registered_tests DIRECTORY PROPERTY TESTS)
foreach(water_test IN LISTS water_registered_tests)
    get_test_property(${water_test} LABELS water_labels)
    if("fast" IN_LIST water_labels)
        foreach(water_label IN LISTS water_labels)
            if(water_label MATCHES "^water-")
                set_property(TEST ${water_test} APPEND PROPERTY LABELS "fast-${water_label}")
            endif()
        endforeach()
    endif()
endforeach()

add_custom_target(frazil_water_fast_tests DEPENDS
    frazil_water_bubble_a1_tests
    frazil_water_droplet_b2_tests
    frazil_water_experiment_tests
    frazil_water_features_tests
    frazil_water_modal_tests
    frazil_water_excitation_tests
    frazil_water_normalization_tests
    frazil_water_motion_tests
    frazil_water_bubble_tests
    frazil_water_flow_tests
    frazil_water_droplet_tests
    frazil_water_activity_tests
    frazil_water_fluid_tests
    frazil_water_event_pool_tests
    frazil_water_mapping_tests
    frazil_water_droplet_b1_physics_tests
    frazil_water_droplet_b1_onset_tests
    frazil_water_droplet_b1_allocation_tests
    frazil_water_flow_d1_tests
    frazil_water_protect_detector_tests
    frazil_water_protect_tests)
if(FRAZIL_BUILD_WATER_PREVIEW)
    add_dependencies(frazil_water_fast_tests frazil_water_preview_tests)
endif()

# Full also retains manual study/benchmark executables from the former smoke closure.
add_custom_target(frazil_water_full_tests DEPENDS ${water_test_targets}
    frazil_water_experiment_render frazil_water_performance frazil_water_research_cases
    frazil_water_bubble_a1_performance frazil_water_droplet_b1_performance
    frazil_water_flow_d1_performance frazil_water_flow_d1_source_probe)
# Module build groups match the module-fast test presets, not the full label.
add_custom_target(frazil_water_common_test_group DEPENDS
    frazil_water_experiment_tests
    frazil_water_features_tests
    frazil_water_modal_tests
    frazil_water_excitation_tests
    frazil_water_normalization_tests
    frazil_water_motion_tests
    frazil_water_bubble_tests
    frazil_water_flow_tests
    frazil_water_droplet_tests
    frazil_water_activity_tests
    frazil_water_fluid_tests
    frazil_water_event_pool_tests
    frazil_water_mapping_tests
    frazil_water_droplet_b1_allocation_tests)
add_custom_target(frazil_water_a1_test_group DEPENDS
    frazil_water_bubble_a1_tests)
add_custom_target(frazil_water_b1_test_group DEPENDS
    frazil_water_droplet_b1_physics_tests
    frazil_water_droplet_b1_onset_tests
    frazil_water_droplet_b1_allocation_tests)
add_custom_target(frazil_water_b2_test_group DEPENDS
    frazil_water_droplet_b2_tests)
add_custom_target(frazil_water_d1_test_group DEPENDS
    frazil_water_flow_d1_tests
    frazil_water_droplet_b1_allocation_tests)
add_custom_target(frazil_water_protect_test_group DEPENDS
    frazil_water_protect_detector_tests
    frazil_water_protect_tests)
if(FRAZIL_BUILD_WATER_PREVIEW)
    add_custom_target(frazil_water_preview_test_group DEPENDS frazil_water_preview_tests)
endif()

if(MSVC AND CMAKE_CXX_FLAGS MATCHES "/fsanitize=address")
    get_filename_component(water_msvc_bin "${CMAKE_CXX_COMPILER}" DIRECTORY)
    set_property(TEST ${water_test_names} frazil_water_experiment_render_cli frazil_water_protect_listening frazil_water_bubble_a1_cli frazil_water_droplet_b1_cli frazil_water_droplet_b2_cli frazil_water_flow_d1_cli
        PROPERTY ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${water_msvc_bin}")
    foreach(target IN LISTS water_test_targets)
        add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${water_msvc_bin}/clang_rt.asan_dynamic-x86_64.dll" "$<TARGET_FILE_DIR:${target}>" VERBATIM)
    endforeach()
    foreach(target IN ITEMS frazil_water_experiment_render frazil_water_performance frazil_water_research_cases frazil_water_bubble_a1_performance frazil_water_droplet_b1_performance frazil_water_flow_d1_performance frazil_water_flow_d1_source_probe)
        add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${water_msvc_bin}/clang_rt.asan_dynamic-x86_64.dll" "$<TARGET_FILE_DIR:${target}>" VERBATIM)
    endforeach()
endif()
