cmake_minimum_required(VERSION 3.10)

if(${CMAKE_VERSION} VERSION_LESS 3.10)
    cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

# project information
project(unit_tests
        VERSION 0.1
	      DESCRIPTION "Unit tests for Ledger Nano application"
        LANGUAGES C)


# guard against bad build-type strings
if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

include(CTest)
ENABLE_TESTING()

# specify C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED True)
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -pedantic -g -O0 --coverage")

set(GCC_COVERAGE_LINK_FLAGS "--coverage -lgcov")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${GCC_COVERAGE_LINK_FLAGS}")

# guard against in-source builds
if(${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there. You may need to remove CMakeCache.txt. ")
endif()

add_compile_definitions(TEST)

include_directories(../src)

add_executable(test_base58 test_base58.c)
add_executable(test_bip32 test_bip32.c)
add_executable(test_buffer test_buffer.c)
add_executable(test_format test_format.c)
add_executable(test_write test_write.c)
add_executable(test_apdu_parser test_apdu_parser.c)
add_executable(test_bits test_bits.c)
# add_executable(test_tx_parser test_tx_parser.c)
add_executable(test_tx_utils test_tx_utils.c)

add_library(base58 SHARED ../src/common/base58.c)
add_library(bip32 SHARED ../src/common/bip32.c)
add_library(buffer SHARED ../src/common/buffer.c)
add_library(read SHARED ../src/common/read.c)
add_library(write SHARED ../src/common/write.c)
add_library(bits SHARED ../src/common/bits.c)
add_library(format SHARED ../src/common/format.c)
add_library(varint SHARED ../src/common/varint.c)
add_library(apdu_parser SHARED ../src/apdu/parser.c)
add_library(transaction_deserialize ../src/transaction/deserialize.c)
add_library(transaction_utils ../src/transaction/utils.c)

target_link_libraries(test_base58 PUBLIC cmocka gcov base58)
target_link_libraries(test_bip32 PUBLIC cmocka gcov bip32 read)
target_link_libraries(test_buffer PUBLIC cmocka gcov buffer bip32 varint write read)
target_link_libraries(test_format PUBLIC cmocka gcov format)
target_link_libraries(test_write PUBLIC cmocka gcov write)
target_link_libraries(test_bits PUBLIC cmocka gcov bits)
target_link_libraries(test_apdu_parser PUBLIC cmocka gcov apdu_parser)
# target_link_libraries(test_tx_parser PUBLIC
#                      transaction_deserialize
#                      buffer
#                      bip32
#                      cmocka
#                      gcov
#                      write
#                      read
#                      varint
#                      transaction_utils)
target_link_libraries(test_tx_utils PUBLIC
                      cmocka
                      gcov
                      transaction_utils)

add_test(test_base58 test_base58)
add_test(test_bip32 test_bip32)
add_test(test_buffer test_buffer)
add_test(test_format test_format)
add_test(test_write test_write)
add_test(test_bits test_bits)
add_test(test_apdu_parser test_apdu_parser)
# add_test(test_tx_parser test_tx_parser)
add_test(test_tx_utils test_tx_utils)
