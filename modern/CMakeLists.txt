cmake_minimum_required(VERSION 3.30)

project(RemoteSupport LANGUAGES CXX)

if(NOT WIN32)
    message(FATAL_ERROR "RemoteSupport currently targets Windows only.")
endif()

add_executable(RemoteSupport WIN32
    RemoteSupport/main.cpp
)

target_compile_features(RemoteSupport PRIVATE cxx_std_20)
target_compile_definitions(RemoteSupport PRIVATE
    UNICODE
    _UNICODE
    WIN32_LEAN_AND_MEAN
    NOMINMAX
)

target_link_libraries(RemoteSupport PRIVATE
    bcrypt
    ole32
    shell32
)

if(MSVC)
    target_compile_options(RemoteSupport PRIVATE
        /W4
        /permissive-
        /sdl
        /GS
        /guard:cf
        /utf-8
    )
    target_link_options(RemoteSupport PRIVATE
        /DYNAMICBASE
        /NXCOMPAT
        /guard:cf
    )

    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        target_link_options(RemoteSupport PRIVATE /CETCOMPAT)
    endif()
endif()
