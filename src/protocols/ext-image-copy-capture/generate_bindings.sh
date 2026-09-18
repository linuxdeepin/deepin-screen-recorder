#!/bin/bash

# ext-image-copy-capture-v1 协议绑定生成脚本
# 这个脚本用于从XML协议文件生成C协议绑定和Qt C++封装

PROTOCOL_XML="ext-image-copy-capture-v1.xml"
PROTOCOL_PREFIX="ext-image-copy-capture-v1"

echo "正在生成 ${PROTOCOL_PREFIX} 协议绑定文件..."

# 生成C协议绑定文件
echo "1. 生成C客户端协议头文件..."
wayland-scanner client-header "${PROTOCOL_XML}" "wayland-${PROTOCOL_PREFIX}-client-protocol.h"

echo "2. 生成C协议实现文件..."
wayland-scanner private-code "${PROTOCOL_XML}" "wayland-${PROTOCOL_PREFIX}-protocol.c"

# 生成Qt C++封装文件
echo "3. 生成Qt C++客户端头文件..."
/usr/lib/qt6/libexec/qtwaylandscanner client-header "${PROTOCOL_XML}" "qwayland-${PROTOCOL_PREFIX}.h"

echo "4. 生成Qt C++客户端实现文件..."
/usr/lib/qt6/libexec/qtwaylandscanner client-code "${PROTOCOL_XML}" "qwayland-${PROTOCOL_PREFIX}.cpp"

echo "协议绑定文件生成完成！"
echo ""
echo "生成的文件："
echo "  - wayland-${PROTOCOL_PREFIX}-client-protocol.h  (C协议头文件)"
echo "  - wayland-${PROTOCOL_PREFIX}-protocol.c         (C协议实现)"
echo "  - qwayland-${PROTOCOL_PREFIX}.h                 (Qt C++头文件)"
echo "  - qwayland-${PROTOCOL_PREFIX}.cpp               (Qt C++实现)"
