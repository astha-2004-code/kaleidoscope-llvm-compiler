# Unit tests configuration
set(TEST_SOURCES
    test_main.cpp
    test_lexer.cpp
    test_parser.cpp
    test_ast_optimizer.cpp
    test_codegen.cpp
)


add_executable(kaleidoscope_tests ${TEST_SOURCES})
target_link_libraries(kaleidoscope_tests PRIVATE kaleidoscope_core)

add_test(NAME AllCompilerTests COMMAND kaleidoscope_tests)
