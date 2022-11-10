list(APPEND CMAKE_MODULE_PATH "${KartDashboard_SOURCE_DIR}/cmake")

find_package(SDL2 REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_gfx REQUIRED)

include_directories(${KartDashboard_SOURCE_DIR}/include ${SDL2_IMAGE_INCLUDE_DIR} ${SDL2_GFX_INCLUDE_DIR})

add_executable(dashboard dashboard.cc rasterizer.cc rasterizer.h)
target_link_libraries(dashboard SDL2::SDL2 ${SDL2_GFX_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})
