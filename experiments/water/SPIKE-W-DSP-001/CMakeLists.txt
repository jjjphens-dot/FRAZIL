add_library(frazil_water_research STATIC
    dsp/ResearchBaseline.h
    ../../../src/dsp/primitives/RandomSource.cpp
)
target_include_directories(frazil_water_research PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}" "${PROJECT_SOURCE_DIR}/src")
target_compile_features(frazil_water_research PUBLIC cxx_std_20)

juce_add_console_app(frazil_water_experiment_render PRODUCT_NAME "FRAZIL Water Research Render")
target_sources(frazil_water_experiment_render PRIVATE render/render_main.cpp)
target_compile_definitions(frazil_water_experiment_render PRIVATE JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0)
target_link_libraries(frazil_water_experiment_render PRIVATE frazil_water_research juce::juce_audio_formats)

# CURRENT is independent from the archived Water/Preview test inventory.
if(FRAZIL_TEST_PROFILE MATCHES "^(CURRENT|ALL)$")
    include(CurrentTests.cmake)
endif()
if(FRAZIL_TEST_PROFILE MATCHES "^(HISTORICAL|ALL)$")
    include(HistoricalTests.cmake)
endif()
