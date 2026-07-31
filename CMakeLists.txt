cmake_minimum_required(VERSION 3.16)

project(SIS
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Gui
    Widgets
)

qt_standard_project_setup()

qt_add_executable(${PROJECT_NAME}

    src/controller/PlcController.h
    src/controller/PlcController.cpp

    src/core/ErrorType.h
    src/core/LogLevel.h
    src/core/State.h
    src/core/StateMachine.h
    src/core/StateMachine.cpp

    src/model/MeasurementRecord.h
    src/model/MeasurementInfo.h
    src/model/Recipe.cpp
    src/model/Recipe.h
    src/model/MeasurementResult.h

    src/repository/CsvWriter.h
    src/repository/CsvWriter.cpp
    src/repository/LogWriter.h
    src/repository/LogWriter.cpp
    src/repository/RecipeRepository.cpp
    src/repository/RecipeRepository.h

    src/ui/MainWindow.cpp
    src/ui/MainWindow.h
    src/ui/MainWindow.ui
    src/ui/RecipeDialog.cpp
    src/ui/RecipeDialog.h
    src/ui/RecipeDialog.ui

    src/validator/MeasurementValidator.h
    src/validator/MeasurementValidator.cpp
    src/validator/RecipeValidator.h
    src/validator/RecipeValidator.cpp

    src/widgets/StatusLamp.cpp
    src/widgets/StatusLamp.h

    src/Main.cpp
)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        src
        src/widgets
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE TRUE
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)