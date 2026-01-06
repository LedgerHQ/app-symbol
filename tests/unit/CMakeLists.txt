cmake_minimum_required(VERSION 3.10)

project(SymbolTests)

set(APP_SRC_DIR "../../src")
set(BOLOS_SDK $ENV{BOLOS_SDK})

include_directories(
    ${APP_SRC_DIR}
    ${APP_SRC_DIR}/apdu
    ${APP_SRC_DIR}/xym
    ${APP_SRC_DIR}/xym/format
    ${APP_SRC_DIR}/xym/parse
    ${BOLOS_SDK}/include
    ${BOLOS_SDK}/lib_standard_app
    ${BOLOS_SDK}/target/nanox/include
)

# Build with code coverage generation
if(CODE_COVERAGE)
    if(CMAKE_C_COMPILER_ID MATCHES "(Apple)?[Cc]lang")
        add_compile_options(-fprofile-instr-generate -fcoverage-mapping)
        add_link_options(-fprofile-instr-generate -fcoverage-mapping)
    elseif(CMAKE_C_COMPILER_ID MATCHES "GNU")
        add_compile_options(-fprofile-arcs -ftest-coverage)
        link_libraries(gcov)
    else()
        message(FATAL_ERROR "Unsupported compiler used with code coverage generation")
    endif()
endif()

set(APP_SOURCES
    ${APP_SRC_DIR}/base32.c
    ${APP_SRC_DIR}/base32.h
    ${APP_SRC_DIR}/app_buffer.c
    ${APP_SRC_DIR}/app_buffer.h
    ${APP_SRC_DIR}/xym/xym_helpers.c
    ${APP_SRC_DIR}/xym/xym_helpers.h
    ${APP_SRC_DIR}/xym/format/fields.c
    ${APP_SRC_DIR}/xym/format/fields.h
    ${APP_SRC_DIR}/xym/format/app_format.c
    ${APP_SRC_DIR}/xym/format/app_format.h
    ${APP_SRC_DIR}/xym/format/printers.c
    ${APP_SRC_DIR}/xym/format/printers.h
    ${APP_SRC_DIR}/xym/parse/xym_parse.c
    ${APP_SRC_DIR}/xym/parse/xym_parse.h
    ${BOLOS_SDK}/lib_standard_app/read.c
    ${BOLOS_SDK}/lib_standard_app/write.c
    ${BOLOS_SDK}/lib_standard_app/varint.c
    ${BOLOS_SDK}/lib_standard_app/bip32.c
    ${BOLOS_SDK}/lib_standard_app/buffer.c
)

add_compile_definitions(
    TARGET_NANOX
)

add_executable(test_transaction_parser
    test_transaction_parser.c
    mock.c
    ${APP_SOURCES}
)

add_executable(test_bip32_path_extraction
    test_bip32_path_extraction.c
    ${APP_SRC_DIR}/app_buffer.c
)

target_compile_options(test_transaction_parser PRIVATE -Wall -Wextra -pedantic -Werror)
add_compile_definitions(test_transaction_parser PRIVATE FUZZ)

target_compile_options(test_bip32_path_extraction PRIVATE -Wall -Wextra -pedantic -Werror)
add_compile_definitions(test_bip32_path_extraction PRIVATE FUZZ)

target_include_directories(test_transaction_parser PRIVATE . ${APP_SRC_DIR}/ ${APP_SRC_DIR}/xym)
target_link_libraries(test_transaction_parser PRIVATE bsd)

target_include_directories(test_bip32_path_extraction PRIVATE . ${APP_SRC_DIR}/ ${APP_SRC_DIR}/xym)
target_link_libraries(test_bip32_path_extraction PRIVATE bsd cmocka)

if (FUZZ)
    # BOLOS SDK
    add_compile_definitions(IO_HID_EP_LENGTH=64)

    include_directories(.
            ${APP_SRC_DIR}/lib
            ${BOLOS_SDK}/include
            ${BOLOS_SDK}/lib_cxng/include
    )

    if (NOT CMAKE_C_COMPILER_ID MATCHES "Clang")
        message(FATAL_ERROR "Fuzzer needs to be built with Clang")
    endif()

    add_executable(fuzz_message fuzz_xym.c ${APP_SOURCES})

    target_compile_options(fuzz_message PRIVATE
        -fsanitize=fuzzer,address,undefined
        -fno-sanitize-recover=undefined
        -g -ggdb2)
    target_link_options(fuzz_message PRIVATE
        -fsanitize=fuzzer,address,undefined
        -fno-sanitize-recover=undefined)
    target_link_libraries(fuzz_message PRIVATE bsd)
endif()
