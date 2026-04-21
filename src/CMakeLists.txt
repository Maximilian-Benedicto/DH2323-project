cmake_minimum_required(VERSION 3.11) # FetchContent is new in version 3.11.
project(DH2323Project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall")

add_executable(DH2323Project 
    main.cpp 
    Window.cpp 
    LambertianShader.cpp 
    DipoleShader.cpp 
    WireframeShader.cpp
    CornellBox.cpp
    PlyModel.cpp
    ObjModel.cpp
    Material.cpp
    BVH.cpp
)

add_executable(SlabIntersectionTest
    SlabIntersectionTest.cpp
    LambertianShader.cpp
)

include(FetchContent)


FetchContent_Declare(
    glm
    GIT_REPOSITORY  https://github.com/g-truc/glm.git
    GIT_TAG     0af55ccecd98d4e5a8d1fad7de25ba429d60e863 #refs/tags/1.0.1
)

FetchContent_MakeAvailable(glm)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY  https://github.com/libsdl-org/SDL.git
    GIT_TAG     main
)

FetchContent_MakeAvailable(SDL3)

FetchContent_Declare(
    happly
    GIT_REPOSITORY  https://github.com/nmwsharp/happly.git
    GIT_TAG     master
)

FetchContent_MakeAvailable(happly)

FetchContent_Declare(
    tinyobjloader
    GIT_REPOSITORY  https://github.com/tinyobjloader/tinyobjloader.git
    GIT_TAG     release
)

FetchContent_MakeAvailable(tinyobjloader)

FetchContent_Declare(
    stb
    GIT_REPOSITORY  https://github.com/nothings/stb.git
    GIT_TAG     master
)

FetchContent_MakeAvailable(stb)


target_include_directories(DH2323Project PRIVATE 
    ${happly_SOURCE_DIR}
    ${tinyobjloader_SOURCE_DIR}
    ${stb_SOURCE_DIR}
)

target_link_libraries(DH2323Project PRIVATE glm::glm SDL3::SDL3)
target_link_libraries(SlabIntersectionTest PRIVATE glm::glm SDL3::SDL3)

if(WIN32)
    # Ensure SDL3.dll and other runtime DLL dependencies are placed beside the executable.
    add_custom_command(TARGET DH2323Project POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:DH2323Project>
            $<TARGET_FILE_DIR:DH2323Project>
        COMMAND_EXPAND_LISTS
    )
endif()