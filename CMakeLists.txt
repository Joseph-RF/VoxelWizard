cmake_minimum_required(VERSION "3.31.0")

set(CMAKE_CXX_STANDARD 17)

# Set this true when "shipping". Will change where the location of resources directory is
# See below for more details.
set(PRODUCTION_BUILD OFF CACHE BOOL "Make this a production build" FORCE)

# Set the C++ runtime to link statically. Code for the runtime is copied into the executable
# It is self-contained and easily portable as users don't have to add separate .dll files
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")


#may give problems on linux
if(PRODUCTION_BUILD)
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE) #link time optimization
else()
	set(CMAKE_INTERPROCEDURAL_OPTIMIZATION FALSE)
endif()



if(MSVC)
	add_compile_options(/arch:AVX2) #make sure SIMD optimizations take place
endif()

project(main)


set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)


add_subdirectory(third_party/glfw-3.4)			# GLFW for creating window
add_subdirectory(third_party/glad)				# Grabs OpenGL
add_subdirectory(third_party/stb_image)			# Loading images
add_subdirectory(third_party/glm)				# Matrix and vector math
add_subdirectory(third_party/imgui)				# GUI


# Define MY_SOURCES to be a list of all the cpp files in the src directory. Reload CMake every time
# a new file is added (Ctrl + S on VS).
file(GLOB_RECURSE MY_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")


add_executable("${CMAKE_PROJECT_NAME}")

set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY CXX_STANDARD 17)


target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC GLFW_INCLUDE_NONE=1)

# Adds a macro for the resources directory to be used in the project. Necessary when using CMake as
# using relative paths CMake will assume root location is where the executable is which changes
# depending on whether Debug or Release is being used. This removes that.
if(PRODUCTION_BUILD)
	# Setup the RESOURCES_PATH macro to be in the root folder of your exe. For release/production
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="./resources/")
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC SHADERS_PATH="./resources/shaders/")
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC TEXTURES_PATH="./resources/textures/")

	# remove the option to debug asserts.
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=1)
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC DEVELOPMENT_BUILD=0)

else()
	# This is useful to get an RESOURCES_PATH in your IDE during development
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC RESOURCES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/")
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC SHADERS_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/shaders/")
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC TEXTURES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/resources/textures/")
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC PRODUCTION_BUILD=0)
	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC DEVELOPMENT_BUILD=1)

endif()

target_sources("${CMAKE_PROJECT_NAME}" PRIVATE ${MY_SOURCES} )


if(MSVC) # If using the VS compiler...

	target_compile_definitions("${CMAKE_PROJECT_NAME}" PUBLIC _CRT_SECURE_NO_WARNINGS)

	# Commented out expression below removes the console

	#set_target_properties("${CMAKE_PROJECT_NAME}" PROPERTIES LINK_FLAGS "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")

	#set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreadedDebug<$<CONFIG:Debug>:Debug>")
	#set_property(TARGET "${CMAKE_PROJECT_NAME}" PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Release>:Release>")

endif()

#force remove unicode
if (WIN32)
	target_compile_options("${CMAKE_PROJECT_NAME}" PRIVATE -UUNICODE -U_UNICODE)
endif()


target_include_directories("${CMAKE_PROJECT_NAME}" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include/")

# Link third party libraries to the executable
target_link_libraries("${CMAKE_PROJECT_NAME}" PRIVATE glm glfw glad stb_image imgui)