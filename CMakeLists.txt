cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME FileXplorer)
project(${PROJECT_NAME} LANGUAGES CXX)
add_definitions(-DPROJECT_NAME="${PROJECT_NAME}")
add_definitions(-DUNICODE -D_UNICODE)
# on windows platform. MediaInfoDLL.h defined a typdef needed

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Search for Qt libraries in the order of Qt6, Qt5
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Sql Widgets LinguistTools)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Sql Widgets LinguistTools)


file(GLOB SOURCES
    Actions/*.cpp
    Model/*.cpp
    View/*.cpp
    Component/*.cpp
    Component/FolderPreview/*.cpp
    Component/JsonEditorWidget/*.cpp
    Component/RenameWidgets/*.cpp
    FileOperation/*.cpp
    Model/*.cpp
    public/*.cpp
    public/Memory/*.cpp
    Tools/*.cpp
    Tools/Classify/*.cpp
    Tools/FileDescriptor/*.cpp
    Tools/Json/*.cpp
    View/*.cpp
    *.cpp
)
list(LENGTH SOURCES src_cpp_count)
message("\n> ${src_cpp_count} file(s) in SOURCES, there are:")
foreach(srv_file ${SOURCES})
    message("${srv_file}")
endforeach()


set(SRV_HEADER_PATH
    Actions
    Model
    View
    Component
    Component/FolderPreview
    Component/JsonEditorWidget
    Component/RenameWidgets
    FileOperation
    Model
    public
    public/Memory
    Tools
    Tools/Classify
    Tools/FileDescriptor
    Tools/Json
    View
    ./
)

file(GLOB HEADERS
    Actions/*.h
    Model/*.h
    View/*.h
    Component/*.h
    Component/FolderPreview/*.h
    Component/JsonEditorWidget/*.h
    Component/RenameWidgets/*.h
    FileOperation/*.h
    Model/*.h
    public/*.h
    public/Memory/*.h
    Tools/*.h
    Tools/Classify/*.h
    Tools/FileDescriptor/*.h
    Tools/Json/*.h
    View/*.h
    *.h
)

list(LENGTH HEADERS header_cpp_count)
message("\n> ${header_cpp_count} file(s) in HEADERS, there are:")
foreach(header_file ${HEADERS})
    message("${header_file}")
endforeach()


set(RESOURCES
    "bin/themes/res.qrc"
    "bin/themes/styles/dark/darkstyle.qrc"
    "bin/themes/styles/default/defaultstyle.qrc"
)

file(GLOB TS_FILES
    Translate/*.ts
)

list(LENGTH TS_FILES ts_files_count)
message("\n> ${ts_files_count} translation file(s), there are:")
foreach(ts_file ${TS_FILES})
    message("${ts_file}")
endforeach()

qt_add_translation(QM_FILES
    ${TS_FILES}
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
target_sources(${PROJECT_NAME} PUBLIC
    ${HEADERS}
    ${QM_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC ${SRV_HEADER_PATH})
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Widgets
)

target_link_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/lib")
if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE dwmapi mediaInfo)
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

# 安装规则
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# 安装额外文件
install(FILES
    "Translate/FileXplorer_zh_CN.ts"
    "bin/TERMINAL_OPEN_BATCH_FILE_PATH.bat"
    "bin/TERMINAL_OPEN_BATCH_FILE_PATH.sh"
    "readme.md"
    DESTINATION share/${PROJECT_NAME}
)
