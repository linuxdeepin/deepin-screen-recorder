# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

include_guard(GLOBAL)

include(GNUInstallDirs)

function(dsr_detect_app_version _out_var)
    set(_version "0.0.0")
    set(_changelog "${CMAKE_SOURCE_DIR}/debian/changelog")
    if(EXISTS "${_changelog}")
        file(STRINGS "${_changelog}" _first_line LIMIT_COUNT 1
            REGEX "^deepin-screen-recorder[ \t]*\\(")
        if(_first_line MATCHES "\\(([^)]+)\\)")
            set(_version "${CMAKE_MATCH_1}")
            string(REGEX REPLACE "-[^-]*$" "" _version "${_version}")
        endif()
    endif()
    set(${_out_var} "${_version}" PARENT_SCOPE)
endfunction()

function(dsr_detect_platform)
    set(_os_file "/etc/os-version")
    set(_community FALSE)
    set(_minor "")
    set(_build_suffix "")

    if(EXISTS "${_os_file}")
        file(READ "${_os_file}" _content)
        if(_content MATCHES "Community")
            set(_community TRUE)
        endif()
        if(_content MATCHES "MinorVersion=([0-9]+)")
            set(_minor "${CMAKE_MATCH_1}")
        endif()
        if(_content MATCHES "OsBuild=[0-9]+\\.([0-9]+)")
            set(_build_suffix "${CMAKE_MATCH_1}")
        endif()
    endif()

    set(_dde_start OFF)
    set(_ocr_scroll OFF)
    set(_kf5_wayland OFF)
    set(_kwayland_remote OFF)
    set(_kwayland_release OFF)

    if(_community OR (NOT _minor STREQUAL "" AND _minor GREATER_EQUAL 2500))
        set(_dde_start ON)
        set(_ocr_scroll ON)
    elseif(NOT _minor STREQUAL "")
        if(_minor GREATER 1029)
            set(_dde_start ON)
        endif()
        if(_minor GREATER 1039)
            set(_ocr_scroll ON)
        endif()
        if(_minor GREATER 1049)
            set(_kf5_wayland ON)
            if(NOT _build_suffix STREQUAL "")
                if(_build_suffix GREATER 109)
                    set(_kwayland_remote ON)
                endif()
                if(_build_suffix GREATER 110)
                    set(_kwayland_release ON)
                endif()
            endif()
            if(_minor GREATER 1059)
                set(_kwayland_remote ON)
                set(_kwayland_release ON)
            endif()
        endif()
    endif()

    set(DSR_DETECTED_DDE_START "${_dde_start}" PARENT_SCOPE)
    set(DSR_DETECTED_OCR_SCROLL "${_ocr_scroll}" PARENT_SCOPE)
    set(DSR_DETECTED_KF5_WAYLAND "${_kf5_wayland}" PARENT_SCOPE)
    set(DSR_DETECTED_KWAYLAND_REMOTE "${_kwayland_remote}" PARENT_SCOPE)
    set(DSR_DETECTED_KWAYLAND_RELEASE "${_kwayland_release}" PARENT_SCOPE)
endfunction()

function(dsr_set_common_compile_options _target)
    target_compile_features(${_target} PRIVATE cxx_std_17)
    target_compile_options(${_target} PRIVATE
        -Wall
        -Wextra
        -Wno-error=deprecated-declarations
        -Wno-deprecated-declarations
    )

    # qmake release builds define QT_NO_DEBUG; keep the same semantics so
    # Q_ASSERT and release-only behaviour match. Only Debug opts out, which
    # also mirrors the qmake debug_and_release split.
    if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_definitions(${_target} PRIVATE QT_NO_DEBUG)
    endif()
endfunction()

# Reproduce the hardening flags that src.pro / pin_screenshots.pro set on the
# executables (Debian packaging only injects part of them through dpkg-buildflags).
function(dsr_apply_executable_hardening _target)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        return()
    endif()

    target_compile_options(${_target} PRIVATE
        -fPIE
        -fstack-protector-strong
    )
    target_link_options(${_target} PRIVATE
        -pie
        -Wl,--as-needed
        -Wl,-z,noexecstack
        -Wl,-z,now
        -Wl,-O1
    )
endfunction()

# Loongson/MIPS tuning that only src.pro applied to the main program.
function(dsr_apply_arch_flags _target)
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(mips64|loongson)")
        target_compile_options(${_target} PRIVATE
            -O3
            -ftree-vectorize
            -march=loongson3a
            -mhard-float
            -mno-micromips
            -mno-mips16
            -flax-vector-conversions
            -mloongson-ext2
            -mloongson-mmi
        )
    endif()
endfunction()

function(dsr_apply_platform_definitions _target)
    target_compile_definitions(${_target} PRIVATE
        APP_VERSION="${DSR_APP_VERSION}"
        _FORTIFY_SOURCE=2
    )

    if(DSR_ENABLE_DDE_START)
        target_compile_definitions(${_target} PRIVATE DDE_START_FLAGE_ON)
    endif()
    if(DSR_ENABLE_OCR_SCROLL)
        target_compile_definitions(${_target} PRIVATE OCR_SCROLL_FLAGE_ON)
    endif()
    if(DSR_ENABLE_KF5_WAYLAND)
        target_compile_definitions(${_target} PRIVATE KF5_WAYLAND_FLAGE_ON)
    endif()
    if(DSR_ENABLE_KWAYLAND_REMOTE)
        target_compile_definitions(${_target} PRIVATE KWAYLAND_REMOTE_FLAGE_ON)
    endif()
    if(DSR_ENABLE_KWAYLAND_RELEASE)
        target_compile_definitions(${_target} PRIVATE KWAYLAND_REMOTE_BUFFER_RELEASE_FLAGE_ON)
    endif()
    if(DSR_ENABLE_TABLE_RECOGNIZER)
        target_compile_definitions(${_target} PRIVATE DSR_HAVE_TABLE_RECOGNIZER)
    endif()
endfunction()

function(dsr_add_dtk_fallback_includes _target)
    foreach(_dir /usr/include/dtk6/DWidget /usr/include/dtk6/DGui /usr/include/dtk6/DCore /usr/include/dtk6/DLog)
        if(EXISTS "${_dir}")
            target_include_directories(${_target} PRIVATE "${_dir}")
        endif()
    endforeach()
endfunction()
