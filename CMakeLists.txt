cmake_minimum_required(VERSION 3.11)

project(mpprinter)

#set(LINUX 1)

set(SRCDIR ${CMAKE_CURRENT_LIST_DIR}/src)
set(SRCRESDIR ${SRCDIR}/res)

add_executable(${PROJECT_NAME}
	${SRCDIR}/errorinfo.cpp
	${SRCDIR}/debugreport.cpp
	${SRCDIR}/mp_model.cpp
	${SRCDIR}/mp1020.cpp
	${SRCDIR}/mp80.cpp
	${SRCDIR}/buffer.cpp
	${SRCDIR}/config.cpp
	${SRCDIR}/coordinate.cpp
	${SRCDIR}/dpbuffer.cpp
	${SRCDIR}/ctrl_setting.cpp
	${SRCDIR}/marginbox.cpp
	${SRCDIR}/densitybox.cpp
	${SRCDIR}/server.cpp
	${SRCDIR}/server_setting.cpp
	${SRCDIR}/mp_printer.cpp
	${SRCDIR}/main.cpp
)

if(APPLE)
  #
  # For MacOS
  #
  set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR})

  set(wxVer 3.2)
  set(wxWidgetsDir $ENV{HOME}/Devel/wxWidgets-3.2.4/)
  set(wxWidgetsBuildDir ${wxWidgetsDir}/build_release_static_unicode/)
  set(wxWidgetsConfig ${wxWidgetsBuildDir}wx-config)
  set(wxWidgetsFlags )
  set(wxWidgetsDefines __WXMAC__ __WXOSX__ __WXOSX_COCOA__ _FILE_OFFSET_BITS=64 wxDEBUG_LEVEL=0)
  set(wxWidgetsIncludeDir ${wxWidgetsBuildDir}lib/wx/include/osx_cocoa-unicode-static-${wxVer} ${wxWidgetsDir}include)
  set(wxWidgetsLibDir ${wxWidgetsBuildDir}lib)
  set(wxWidgetsLibs
    ${wxWidgetsLibDir}/libwx_osx_cocoau_xrc-${wxVer}.a
    ${wxWidgetsLibDir}/libwx_osx_cocoau_stc-${wxVer}.a
    ${wxWidgetsLibDir}/libwx_osx_cocoau_richtext-${wxVer}.a
    ${wxWidgetsLibDir}/libwx_osx_cocoau_ribbon-${wxVer}.a
    ${wxWidgetsLibDir}/libwx_osx_cocoau_propgrid-${wxVer}.a
    ${wxWidgetsLibDir}/libwx_osx_cocoau_aui-${wxVer}.a
    ${wxWidgetsLibDir}/libwx_osx_cocoau_gl-${wxVer}.a
    ${wxWidgetsLibDir}/libwx_osx_cocoau_qa-${wxVer}.a
    ${wxWidgetsLibDir}/libwx_baseu_net-${wxVer}.a
    ${wxWidgetsLibDir}/libwx_osx_cocoau_html-${wxVer}.a
    ${wxWidgetsLibDir}/libwx_osx_cocoau_core-${wxVer}.a
    ${wxWidgetsLibDir}/libwx_baseu_xml-${wxVer}.a
    ${wxWidgetsLibDir}/libwx_baseu-${wxVer}.a
    -lwxjpeg-${wxVer}
    -lwxpng-${wxVer}
    -lwxregexu-${wxVer}
    -lwxscintilla-${wxVer}
  )

  set(AppleLibs
    -Wl,-framework,IOKit
    -Wl,-framework,Carbon
    -Wl,-framework,Cocoa
    -Wl,-framework,QuartzCore
    -Wl,-framework,AudioToolbox
    -Wl,-framework,System
    -Wl,-framework,OpenGL
    -Wl,-framework,Security
    -Wl,-framework,AGL
    -lexpat
    -lz
    -lpthread
    -liconv
  )

  target_compile_definitions(${PROJECT_NAME} PUBLIC ${wxWidgetsDefines})
  target_compile_options(${PROJECT_NAME} PUBLIC ${wxWidgetsFlags})
  target_include_directories(${PROJECT_NAME} PUBLIC ${SRCDIR} ${wxWidgetsIncludeDir})
  target_link_directories(${PROJECT_NAME} PUBLIC ${wxWidgetsLibDir})
  target_link_libraries(${PROJECT_NAME} PUBLIC ${wxWidgetsLibs} ${AppleLibs})
  target_link_options(${PROJECT_NAME} PUBLIC )

  install(TARGETS ${PROJECT_NAME} DESTINATION Release/${PROJECT_NAME}.app/Contents/MacOS BUNDLE)
  install(FILES font/prn_font_l3.dat DESTINATION Release/${PROJECT_NAME}.app/Contents/Resources)
  install(FILES font/prn_font_m8.dat DESTINATION Release/${PROJECT_NAME}.app/Contents/Resources)
  install(DIRECTORY lang DESTINATION Release/${PROJECT_NAME}.app/Contents/Resources)
  install(FILES src/res/Info.plist DESTINATION Release/${PROJECT_NAME}.app/Contents/)
  install(FILES src/res/${PROJECT_NAME}.icns DESTINATION Release/${PROJECT_NAME}.app/Contents/Resources/)

elseif(LINUX)
  #
  # For LINUX
  #
  set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR})

  find_package(wxWidgets REQUIRED COMPONENTS core base net)
  include(${wxWidgets_USE_FILE})
  target_link_libraries(${PROJECT_NAME} PUBLIC ${wxWidgets_LIBRARIES})

  install(TARGETS ${PROJECT_NAME} DESTINATION Release)
  install(FILES font/prn_font_l3.dat DESTINATION Release)
  install(FILES font/prn_font_m8.dat DESTINATION Release)
  install(DIRECTORY lang DESTINATION Release)

elseif(MSVC_ALT)
  #
  # For Visual C++
  #
  set(wxWidgets_DIR /Devel/vc/wxWidgets-3.2.4)
  set(wxWidgets_ROOT_DIR ${wxWidgets_DIR})
  set(wxWidgets_LIB_DIR ${wxWidgets_ROOT_DIR}/lib/vc14/vc_x64_lib)
  set(wxWidgets_CONFIGURATION mswu)
#  set(wxWidgets_EXCLUDE_COMMON_LIBRARIES )
  find_package(wxWidgets REQUIRED COMPONENTS base core richtext xml)
  include(${wxWidgets_USE_FILE})
  target_link_libraries(${PROJECT_NAME} PUBLIC ${wxWidgets_LIBRARIES})
  target_link_options(${PROJECT_NAME} PUBLIC /SUBSYSTEM:WINDOWS)

  # copy resource files
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/lang $<TARGET_FILE_DIR:${PROJECT_NAME}>/lang)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/font/prn_font_l3.dat $<TARGET_FILE_DIR:${PROJECT_NAME}>)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/font/prn_font_m8.dat $<TARGET_FILE_DIR:${PROJECT_NAME}>)

elseif(MSVC)
  #
  # For Visual C++ (manual settings)
  #
  set(DevelopDir \\Devel\\vc\\)
  set(wxWidgetsDir ${DevelopDir}\\wxWidgets-3.2.4\\)
  set(wxWidgetsIncludeDir ${wxWidgetsDir}include\\msvc\\;${wxWidgetsDir}include\\)
  set(wxWidgetsStaticDefs )
  set(wxWidgetsDynamicDefs WXUSINGDLL)
  set(wxWidgetsStaticLibDirWin32 ${wxWidgetsDir}lib\\vc14\\vc_lib\\)
  set(wxWidgetsStaticLibDirX64 ${wxWidgetsDir}lib\\vc14\\vc_x64_lib\\)
  set(wxWidgetsDynamicLibDirWin32 ${wxWidgetsDir}lib\\vc14\\vc_dll\\)
  set(wxWidgetsDynamicLibDirX64 ${wxWidgetsDir}lib\\vc14\\vc_x64_dll\\)
  set(wxWidgetsLibsDebug wxbase32ud.lib wxmsw32ud_core.lib)
  set(wxWidgetsLibsRelease wxbase32u.lib wxmsw32u_core.lib)

  target_compile_definitions(${PROJECT_NAME} PUBLIC UNICODE _UNICODE _DEBUG _DEBUG_LOG)
  target_include_directories(${PROJECT_NAME} PUBLIC ${SRCDIR} ${wxWidgetsIncludeDir})

  target_link_directories(${PROJECT_NAME} PUBLIC ${wxWidgetsStaticLibDirX64})
  target_link_libraries(${PROJECT_NAME} PUBLIC ${wxWidgetsLibsDebug})
  target_link_options(${PROJECT_NAME} PUBLIC /SUBSYSTEM:WINDOWS)

  # copy resource files
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_LIST_DIR}/lang $<TARGET_FILE_DIR:${PROJECT_NAME}>/lang)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/font/prn_font_l3.dat $<TARGET_FILE_DIR:${PROJECT_NAME}>)
  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_LIST_DIR}/font/prn_font_m8.dat $<TARGET_FILE_DIR:${PROJECT_NAME}>)

else()
  #
  # For MSYS2 + MINGW64
  #
  set(CMAKE_C_COMPILER gcc)
  set(CMAKE_CXX_COMPILER g++)

  set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_LIST_DIR})

  set(wxVer 3.2)
  set(wxWidgetsDir /D/Devel/sources/wxWidgets-3.2.4/)
  set(wxWidgetsBuildDir ${wxWidgetsDir}build_release_static_unicode_mingw64/)
  set(wxWidgetsConfig ${wxWidgetsBuildDir}wx-config)
  set(wxWidgetsDefines __WXMSW__ _FILE_OFFSET_BITS=64 wxDEBUG_LEVEL=0)
  set(wxWidgetsIncludeDir ${wxWidgetsBuildDir}lib/wx/include/msw-unicode-static-${wxVer} ${wxWidgetsDir}include)
  set(wxWidgetsLibDir ${wxWidgetsBuildDir}lib/)
  set(wxWidgetsLibs
    ${wxWidgetsLibDir}libwx_mswu_xrc-${wxVer}.a
    ${wxWidgetsLibDir}libwx_mswu_webview-${wxVer}.a
    ${wxWidgetsLibDir}libwx_mswu_stc-${wxVer}.a
    ${wxWidgetsLibDir}libwx_mswu_richtext-${wxVer}.a
    ${wxWidgetsLibDir}libwx_mswu_ribbon-${wxVer}.a
    ${wxWidgetsLibDir}libwx_mswu_propgrid-${wxVer}.a
    ${wxWidgetsLibDir}libwx_mswu_aui-${wxVer}.a
    ${wxWidgetsLibDir}libwx_mswu_gl-${wxVer}.a
    ${wxWidgetsLibDir}libwx_mswu_qa-${wxVer}.a
    ${wxWidgetsLibDir}libwx_baseu_net-${wxVer}.a
    ${wxWidgetsLibDir}libwx_mswu_html-${wxVer}.a
    ${wxWidgetsLibDir}libwx_mswu_core-${wxVer}.a
    ${wxWidgetsLibDir}libwx_baseu_xml-${wxVer}.a
    ${wxWidgetsLibDir}libwx_baseu-${wxVer}.a
    ${wxWidgetsLibDir}libwx_mswu_adv-${wxVer}.a
    ${wxWidgetsLibDir}libwx_mswu_media-${wxVer}.a
#   ${wxWidgetsLibDir}libwxregexu-${wxVer}.a
    ${wxWidgetsLibDir}libwxscintilla-${wxVer}.a
#   ${wxWidgetsLibDir}libwxpng-${wxVer}.a
#   ${wxWidgetsLibDir}libwxexpat-${wxVer}.a
#   ${wxWidgetsLibDir}libwxjpeg-${wxVer}.a
  )

  target_sources(${PROJECT_NAME} PUBLIC
    ${SRCRESDIR}/${PROJECT_NAME}.rc
  )
  set(CMAKE_RC_COMPILER_INIT windres.exe)
  enable_language(RC)

  set(WindowsLibs
    -limm32
    -lopengl32
    -lglu32
    -lexpat
    -lpcre2-16
    -lpng
    -ltiff
    -ljbig
    -lz
    -lrpcrt4
    -loleaut32
    -lole32
    -luuid
    -luxtheme
    -lwinspool
    -lwinmm
    -lshell32
    -lshlwapi
    -lcomctl32
    -ladvapi32
    -lversion
    -lws2_32
    -lgdi32
    -loleacc
    -lwinhttp
  )

  SET(CMAKE_RC_COMPILE_OBJECT
    "<CMAKE_RC_COMPILER> -i <SOURCE> <DEFINES> <FLAGS> <INCLUDES> -O coff -o <OBJECT>")

  target_compile_definitions(${PROJECT_NAME} PUBLIC ${wxWidgetsDefines})
  target_include_directories(${PROJECT_NAME} PUBLIC ${SRCDIR} ${wxWidgetsIncludeDir})
  target_link_directories(${PROJECT_NAME} PUBLIC ${wxWidgetsLibDir})
  target_link_libraries(${PROJECT_NAME} PUBLIC ${RESOURCE_OBJECT} ${wxWidgetsLibs} ${WindowsLibs})
  target_link_options(${PROJECT_NAME} PUBLIC -static -Wl,--subsystem,windows -mwindows)

  install(PROGRAMS ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.exe DESTINATION Release)
  install(FILES font/prn_font_l3.dat DESTINATION Release)
  install(FILES font/prn_font_m8.dat DESTINATION Release)
  install(DIRECTORY lang DESTINATION Release)

endif()

