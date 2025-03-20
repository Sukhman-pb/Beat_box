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

# Copy executable to final location (change `beatbox` to project name as needed)
add_custom_command(TARGET beatbox POST_BUILD 
  COMMAND "${CMAKE_COMMAND}" -E copy 
     "$<TARGET_FILE:beatbox>"
     "~/cmpt433/public/myApps/beatbox" 
  COMMENT "Copying ARM executable to public NFS directory")
  


  # Copy `beatbox-wave-files` directory to the destination
add_custom_command(TARGET beatbox POST_BUILD
COMMAND "${CMAKE_COMMAND}" -E copy_directory
   "${CMAKE_SOURCE_DIR}/beatbox-wave-files"
   "~/cmpt433/public/myApps/beatbox-wave-files"
COMMENT "Copying beatbox-wave-files to public NFS directory")

# Copy `beatbox-server-copy` directory to the destination
add_custom_command(TARGET beatbox POST_BUILD
COMMAND "${CMAKE_COMMAND}" -E copy_directory
   "${CMAKE_SOURCE_DIR}/beatbox-server-copy"
   "~/cmpt433/public/myApps/beatbox-server-copy"
COMMENT "Copying beatbox-server-copy to public NFS directory")
