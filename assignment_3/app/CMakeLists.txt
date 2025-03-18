# Build the app, using the HAL

include_directories(include)
file(GLOB MY_SOURCES "src/*.c")
add_executable(beatbox ${MY_SOURCES})

find_library(GPIOD_LIBRARY gpiod)       # UNSURE IF NEEDED
# Make use of the HAL library
target_link_libraries(beatbox LINK_PRIVATE hal)
target_link_libraries(beatbox LINK_PRIVATE lcd)
target_link_libraries(beatbox LINK_PRIVATE lgpio)
target_link_libraries(beatbox LINK_PRIVATE gpiod) # May need to change

# Copy executable to final location (change `hello_world` to project name as needed)
add_custom_command(TARGET beatbox POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy 
     "$<TARGET_FILE:beatbox>"
     "~/cmpt433/public/myApps/beatbox" 
  COMMENT "Copying ARM executable to public NFS directory")
  

