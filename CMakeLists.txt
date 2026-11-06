cmake_minimum_required(VERSION 3.16)
project(psdoom C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3 REQUIRED sdl3)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/xdoomsrc/xdoom
    ${SDL3_INCLUDE_DIRS}
)

add_definitions(
    -DNORMALUNIX
    -DLINUX
    -DBUGFIXES
)

set(PSDOOM_SOURCES
    xdoomsrc/xdoom/am_map.c
    xdoomsrc/xdoom/cd_stub.c
    xdoomsrc/xdoom/d_items.c
    xdoomsrc/xdoom/d_main.c
    xdoomsrc/xdoom/d_net.c
    xdoomsrc/xdoom/doomdef.c
    xdoomsrc/xdoom/doomstat.c
    xdoomsrc/xdoom/dstrings.c
    xdoomsrc/xdoom/endtxt.c
    xdoomsrc/xdoom/f_finale.c
    xdoomsrc/xdoom/f_wipe.c
    xdoomsrc/xdoom/g_game.c
    xdoomsrc/xdoom/hu_lib.c
    xdoomsrc/xdoom/hu_stuff.c
    xdoomsrc/xdoom/i_main.c
    xdoomsrc/xdoom/i_net.c
    xdoomsrc/xdoom/i_system.c
    xdoomsrc/xdoom/i_video_sdl3.c
    xdoomsrc/xdoom/i_sound_sdl3.c
    xdoomsrc/xdoom/info.c
    xdoomsrc/xdoom/m_argv.c
    xdoomsrc/xdoom/m_bbox.c
    xdoomsrc/xdoom/m_cheat.c
    xdoomsrc/xdoom/m_comdev.c
    xdoomsrc/xdoom/m_fixed.c
    xdoomsrc/xdoom/m_menu.c
    xdoomsrc/xdoom/m_misc.c
    xdoomsrc/xdoom/m_random.c
    xdoomsrc/xdoom/m_swap.c
    xdoomsrc/xdoom/p_acs.c
    xdoomsrc/xdoom/p_ceilng.c
    xdoomsrc/xdoom/p_doors.c
    xdoomsrc/xdoom/p_doorsplus.c
    xdoomsrc/xdoom/p_enemy.c
    xdoomsrc/xdoom/p_floor.c
    xdoomsrc/xdoom/p_inter.c
    xdoomsrc/xdoom/p_lights.c
    xdoomsrc/xdoom/p_map.c
    xdoomsrc/xdoom/p_maputl.c
    xdoomsrc/xdoom/p_mobj.c
    xdoomsrc/xdoom/p_plats.c
    xdoomsrc/xdoom/p_pspr.c
    xdoomsrc/xdoom/pr_process.c
    xdoomsrc/xdoom/p_saveg.c
    xdoomsrc/xdoom/p_setup.c
    xdoomsrc/xdoom/p_sight.c
    xdoomsrc/xdoom/p_spec.c
    xdoomsrc/xdoom/p_specplus.c
    xdoomsrc/xdoom/p_switch.c
    xdoomsrc/xdoom/p_telept.c
    xdoomsrc/xdoom/p_tick.c
    xdoomsrc/xdoom/p_user.c
    xdoomsrc/xdoom/r_bsp.c
    xdoomsrc/xdoom/r_data.c
    xdoomsrc/xdoom/r_draw.c
    xdoomsrc/xdoom/r_main.c
    xdoomsrc/xdoom/r_plane.c
    xdoomsrc/xdoom/r_segs.c
    xdoomsrc/xdoom/r_sky.c
    xdoomsrc/xdoom/r_things.c
    xdoomsrc/xdoom/s_sound.c
    xdoomsrc/xdoom/searchp.c
    xdoomsrc/xdoom/sounds.c
    xdoomsrc/xdoom/st_lib.c
    xdoomsrc/xdoom/st_stuff.c
    xdoomsrc/xdoom/tables.c
    xdoomsrc/xdoom/v_video.c
    xdoomsrc/xdoom/w_wad.c
    xdoomsrc/xdoom/wi_stuff.c
    xdoomsrc/xdoom/z_zone.c
)

add_executable(psdoom ${PSDOOM_SOURCES})

target_compile_options(psdoom PRIVATE
    -Wall
)

target_link_libraries(psdoom PRIVATE ${SDL3_LIBRARIES} m)

install(TARGETS psdoom DESTINATION bin)
install(FILES psdoom-data/psdoom1.wad psdoom-data/psdoom2.wad xdoomsrc/data/xdoom.wad DESTINATION share/games/psdoom)
