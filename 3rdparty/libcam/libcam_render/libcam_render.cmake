set(libcam_render_HEAD
  ${CMAKE_CURRENT_LIST_DIR}/gview.h
  ${CMAKE_CURRENT_LIST_DIR}/gviewrender.h
  ${CMAKE_CURRENT_LIST_DIR}/render.h

)
set(libcam_render_SRC 
  ${CMAKE_CURRENT_LIST_DIR}/render.c
  ${CMAKE_CURRENT_LIST_DIR}/render_fx.c 
  ${CMAKE_CURRENT_LIST_DIR}/render_osd_crosshair.c 
  ${CMAKE_CURRENT_LIST_DIR}/render_osd_vu_meter.c 
)
include_directories(${CMAKE_CURRENT_LIST_DIR})
