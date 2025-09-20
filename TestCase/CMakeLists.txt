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
    -DTESTCASE_ROOT_PATH="${CMAKE_CURRENT_SOURCE_DIR}"
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
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Test Core Gui Sql Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Test Core Gui Sql Widgets LinguistTools)

file(GLOB SOURCES
    # service module sources
    ${PARENT_DIR}/Actions/*.cpp
    ${PARENT_DIR}/Model/*.cpp
    ${PARENT_DIR}/View/*.cpp
    ${PARENT_DIR}/Component/*.cpp
    ${PARENT_DIR}/Component/FolderPreview/*.cpp
    ${PARENT_DIR}/Component/RenameWidgets/*.cpp
    ${PARENT_DIR}/FileOperation/*.cpp
    ${PARENT_DIR}/Model/*.cpp
    ${PARENT_DIR}/public/*.cpp
    ${PARENT_DIR}/public/Memory/*.cpp
    ${PARENT_DIR}/Tools/*.cpp
    ${PARENT_DIR}/Tools/Classify/*.cpp
    ${PARENT_DIR}/Tools/FileDescriptor/*.cpp
    ${PARENT_DIR}/Tools/Json/*.cpp
    ${PARENT_DIR}/View/*.cpp
    # test only module sources
    *.cpp
    ActionsTest/*.cpp
    ComponentTest/*.cpp
    ComponentTest/FolderPreviewTest/*.cpp
    DbManagerTest/*.cpp
    FileOperationTest/*.cpp
    JsonTest/*.cpp
    ModelTest/*.cpp
    publicTest/*.cpp
    pubTestTool/*.cpp
    RenameTest/*.cpp
    ToolsTest/*.cpp
    ViewTest/*.cpp
)

list(LENGTH SOURCES src_cpp_count)
message("\n> ${src_cpp_count} file(s) found in SOURCES, there are as follows:")
foreach(srv_file ${SOURCES})
    message("${srv_file}")
endforeach()

set(SRV_HEADER_PATH
    ${PARENT_DIR}/Actions
    ${PARENT_DIR}/Model
    ${PARENT_DIR}/View
    ${PARENT_DIR}/Component
    ${PARENT_DIR}/Component/FolderPreview
    ${PARENT_DIR}/Component/RenameWidgets
    ${PARENT_DIR}/FileOperation
    ${PARENT_DIR}/lib
    ${PARENT_DIR}/Model
    ${PARENT_DIR}/public
    ${PARENT_DIR}/public/Memory
    ${PARENT_DIR}/Tools
    ${PARENT_DIR}/Tools/Classify
    ${PARENT_DIR}/Tools/FileDescriptor
    ${PARENT_DIR}/Tools/Json
    ${PARENT_DIR}/View
    ./
    pubTestTool
)

file(GLOB HEADERS
    ${PARENT_DIR}/Actions/*.h
    ${PARENT_DIR}/Model/*.h
    ${PARENT_DIR}/View/*.h
    ${PARENT_DIR}/Component/*.h
    ${PARENT_DIR}/Component/FolderPreview/*.h
    ${PARENT_DIR}/Component/RenameWidgets/*.h
    ${PARENT_DIR}/FileOperation/*.h
    ${PARENT_DIR}/lib/*.h
    ${PARENT_DIR}/Model/*.h
    ${PARENT_DIR}/public/*.h
    ${PARENT_DIR}/public/Memory/*.h
    ${PARENT_DIR}/Tools/*.h
    ${PARENT_DIR}/Tools/Classify/*.h
    ${PARENT_DIR}/Tools/FileDescriptor/*.h
    ${PARENT_DIR}/Tools/Json/*.h
    ${PARENT_DIR}/View/*.h
    pubTestTool/*.h
)

set(READ_TR_FROM_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/publicTest/PublicToolTest.cpp
)

list(LENGTH HEADERS header_cpp_count)
message("\n> ${header_cpp_count} file(s) found in HEADERS, there are as follows:")
foreach(header_file ${HEADERS})
    message("${header_file}")
endforeach()

set(RESOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/test/translations.qrc
)

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


set(QM_FILE_IN_QRC
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_zh_CN.qm
)
set(TS_FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/test/test_zh_CN.ts
)
qt_add_translation(QM_FILES ${TS_FILES})

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

if(UNIX AND NOT APPLE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fprofile-arcs -ftest-coverage")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -lgcov")
add_compile_options(-fprofile-update=atomic)
endif()

set(LUPDATE_EXECUTABLE "/home/ariel/Qt/5.15.2/gcc_64/bin/lupdate")
set(LRELEASE_EXECUTABLE "/home/ariel/Qt/5.15.2/gcc_64/bin/lrelease")
message(STATUS "Run following command in terminal if translation need update")
message("${LUPDATE_EXECUTABLE} ${READ_TR_FROM_SOURCE} -ts ${TS_FILES}")
message("${LRELEASE_EXECUTABLE} ${TS_FILES} -qm ${QM_FILE_IN_QRC}")
message(STATUS "The End")
