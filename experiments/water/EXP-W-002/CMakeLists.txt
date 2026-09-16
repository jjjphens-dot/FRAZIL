add_library(frazil_water_research STATIC
    dsp/ResearchBaseline.h
    ../../../src/dsp/primitives/RandomSource.cpp
)
target_include_directories(frazil_water_research PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/src")
target_compile_features(frazil_water_research PUBLIC cxx_std_20)

add_executable(frazil_water_experiment_tests tests/baseline_tests.cpp)
target_link_libraries(frazil_water_experiment_tests PRIVATE frazil_water_research)
add_test(NAME frazil_water_experiment_unit COMMAND frazil_water_experiment_tests)

juce_add_console_app(frazil_water_experiment_render PRODUCT_NAME "FRAZIL Water Research Render")
target_sources(frazil_water_experiment_render PRIVATE render/render_main.cpp)
target_compile_definitions(frazil_water_experiment_render PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
target_link_libraries(frazil_water_experiment_render PRIVATE frazil_water_research juce::juce_audio_formats)
add_test(NAME frazil_water_experiment_render_cli
    COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/tests/render_cli_test.py"
            $<TARGET_FILE:frazil_water_experiment_render>)

if(MSVC AND CMAKE_CXX_FLAGS MATCHES "/fsanitize=address")
    get_filename_component(water_msvc_bin "${CMAKE_CXX_COMPILER}" DIRECTORY)
    set_property(TEST frazil_water_experiment_unit frazil_water_experiment_render_cli
        PROPERTY ENVIRONMENT_MODIFICATION "PATH=path_list_prepend:${water_msvc_bin}")
endif()

# Include opted-in research in existing safe build presets without changing their target lists.
add_dependencies(frazil_smoke frazil_water_experiment_tests frazil_water_experiment_render)
