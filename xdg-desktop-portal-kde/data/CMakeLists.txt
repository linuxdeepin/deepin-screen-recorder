configure_file(org.freedesktop.impl.portal.desktop.kde.cmake.in org.freedesktop.impl.portal.desktop.kde.service @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.freedesktop.impl.portal.desktop.kde.service DESTINATION ${KDE_INSTALL_DBUSSERVICEDIR})
install(FILES kde.portal DESTINATION ${DATA_INSTALL_DIR}/xdg-desktop-portal/portals)
