cmake_minimum_required(VERSION 3.8)

project(portal-cpp-opengl)

set(CMAKE_CXX_STANDARD 17)

file(GLOB SOURCE_FILES *.cpp *.h)

find_package(Bullet REQUIRED)
find_package(FreeGLUT REQUIRED)

add_executable(portal-cpp-opengl ${SOURCE_FILES})
target_link_libraries(portal-cpp-opengl
PUBLIC
    ${BULLET_LIBRARIES}
    FreeGLUT::freeglut
    GLEW
)
target_include_directories(portal-cpp-opengl
PUBLIC
    ${BULLET_INCLUDE_DIRS}

)

file(COPY resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})