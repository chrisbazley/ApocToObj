# RunTests.cmake
cmake_minimum_required(VERSION 3.11)

if(NOT APOCTOOBJ)
    set(APOCTOOBJ "./ApocToObj")
endif()

if(FORTIFY_FAILURE_TEST)
    set(TEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/ApocToObj-fortify-test")
else()
    set(TEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/ApocToObj-test")
endif()
set(APCOD "${TEST_DIR}/APCOD")
set(OUTPUT_DIR "${TEST_DIR}/output")
file(MAKE_DIRECTORY "${TEST_DIR}" "${OUTPUT_DIR}")

message(STATUS "Downloading the Apocalypse game data...")
file(DOWNLOAD
    "https://files.dcford.org.uk/download.php?id=f48d8bde8c74134652633d7173487ae0"
    "${APCOD}"
    EXPECTED_HASH
        SHA256=7c722257c773c9cbbf73309dc8962ce72f600f977255a3317ef42ace1b22bc7f
    STATUS download_status
    SHOW_PROGRESS
    TIMEOUT 60
)
list(GET download_status 0 download_result)
list(GET download_status 1 download_error)
if(NOT download_result EQUAL 0)
    message(FATAL_ERROR "Failed to download game data: ${download_error}")
endif()

function(body_hash file result_name)
    if(NOT EXISTS "${file}")
        message(FATAL_ERROR "${file} was not created")
    endif()
    file(READ "${file}" contents)
    string(REPLACE "\r\n" "\n" contents "${contents}")
    foreach(header_line RANGE 1 2)
        string(FIND "${contents}" "\n" newline)
        if(newline LESS 0)
            message(FATAL_ERROR "${file} has an incomplete header")
        endif()
        math(EXPR body_start "${newline} + 1")
        string(SUBSTRING "${contents}" ${body_start} -1 contents)
    endforeach()
    string(SHA256 hash "${contents}")
    set(${result_name} "${hash}" PARENT_SCOPE)
endfunction()

function(verify_body_checksum file description expected_hash)
    body_hash("${file}" actual_hash)
    if(NOT actual_hash STREQUAL expected_hash)
        message(FATAL_ERROR
            "Checksum mismatch for ${description}:\n"
            "  expected: ${expected_hash}\n"
            "  actual:   ${actual_hash}")
    endif()
    message(STATUS "Verified ${description}")
endfunction()

function(convert_and_verify output expected_hash)
    set(output_file "${OUTPUT_DIR}/${output}.obj")
    set(error_file "${OUTPUT_DIR}/${output}.stderr")
    message(STATUS "Converting ${output}.obj...")
    execute_process(
        COMMAND "${APOCTOOBJ}" ${ARGN} "${APCOD}" "${output_file}"
        RESULT_VARIABLE command_result
        OUTPUT_QUIET
        ERROR_FILE "${error_file}"
    )
    if(NOT command_result EQUAL 0)
        message(FATAL_ERROR
            "Conversion of ${output}.obj failed with code ${command_result}; "
            "see ${error_file}")
    endif()
    verify_body_checksum("${output_file}" "${output}.obj" "${expected_hash}")
endfunction()

function(expect_failure description expected_error)
    execute_process(
        COMMAND "${APOCTOOBJ}" ${ARGN}
        RESULT_VARIABLE command_result
        OUTPUT_VARIABLE command_stdout
        ERROR_VARIABLE command_stderr
    )
    if(command_result EQUAL 0)
        message(FATAL_ERROR "${description} unexpectedly succeeded")
    endif()
    if(NOT command_stderr MATCHES "${expected_error}")
        message(FATAL_ERROR
            "Unexpected error for ${description}: '${command_stderr}'")
    endif()
endfunction()

if(FORTIFY_FAILURE_TEST)
    # Flat 13 produces the smallest nonempty conversion from APCOD.
    set(reference_file "${OUTPUT_DIR}/fortify-reference.obj")
    set(output_file "${OUTPUT_DIR}/fortify.obj")
    set(fortify_args -flats -index 13 "${APCOD}")
    execute_process(
        COMMAND "${CMAKE_COMMAND}" -E env
            APOC_FORTIFY_FAILURE_SIMULATION=0
            "${APOCTOOBJ}" ${fortify_args} "${reference_file}"
        RESULT_VARIABLE command_result
        OUTPUT_QUIET ERROR_QUIET
    )
    if(NOT command_result EQUAL 0)
        message(FATAL_ERROR "Fortify reference conversion failed")
    endif()
    body_hash("${reference_file}" expected_hash)
    execute_process(
        COMMAND "${APOCTOOBJ}" ${fortify_args} "${output_file}"
        RESULT_VARIABLE command_result
        OUTPUT_FILE "${TEST_DIR}/fortify.stdout"
        ERROR_FILE "${TEST_DIR}/fortify.stderr"
    )
    if(NOT command_result EQUAL 0)
        message(FATAL_ERROR
            "Fortify failure simulation failed with code ${command_result}; "
            "see fortify.stdout and fortify.stderr in ${TEST_DIR}")
    endif()
    verify_body_checksum("${output_file}" "Fortify failure simulation output"
        "${expected_hash}")
    return()
endif()

# Complete conversions of both object tables using the principal output options.
convert_and_verify(apocalypse
    f7b4bbf92b8abee2b9db7e1d79c7b531952b212c52b5d1c970132067627d4855
    -human -negative -clip)
convert_and_verify(flats
    8874e3968a3029e8f9ce7148d1c635e57205fed7b1fe73b1e472c8380f3467ad
    -flats -flip -human -negative -clip)

if(DEBUG_OUTPUT)
    message(STATUS "Complete conversions passed with debug output")
    return()
endif()

# Alternative input and output syntax, all checked against the same object.
set(SAUCER_OPTIONS -name saucer_2 -human)
set(SAUCER_HASH c0e5a359335428dc451445cab4f9b8d8e4dd69bd83371cb449e921a352a43f23)

set(test_output "${OUTPUT_DIR}/saucer-positional.obj")
execute_process(COMMAND "${APOCTOOBJ}" ${SAUCER_OPTIONS}
    "${APCOD}" "${test_output}" RESULT_VARIABLE command_result)
if(NOT command_result EQUAL 0)
    message(FATAL_ERROR "Positional output conversion failed")
endif()
verify_body_checksum("${test_output}" "positional output" "${SAUCER_HASH}")

set(test_output "${OUTPUT_DIR}/saucer-outfile.obj")
execute_process(COMMAND "${APOCTOOBJ}" ${SAUCER_OPTIONS}
    -outfile "${test_output}" "${APCOD}" RESULT_VARIABLE command_result)
if(NOT command_result EQUAL 0)
    message(FATAL_ERROR "-outfile conversion failed")
endif()
verify_body_checksum("${test_output}" "-outfile output" "${SAUCER_HASH}")

set(test_output "${OUTPUT_DIR}/saucer-stdin.obj")
execute_process(COMMAND "${APOCTOOBJ}" ${SAUCER_OPTIONS}
    -outfile "${test_output}" INPUT_FILE "${APCOD}"
    RESULT_VARIABLE command_result)
if(NOT command_result EQUAL 0)
    message(FATAL_ERROR "Input from stdin failed")
endif()
verify_body_checksum("${test_output}" "stdin output" "${SAUCER_HASH}")

set(test_output "${OUTPUT_DIR}/saucer-stdout.obj")
execute_process(COMMAND "${APOCTOOBJ}" ${SAUCER_OPTIONS} "${APCOD}"
    OUTPUT_FILE "${test_output}" RESULT_VARIABLE command_result)
if(NOT command_result EQUAL 0)
    message(FATAL_ERROR "Output to stdout failed")
endif()
verify_body_checksum("${test_output}" "stdout output" "${SAUCER_HASH}")

set(test_output "${OUTPUT_DIR}/saucer-stdio.obj")
execute_process(COMMAND "${APOCTOOBJ}" ${SAUCER_OPTIONS}
    INPUT_FILE "${APCOD}" OUTPUT_FILE "${test_output}"
    RESULT_VARIABLE command_result)
if(NOT command_result EQUAL 0)
    message(FATAL_ERROR "stdin/stdout conversion failed")
endif()
verify_body_checksum("${test_output}" "stdin/stdout output" "${SAUCER_HASH}")

# Selection syntax and abbreviated switches must reproduce the same output.
foreach(option_set IN ITEMS
        "-index;31;-human"
        "-first;31;-last;31;-human"
        "-offset;0x10C6C;-index;31;-human"
        "-na;saucer_2;-hu")
    set(test_output "${OUTPUT_DIR}/selection.obj")
    execute_process(COMMAND "${APOCTOOBJ}" ${option_set}
        -outfile "${test_output}" "${APCOD}"
        RESULT_VARIABLE command_result)
    if(NOT command_result EQUAL 0)
        message(FATAL_ERROR "Selection options '${option_set}' failed")
    endif()
    verify_body_checksum("${test_output}" "options '${option_set}'"
        "${SAUCER_HASH}")
endforeach()

# Remaining output switches, using cases where the option affects the result.
convert_and_verify(false-colour
    6c4ea7c493891f9d01358f7cf1672bf84c0056ea5e6e90dbf55eed02c4a4f733
    -index 31 -false)
convert_and_verify(unused
    489d211bba488f3e74459afbe7a4bc04b272864e884641b3e562f92be37e4665
    -index 163 -unused)
convert_and_verify(duplicate
    5cbb671fe851db41315dcdb7565ffeecbb9f97c3772dfe63d354ae757f2ed04d
    -index 22 -duplicate)
convert_and_verify(negative
    d8b0c1f349d260747d8a21ec6f9db10b4eed248a8c04f0e8f022ce280554ed8c
    -index 31 -negative)
convert_and_verify(clipped
    8da1b527d71836cb1675a832e5785ab3f32129b9c305b3d44fcb408102b3a031
    -index 0 -clip)
convert_and_verify(fans
    91ababa9fb2dbdf67b2cdf40c4bbc459c4351b82c202f94d91596d4cbd0509fd
    -index 31 -fans)
convert_and_verify(strips
    4bd7d83af777d6444f5a2991dfc7316a8ea12f650931236f54f69c59bf7f7a0f
    -index 31 -strips)
convert_and_verify(material-library
    601225226f4a5d9c31bcceefd427fac00b03d766f6653c2f7ec65fd1d538e0f1
    -index 31 -mtllib alternative.mtl)
convert_and_verify(flipped-flat
    0de435091aa807c55dd5405689cc164ae1bfd7a35e047c55a70b2b2f10568a0b
    -flats -flip -index 13)

# Batch mode generates output beside each input file.
set(batch_one "${TEST_DIR}/batch-one")
set(batch_two "${TEST_DIR}/batch-two")
configure_file("${APCOD}" "${batch_one}" COPYONLY)
configure_file("${APCOD}" "${batch_two}" COPYONLY)
execute_process(COMMAND "${APOCTOOBJ}" -batch ${SAUCER_OPTIONS}
    "${batch_one}" "${batch_two}" RESULT_VARIABLE command_result)
if(NOT command_result EQUAL 0)
    message(FATAL_ERROR "Batch conversion failed")
endif()
verify_body_checksum("${batch_one}.obj" "first batch output" "${SAUCER_HASH}")
verify_body_checksum("${batch_two}.obj" "second batch output" "${SAUCER_HASH}")

# Listing, help and diagnostic modes have non-OBJ output as well as checked OBJ.
execute_process(COMMAND "${APOCTOOBJ}" -list -first 31 -last 32 "${APCOD}"
    RESULT_VARIABLE command_result OUTPUT_VARIABLE command_stdout)
if(NOT command_result EQUAL 0 OR
   NOT command_stdout MATCHES "saucer_2" OR
   NOT command_stdout MATCHES "apocalypse_32")
    message(FATAL_ERROR "List output was unsuccessful or malformed")
endif()

execute_process(COMMAND "${APOCTOOBJ}" -help RESULT_VARIABLE command_result
    OUTPUT_VARIABLE command_stdout)
if(NOT command_result EQUAL 0 OR NOT command_stdout MATCHES "usage:")
    message(FATAL_ERROR "Help output was unsuccessful or malformed")
endif()

foreach(option IN ITEMS -verbose -debug -time)
    set(test_output "${OUTPUT_DIR}/diagnostic.obj")
    execute_process(COMMAND "${APOCTOOBJ}" ${option} ${SAUCER_OPTIONS}
        -outfile "${test_output}" "${APCOD}"
        RESULT_VARIABLE command_result OUTPUT_VARIABLE command_stdout)
    if(NOT command_result EQUAL 0)
        message(FATAL_ERROR "${option} conversion failed")
    endif()
    verify_body_checksum("${test_output}" "${option} output" "${SAUCER_HASH}")
    if(option STREQUAL "-time" AND
       NOT command_stdout MATCHES "Time taken: [0-9]+\\.[0-9]+ seconds")
        message(FATAL_ERROR "Timer output was malformed")
    endif()
endforeach()

# Invalid command lines.
expect_failure("unknown switch" "Unrecognised switch" -unknown)
expect_failure("missing output name" "Missing output file name" -outfile)
expect_failure("missing object name" "Missing object name" -name)
expect_failure("missing material library name"
    "Missing materials library file name" -mtllib)
expect_failure("reversed object range"
    "First object number must not exceed last object number" -first 2 -last 1)
expect_failure("fan and strip output together"
    "Cannot split polygons into both triangle fans and strips" -fans -strips)
expect_failure("batch output name" "Cannot specify an output file"
    -batch -outfile one "${APCOD}")
expect_failure("empty batch" "Must specify file.* in batch" -batch)
expect_failure("two output names" "Cannot specify more than one output file"
    -outfile one "${APCOD}" two)
expect_failure("output in list mode" "Cannot specify an output file in list mode"
    -list "${APCOD}" unused)
expect_failure("verbose output sent to stdout"
    "Must specify an output file in verbose/timer mode" -verbose "${APCOD}")
expect_failure("too many arguments" "Too many arguments" "${APCOD}" one two)

message(STATUS "All ApocToObj integration tests passed")
