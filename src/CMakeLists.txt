project("main")

#Put the sources into a variable
set(SOURCE_MAIN "main.cpp" "camera.h" "simple_shader.h" "tess_shader.h" "terrain_generation.h" "object.h")


add_compile_definitions(PATH_TO_SHADER="${CMAKE_CURRENT_SOURCE_DIR}/shaders")
add_compile_definitions(PATH_TO_OBJECTS="${CMAKE_CURRENT_SOURCE_DIR}/assets/objects")
add_compile_definitions(PATH_TO_TEXTURE="${CMAKE_CURRENT_SOURCE_DIR}/assets/textures")
#To use the content of a variable you need to use ${NAME_OF_YOUR_VARIABLE}
#Specify that you want to generate an executable with a certain name using a set of sources
add_executable(${PROJECT_NAME}_exe ${SOURCE_MAIN})
#Specify which libraries you want to use with your executable
target_link_libraries(${PROJECT_NAME}_exe PUBLIC OpenGL::GL glfw glad)


