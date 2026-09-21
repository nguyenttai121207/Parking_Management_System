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

project(ParkingManagementSystem VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql)

set(SOURCES
    src/main.cpp
    src/models/Vehicle.cpp
    src/strategies/HourlyPricingStrategy.cpp
    src/strategies/MonthlyPassStrategy.cpp
    src/db/DatabaseManager.cpp
    src/db/ParkingRepository.cpp
    src/services/PricingService.cpp
    src/services/ParkingManager.cpp
    src/ui/MainWindow.cpp
    src/ui/CheckInDialog.cpp
    src/ui/CheckOutDialog.cpp
    src/ui/SubscriptionDialog.cpp
    src/ui/PricingAdminDialog.cpp
    src/ui/HistoryWidget.cpp
)

set(HEADERS
    src/models/VehicleType.h
    src/models/Vehicle.h
    src/models/ParkingSlot.h
    src/models/Ticket.h
    src/models/MonthlySubscription.h
    src/models/PricingConfig.h
    src/strategies/IPricingStrategy.h
    src/strategies/HourlyPricingStrategy.h
    src/strategies/MonthlyPassStrategy.h
    src/db/DatabaseManager.h
    src/db/ParkingRepository.h
    src/services/PricingService.h
    src/services/ParkingManager.h
    src/ui/MainWindow.h
    src/ui/CheckInDialog.h
    src/ui/CheckOutDialog.h
    src/ui/SubscriptionDialog.h
    src/ui/PricingAdminDialog.h
    src/ui/HistoryWidget.h
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
