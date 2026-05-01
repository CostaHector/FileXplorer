cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME FileXplorer)
project(${PROJECT_NAME} LANGUAGES CXX)

string(LENGTH "${CMAKE_CURRENT_SOURCE_DIR}/" PARENT_PATH_LEN)
add_definitions(
    -DPROJECT_NAME="${PROJECT_NAME}"
    -DPARENT_PATH_LEN=${PARENT_PATH_LEN}
)

SET(PASSVAULT_ENABLED ON)
if (PASSVAULT_ENABLED)
    add_definitions(
        -DPASSVAULT_ENABLED
    )
endif()

# on windows platform. MediaInfoDLL.h defined a typdef needed
message("[${PROJECT_NAME}] Start...")

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC_MOC_OPTIONS "-DLOG_MACRO_IS_SAFE")

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Search for Qt libraries in the order of Qt6, Qt5
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Gui Sql Widgets Charts LinguistTools Multimedia MultimediaWidgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Sql Widgets Charts LinguistTools Multimedia MultimediaWidgets)


file(GLOB SOURCES
    Actions/*.cpp
    Events/*.cpp
    Model/*.cpp
    View/*.cpp
    Component/*.cpp
    Component/Configs/*.cpp
    Component/DupImages/*.cpp
    Component/DupVideos/*.cpp
    Component/FolderPreview/*.cpp
    Component/GeneralDataEditor/*.cpp
    Component/HeaderView/*.cpp
    Component/LineEdit/*.cpp
    Component/Navigation/*.cpp
    Component/PassVault/*.cpp
    Component/RenameWidgets/*.cpp
    Component/Stylesheet/*.cpp
    Component/VideoPlayer/*.cpp
    FileOperation/*.cpp
    Model/*.cpp
    public/*.cpp
    public/Memory/*.cpp
    Tools/*.cpp
    Tools/Classify/*.cpp
    Tools/FileDescriptor/*.cpp
    Tools/Json/*.cpp
    View/*.cpp
    main.cpp
)
list(LENGTH SOURCES src_cpp_count)
message("\n> ${src_cpp_count} file(s) in SOURCES, there are:")
foreach(srv_file ${SOURCES})
    message("${srv_file}")
endforeach()


set(SRV_HEADER_PATH
    Actions
    Events
    Model
    View
    Component
    Component/Configs
    Component/FolderPreview
    Component/GeneralDataEditor
    Component/DupImages
    Component/DupVideos
    Component/HeaderView
    Component/LineEdit
    Component/Navigation
    Component/PassVault
    Component/RenameWidgets
    Component/Stylesheet
    Component/VideoPlayer
    FileOperation
    Model
    public
    public/Memory
    Tools
    Tools/Classify
    Tools/FileDescriptor
    Tools/Json
    third_party/ffmpeg/include
    third_party/mediaInfo/include
    third_party/openssl/include
    View
)

file(GLOB HEADERS
    Actions/*.h
    Events/*.h
    Model/*.h
    View/*.h
    Component/*.h
    Component/Configs/*.h
    Component/DupImages/*.h
    Component/DupVideos/*.h
    Component/FolderPreview/*.h
    Component/GeneralDataEditor/*.h
    Component/HeaderView/*.h
    Component/LineEdit/*.h
    Component/Navigation/*.h
    Component/PassVault/*.h
    Component/RenameWidgets/*.h
    Component/Stylesheet/*.h
    Component/VideoPlayer/*.h
    FileOperation/*.h
    Model/*.h
    public/*.h
    public/Memory/*.h
    Tools/*.h
    Tools/Classify/*.h
    Tools/FileDescriptor/*.h
    Tools/Json/*.h
    third_party/ffmpeg/include/libavcodec/*.h
    third_party/ffmpeg/include/libavdevice/*.h
    third_party/ffmpeg/include/libavfilter/*.h
    third_party/ffmpeg/include/libavformat/*.h
    third_party/ffmpeg/include/libavutil/*.h
    third_party/ffmpeg/include/libpostproc/*.h
    third_party/ffmpeg/include/libswresample/*.h
    third_party/ffmpeg/include/libswscale/*.h
    third_party/mediaInfo/include/*.h
    third_party/openssl/include/openssl/*.h
    View/*.h
)

list(LENGTH HEADERS header_cpp_count)
message("\n> ${header_cpp_count} file(s) in HEADERS, there are:")
foreach(header_file ${HEADERS})
    message("${header_file}")
endforeach()

if (PASSVAULT_ENABLED)
    list(APPEND SOURCES Tools/SimpleAES.cpp)
    list(APPEND HEADERS Tools/SimpleAES.h)
endif()

set(RESOURCES
    "resources/images.qrc"
)

file(GLOB TS_FILES
    resources/Translate/*.ts
)

list(LENGTH TS_FILES ts_files_count)
message("\n> ${ts_files_count} translation file(s), there are:")
foreach(ts_file ${TS_FILES})
    message("${ts_file}")
endforeach()

qt_add_translation(QM_FILES ${TS_FILES})

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
    ${CMAKE_CURRENT_SOURCE_DIR}/readme.md
    ${CMAKE_CURRENT_SOURCE_DIR}/gencov.sh
)

target_include_directories(${PROJECT_NAME} PUBLIC ${SRV_HEADER_PATH})
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Charts
    Qt${QT_VERSION_MAJOR}::Multimedia
    Qt${QT_VERSION_MAJOR}::MultimediaWidgets
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE Psapi dwmapi)
    target_link_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/mediaInfo/bin")
    target_link_libraries(${PROJECT_NAME} PRIVATE MediaInfo)
    target_link_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/ffmpeg/lib")
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE avformat avcodec avutil swscale)

if(PASSVAULT_ENABLED)
    if(WIN32)    
        target_link_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/openssl/lib")
        target_link_libraries(${PROJECT_NAME} PRIVATE libssl libcrypto)
    elseif(UNIX AND NOT APPLE)  # Linux
        find_package(OpenSSL REQUIRED)
        target_link_libraries(${PROJECT_NAME} PRIVATE OpenSSL::SSL OpenSSL::Crypto)

        find_package(PkgConfig REQUIRED)
        pkg_check_modules(LIBSECRET REQUIRED libsecret-1)
        include_directories(${LIBSECRET_INCLUDE_DIRS})
        target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBSECRET_LIBRARIES})
    endif()
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
    DESTINATION share/${PROJECT_NAME}
)
