cmake_minimum_required(VERSION 2.6)
project(nnn LANGUAGES C)

# find curses
find_path( CURSES_INCLUDE_PATH NAMES curses.h )
find_library( CURSES_LIBRARY NAMES ncursesw )
include_directories( ${CURSES_INCLUDE_PATHS} )

# realine.
# Now find the realine library
find_path( READLINE_INCLUDE_PATH NAMES readline.h readline/readline.h )
find_library( READLINE_LIBRARY readline )
include_directories( ${READLINE_INCLUDE_PATH} )

message( STATUS "READLINE include path ${READLINE_INCLUDE_PATH}" )
message( STATUS "READLINE library ${READLINE_LIBRARY}" )
message( STATUS "CURSES include path ${CURSES_INCLUDE_PATH}" )
message( STATUS "CURSES library ${CURSES_LIBRARY}" )

add_executable(nnn nnn.c)
target_link_libraries(nnn ${CURSES_LIBRARY} ${READLINE_LIBRARY} )

# install 
install( TARGETS nnn DESTINATION bin )
install( PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/nlay DESTINATION bin)
install( FILES ${CMAKE_CURRENT_SOURCE_DIR}/nnn.1 DESTINATION share/man/man1/)
