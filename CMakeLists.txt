cmake_minimum_required(VERSION 3.16)

# Guard against in-source builds
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR
        "In-source builds are forbidden. "
        "Please create a separate build directory (e.g. 'build/') and run CMake from there:\n"
        "  cmake -B build -S .\n"
        "  cmake --build build"
    )
endif()

set(CMAKE_C_COMPILER "C:/Qt/Tools/mingw1310_64/bin/gcc.exe" CACHE PATH "" FORCE)
set(CMAKE_CXX_COMPILER "C:/Qt/Tools/mingw1310_64/bin/g++.exe" CACHE PATH "" FORCE)

project(ParkingManagementSystem VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql)

set(SOURCES
    src/main.cpp
    src/models/Vehicle.cpp
    src/models/PricingModel.cpp
    src/strategies/HourlyPricingStrategy.cpp
    src/strategies/MonthlyPassStrategy.cpp
    src/db/DatabaseManager.cpp
    src/db/ParkingRepository.cpp
    src/services/PricingService.cpp
    src/services/ParkingManager.cpp
    src/services/SessionManager.cpp
    src/services/CameraService.cpp
    src/services/ANPRService.cpp
    src/services/HardwareController.cpp
    src/services/BankWebhookSimulator.cpp
    src/services/AutoGateManager.cpp
    src/ui/MainWindow.cpp
    src/ui/CheckInDialog.cpp
    src/ui/CheckOutDialog.cpp
    src/ui/SubscriptionDialog.cpp
    src/ui/PricingAdminDialog.cpp
    src/ui/HistoryWidget.cpp
    src/ui/LoginDialog.cpp
    src/ui/KioskGateDialog.cpp
    src/ui/SelfCheckoutDialog.cpp
    src/ui/MonthlyPassDialog.cpp
    src/ui/MaintenanceLogDialog.cpp
)

set(HEADERS
    src/models/VehicleType.h
    src/models/Vehicle.h
    src/models/ParkingSlot.h
    src/models/Ticket.h
    src/models/MonthlySubscription.h
    src/models/PricingConfig.h
    src/models/ParkingSession.h
    src/models/MonthlyPass.h
    src/models/User.h
    src/models/PricingModel.h
    src/strategies/IPricingStrategy.h
    src/strategies/HourlyPricingStrategy.h
    src/strategies/MonthlyPassStrategy.h
    src/db/DatabaseManager.h
    src/db/ParkingRepository.h
    src/services/PricingService.h
    src/services/ParkingManager.h
    src/services/SessionManager.h
    src/services/CameraService.h
    src/services/ANPRService.h
    src/services/HardwareController.h
    src/services/BankWebhookSimulator.h
    src/services/AutoGateManager.h
    src/ui/MainWindow.h
    src/ui/CheckInDialog.h
    src/ui/CheckOutDialog.h
    src/ui/SubscriptionDialog.h
    src/ui/PricingAdminDialog.h
    src/ui/HistoryWidget.h
    src/ui/LoginDialog.h
    src/ui/KioskGateDialog.h
    src/ui/SelfCheckoutDialog.h
    src/ui/MonthlyPassDialog.h
    src/ui/MaintenanceLogDialog.h
)

set(RESOURCES
    resources/app.qrc
)

add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

target_include_directories(${PROJECT_NAME} PRIVATE
    src
    src/models
    src/strategies
    src/db
    src/services
    src/ui
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Sql
)

if(WIN32 AND CMAKE_BUILD_TYPE STREQUAL "Release")
    set_property(TARGET ${PROJECT_NAME} PROPERTY WIN32_EXECUTABLE ON)
endif()

# ==============================================================================
# TỰ ĐỘNG COPY FILE .EXE MỚI SANG THƯ MỤC 'App' SAU KHI BUILD THÀNH CÔNG
# ==============================================================================
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_SOURCE_DIR}/App"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:${PROJECT_NAME}>"
        "${CMAKE_SOURCE_DIR}/App/$<TARGET_FILE_NAME:${PROJECT_NAME}>"
    COMMENT "Dang tu dong copy file .exe moi sang thu muc App..."
)