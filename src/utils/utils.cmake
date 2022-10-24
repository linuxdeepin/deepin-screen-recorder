set(utils_HEAD
  ${CMAKE_CURRENT_LIST_DIR}/baseutils.h
  ${CMAKE_CURRENT_LIST_DIR}/eventlogutils.h
  ${CMAKE_CURRENT_LIST_DIR}/calculaterect.h
  ${CMAKE_CURRENT_LIST_DIR}/saveutils.h
  ${CMAKE_CURRENT_LIST_DIR}/shapesutils.h
  ${CMAKE_CURRENT_LIST_DIR}/dbusutils.h
  ${CMAKE_CURRENT_LIST_DIR}/desktopinfo.h
  ${CMAKE_CURRENT_LIST_DIR}/audioutils.h
  ${CMAKE_CURRENT_LIST_DIR}/configsettings.h
  ${CMAKE_CURRENT_LIST_DIR}/shortcut.h
  ${CMAKE_CURRENT_LIST_DIR}/tempfile.h
  ${CMAKE_CURRENT_LIST_DIR}/voicevolumewatcher.h
  ${CMAKE_CURRENT_LIST_DIR}/camerawatcher.h
  ${CMAKE_CURRENT_LIST_DIR}/screengrabber.h
)
set(utils_SRC 
  ${CMAKE_CURRENT_LIST_DIR}/audioutils.cpp
  ${CMAKE_CURRENT_LIST_DIR}/shapesutils.cpp
  ${CMAKE_CURRENT_LIST_DIR}/tempfile.cpp
  ${CMAKE_CURRENT_LIST_DIR}/eventlogutils.cpp
  ${CMAKE_CURRENT_LIST_DIR}/calculaterect.cpp
  ${CMAKE_CURRENT_LIST_DIR}/shortcut.cpp
  ${CMAKE_CURRENT_LIST_DIR}/configsettings.cpp
  ${CMAKE_CURRENT_LIST_DIR}/baseutils.cpp
  ${CMAKE_CURRENT_LIST_DIR}/dbusutils.cpp
  ${CMAKE_CURRENT_LIST_DIR}/voicevolumewatcher.cpp
  ${CMAKE_CURRENT_LIST_DIR}/camerawatcher.cpp
  ${CMAKE_CURRENT_LIST_DIR}/desktopinfo.cpp
  ${CMAKE_CURRENT_LIST_DIR}/screengrabber.cpp
)
set(utils_WAYLAND
  ${CMAKE_CURRENT_LIST_DIR}/waylandscrollmonitor.h
  ${CMAKE_CURRENT_LIST_DIR}/waylandscrollmonitor.cpp
)
set(utils_OPENCV
  ${CMAKE_CURRENT_LIST_DIR}/pixmergethread.h
  ${CMAKE_CURRENT_LIST_DIR}/scrollScreenshot.h
  ${CMAKE_CURRENT_LIST_DIR}/pixmergethread.cpp
  ${CMAKE_CURRENT_LIST_DIR}/scrollScreenshot.cpp
)
