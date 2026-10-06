# Registry entries bind actual sources/tests. Adding C requires real implementations,
# not a placeholder test or a new set of hardcoded planner/workflow branches.
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/tests/current_modules.json")
file(READ "${PROJECT_SOURCE_DIR}/tests/current_modules.json" current_registry)
string(JSON module_count LENGTH "${current_registry}" modules)
if(module_count LESS 1)
    message(FATAL_ERROR "CURRENT registry cannot be empty")
endif()
add_custom_target(frazil_test_current)
add_custom_target(frazil_test_current_performance)
math(EXPR last_module "${module_count} - 1")
set(current_ids)
foreach(index RANGE ${last_module})
    string(JSON entry GET "${current_registry}" modules ${index})
    foreach(key IN ITEMS id native_target performance_target performance_source performance_kind)
        string(JSON ${key} GET "${entry}" ${key})
    endforeach()
    if(id IN_LIST current_ids OR NOT id MATCHES "^[a-z][a-z0-9]*$")
        message(FATAL_ERROR "Invalid/duplicate CURRENT module: ${id}")
    endif()
    list(APPEND current_ids ${id})
    string(JSON source_count LENGTH "${entry}" sources)
    math(EXPR last_source "${source_count} - 1")
    set(sources)
    foreach(source_index RANGE ${last_source})
        string(JSON source GET "${entry}" sources ${source_index})
        list(APPEND sources "${source}")
    endforeach()
    add_executable(${native_target} EXCLUDE_FROM_ALL ${sources})
    target_link_libraries(${native_target} PRIVATE frazil_water_research)
    add_executable(${performance_target} EXCLUDE_FROM_ALL ${performance_source})
    target_link_libraries(${performance_target} PRIVATE frazil_water_research)
    add_custom_target(frazil_test_${id}_current DEPENDS ${native_target} frazil_water_experiment_render)
    add_custom_target(frazil_test_${id}_performance DEPENDS ${performance_target})
    add_dependencies(frazil_test_current frazil_test_${id}_current)
    add_dependencies(frazil_test_current_performance frazil_test_${id}_performance)

    if(NOT FRAZIL_TEST_PURPOSE STREQUAL "performance" OR FRAZIL_TEST_PROFILE STREQUAL "ALL")
        add_test(NAME frazil_current_${id} COMMAND ${native_target} --current)
        add_test(NAME frazil_current_${id}_cli COMMAND "${Python3_EXECUTABLE}"
            "${CMAKE_CURRENT_SOURCE_DIR}/tests/current_cli_test.py"
            --module ${id} --renderer $<TARGET_FILE:frazil_water_experiment_render>)
        set_tests_properties(frazil_current_${id} frazil_current_${id}_cli PROPERTIES
            LABELS "current;current-${id};correctness;memory-safety;current-${id}-correctness;current-${id}-memory"
            TIMEOUT 60)
    endif()
    if(FRAZIL_TEST_PURPOSE STREQUAL "performance" OR FRAZIL_TEST_PROFILE STREQUAL "ALL")
        add_test(NAME frazil_current_${id}_performance COMMAND "${Python3_EXECUTABLE}"
            "${PROJECT_SOURCE_DIR}/tools/performance_observation.py"
            ${performance_kind} $<TARGET_FILE:${performance_target}>)
        set_tests_properties(frazil_current_${id}_performance PROPERTIES
            LABELS "current;current-${id};performance;current-${id}-performance" TIMEOUT 180)
    endif()
    if(MSVC AND CMAKE_CXX_FLAGS MATCHES "/fsanitize=address")
        get_filename_component(current_msvc_bin "${CMAKE_CXX_COMPILER}" DIRECTORY)
        foreach(target IN ITEMS ${native_target} ${performance_target})
            add_custom_command(TARGET ${target} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${current_msvc_bin}/clang_rt.asan_dynamic-x86_64.dll" "$<TARGET_FILE_DIR:${target}>" VERBATIM)
        endforeach()
    endif()
endforeach()
get_property(current_tests DIRECTORY PROPERTY TESTS)
file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/test-tmp")
foreach(test IN LISTS current_tests)
    set_property(TEST ${test} APPEND PROPERTY ENVIRONMENT_MODIFICATION
        "TEMP=set:${CMAKE_BINARY_DIR}/test-tmp" "TMP=set:${CMAKE_BINARY_DIR}/test-tmp")
    if(current_msvc_bin)
        set_property(TEST ${test} APPEND PROPERTY ENVIRONMENT_MODIFICATION
            "PATH=path_list_prepend:${current_msvc_bin}")
    endif()
endforeach()
if(current_msvc_bin)
    add_custom_command(TARGET frazil_water_experiment_render POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${current_msvc_bin}/clang_rt.asan_dynamic-x86_64.dll"
        "$<TARGET_FILE_DIR:frazil_water_experiment_render>" VERBATIM)
endif()
