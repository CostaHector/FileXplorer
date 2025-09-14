cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME FileXplorerTest)
project(${PROJECT_NAME} LANGUAGES CXX)
# on windows platform. MediaInfoDLL.h defined a typdef UNICODE _UNICODE needed
get_filename_component(PARENT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
string(LENGTH "${PARENT_DIR}" PARENT_DIR_LENGTH)
math(EXPR RELATIVE_INDEX_START_AT "${PARENT_DIR_LENGTH} + 1")
message("absFilePath(CMakeLists.txt)=${CMAKE_CURRENT_SOURCE_DIR}")
message("absolutePath(CMakeLists.txt)=${PARENT_DIR}")
message("relativeIndexStartAt=${RELATIVE_INDEX_START_AT}")

add_definitions(
    -DPROJECT_NAME="FileXplorer"
    -DPARENT_PATH_LEN=${RELATIVE_INDEX_START_AT}
    -DRUNNING_UNIT_TESTS=1
    -DUNICODE
    -D_UNICODE
)
message("[${PROJECT_NAME}] Start...")
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Search for Qt libraries in the order of Qt6, Qt5
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Test Core Gui Sql Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test Core Gui Sql Widgets)


file(GLOB SOURCES
    # service module sources
    ../Actions/*.cpp
    ../Model/*.cpp
    ../View/*.cpp
    ../Component/*.cpp
    ../Component/FolderPreview/*.cpp
    ../Component/JsonEditorWidget/*.cpp
    ../Component/RenameWidgets/*.cpp
    ../FileOperation/*.cpp
    ../Model/*.cpp
    ../public/*.cpp
    ../public/Memory/*.cpp
    ../Tools/*.cpp
    ../Tools/Classify/*.cpp
    ../Tools/FileDescriptor/*.cpp
    ../Tools/Json/*.cpp
    ../View/*.cpp
    # test only module sources
    *.cpp
    ComponentTest/*.cpp
    ComponentTest/JsonEditorWidgetTest/*.cpp
    DbManagerTest/*.cpp
    FileOperationTest/*.cpp
    JsonTest/*.cpp
    publicTest/*.cpp
    pubTestTool/*.cpp
    RenameTest/*.cpp
    ToolsTest/*.cpp
)

list(LENGTH SOURCES src_cpp_count)
message("\n> ${src_cpp_count} file(s) found in SOURCES, there are as follows:")
foreach(srv_file ${SOURCES})
    message("${srv_file}")
endforeach()

set(SRV_HEADER_PATH
    ../Actions
    ../Model
    ../View
    ../Component
    ../Component/FolderPreview
    ../Component/JsonEditorWidget
    ../Component/RenameWidgets
    ../FileOperation
    ../lib
    ../Model
    ../public
    ../public/Memory
    ../Tools
    ../Tools/Classify
    ../Tools/FileDescriptor
    ../Tools/Json
    ../View
    ./
    pubTestTool
)

file(GLOB HEADERS
    ../Actions/*.h
    ../Model/*.h
    ../View/*.h
    ../Component/*.h
    ../Component/FolderPreview/*.h
    ../Component/JsonEditorWidget/*.h
    ../Component/RenameWidgets/*.h
    ../FileOperation/*.h
    ../lib/*.h
    ../Model/*.h
    ../public/*.h
    ../public/Memory/*.h
    ../Tools/*.h
    ../Tools/Classify/*.h
    ../Tools/FileDescriptor/*.h
    ../Tools/Json/*.h
    ../View/*.h
    *.h
    pubTestTool/*.h
)

list(LENGTH HEADERS header_cpp_count)
message("\n> ${header_cpp_count} file(s) found in HEADERS, there are as follows:")
foreach(header_file ${HEADERS})
    message("${header_file}")
endforeach()

if(WIN32)
    # Windows-specific platform: Use the WIN32 option to hide the console
    add_executable(${PROJECT_NAME} WIN32
        ${SOURCES}
        ${RESOURCES}
    )
else()
    # Linux Platform: Directly generate a GUI application (no terminal displayed by default). No extra flags needed for GUI apps
    add_executable(${PROJECT_NAME}
        ${SOURCES}
        ${RESOURCES}
    )
endif()
target_sources(${PROJECT_NAME} PUBLIC
    ${HEADERS}
    ${QM_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC ${SRV_HEADER_PATH})
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Test
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Widgets
)

target_link_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../lib")
if(WIN32)
    find_library(DWMWAPI_LIBRARY dwmapi)
    find_library(SHLWAPI_LIBRARY shlwapi)

    target_link_libraries(${PROJECT_NAME} PRIVATE dwmapi shlwapi mediaInfo)
    set(ffmpegpath "C:/home/ffmpeg")

    target_include_directories(${PROJECT_NAME} PRIVATE "${ffmpegpath}/include")
    target_link_libraries(${PROJECT_NAME} PRIVATE ws2_32 Qt5::Core
        "${ffmpegpath}/lib/avformat.lib"
        "${ffmpegpath}/lib/avcodec.lib"
        "${ffmpegpath}/lib/avutil.lib"
        "${ffmpegpath}/lib/swscale.lib")
elseif(UNIX AND NOT APPLE)  # Linux
    target_link_libraries(${PROJECT_NAME} PRIVATE avformat avcodec avutil swscale )
endif()

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    $<$<CONFIG:Debug>:QT_MESSAGELOGCONTEXT>
)
message("[${PROJECT_NAME}] Finished...")
