find_package(Qt6 REQUIRED COMPONENTS Test)

# Đường dẫn Qt bin để test exe tìm được DLL khi chạy qua ctest
get_target_property(_qt_bin Qt6::qmake IMPORTED_LOCATION)
get_filename_component(_qt_bin_dir "${_qt_bin}" DIRECTORY)

# ── Test: HourlyPricingStrategy ────────────────────────────────
qt_add_executable(tst_hourly tst_HourlyPricingStrategy.cpp)
target_include_directories(tst_hourly PRIVATE
    ${CMAKE_SOURCE_DIR}/src/models
    ${CMAKE_SOURCE_DIR}/src/strategies
)
target_link_libraries(tst_hourly PRIVATE parking_core Qt6::Test)
add_test(NAME tst_hourly COMMAND tst_hourly)
set_tests_properties(tst_hourly PROPERTIES
    ENVIRONMENT "PATH=${_qt_bin_dir};$ENV{PATH}"
)

# ── Test: MonthlyPassStrategy ──────────────────────────────────
qt_add_executable(tst_monthly tst_MonthlyPassStrategy.cpp)
target_include_directories(tst_monthly PRIVATE
    ${CMAKE_SOURCE_DIR}/src/models
    ${CMAKE_SOURCE_DIR}/src/strategies
)
target_link_libraries(tst_monthly PRIVATE parking_core Qt6::Test)
add_test(NAME tst_monthly COMMAND tst_monthly)
set_tests_properties(tst_monthly PROPERTIES
    ENVIRONMENT "PATH=${_qt_bin_dir};$ENV{PATH}"
)

# ── Test: PasswordHasher ───────────────────────────────────────
qt_add_executable(tst_password tst_PasswordHasher.cpp)
target_include_directories(tst_password PRIVATE
    ${CMAKE_SOURCE_DIR}/src/services
    ${CMAKE_SOURCE_DIR}/src/models
)
target_link_libraries(tst_password PRIVATE parking_core Qt6::Test)
add_test(NAME tst_password COMMAND tst_password)
set_tests_properties(tst_password PROPERTIES
    ENVIRONMENT "PATH=${_qt_bin_dir};$ENV{PATH}"
)
