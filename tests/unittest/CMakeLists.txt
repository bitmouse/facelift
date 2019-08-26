
if(${FACELIFT_USE_SYSTEM_GTEST})
    find_package(GTest REQUIRED)
    if(NOT ${GTEST_FOUND})
        message(FATAL_ERROR "Required package google test not found!")
    endif()
    set(FACELIFT_GTEST_LIBS ${GTEST_BOTH_LIBRARIES})
else()
    set(FACELIFT_GTEST_LIBS gtest gmock gtest_main)
endif()

add_executable(runUnitTests
    FaceliftUtilsTest.cpp
)

find_package(Threads)

# TODO: check thread library for Windows OS
target_link_libraries(runUnitTests
    ${FACELIFT_GTEST_LIBS}
    FaceliftCommonLib
    Threads::Threads)

add_test(NAME UnitTests COMMAND runUnitTests)
