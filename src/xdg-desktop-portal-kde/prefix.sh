export PATH=/usr/local/bin:$PATH

# LD_LIBRARY_PATH only needed if you are building without rpath
# export LD_LIBRARY_PATH=/usr/local/lib/aarch64-linux-gnu:$LD_LIBRARY_PATH

export XDG_DATA_DIRS=/usr/local/share:${XDG_DATA_DIRS:-/usr/local/share/:/usr/share/}
export XDG_CONFIG_DIRS=/usr/local/etc/xdg:${XDG_CONFIG_DIRS:-/etc/xdg}

export QT_PLUGIN_PATH=/usr/local/lib/aarch64-linux-gnu/plugins:$QT_PLUGIN_PATH
export QML2_IMPORT_PATH=/usr/local/lib/aarch64-linux-gnu/qml:$QML2_IMPORT_PATH
