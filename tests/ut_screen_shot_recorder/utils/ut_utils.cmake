set(ut_utils_HEAD
  ${CMAKE_CURRENT_LIST_DIR}/ut_audioutils.cpp
  ${CMAKE_CURRENT_LIST_DIR}/ut_baseutils.cpp
  ${CMAKE_CURRENT_LIST_DIR}/ut_calculaterect.cpp
  ${CMAKE_CURRENT_LIST_DIR}/ut_configsettings.cpp
  ${CMAKE_CURRENT_LIST_DIR}/ut_dbusutils.cpp
  ${CMAKE_CURRENT_LIST_DIR}/ut_desktopinfo.cpp
  ${CMAKE_CURRENT_LIST_DIR}/ut_screengrabber.cpp
  ${CMAKE_CURRENT_LIST_DIR}/ut_shortcut.cpp
  ${CMAKE_CURRENT_LIST_DIR}/ut_tempfile.cpp
  ${CMAKE_CURRENT_LIST_DIR}/ut_toolshape.cpp
  ${CMAKE_CURRENT_LIST_DIR}/ut_utils_other.cpp
  ${CMAKE_CURRENT_LIST_DIR}/ut_voiceVolumeWatcher.cpp
)
if(USE_DEEPIN_WAYLAND)
  list(APPEND ut_utils_HEAD 
    ${CMAKE_CURRENT_LIST_DIR}/ut_WaylandScrollMonitor.cpp
  )
endif()
if(OCR_SCROLL_FLAGE_ON)
  list(APPEND ut_utils_HEAD 
    ${CMAKE_CURRENT_LIST_DIR}/ut_scrollScreenshot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ut_pixmergethread.cpp
  )
endif()
