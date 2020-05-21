#.rst:
# FindEpoxy
# -------
#
# - Try to find libepoxy.
#
# This will define the following variables:
#
#  ``Epoxy_FOUND``
#     TRUE if libepoxy was found
#  ``Epoxy_LIBRARIES``
#     Pass this variable to target_link_libraries()
#  ``Epoxy_INCLUDE_DIRS``
#     This should be passed to target_include_directories() to use the
#     libepoxy headers
#  ``Epoxy_DEFINITIONS``
#     Can be passed to target_compile_options() if necessary
#
#  ``Epoxy_HAS_GLX``
#     Whether GLX support is available

# Copyright (c) 2014 Fredrik Höglund <fredrik@kde.org>
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the University nor the names of its contributors
#    may be used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
# OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
# HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
# OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
# SUCH DAMAGE.

find_package(PkgConfig QUIET)
pkg_check_modules(PKG_Epoxy QUIET epoxy)

set(Epoxy_DEFINITIONS "${PKG_Epoxy_CFLAGS}")

find_path(Epoxy_INCLUDE_DIRS NAMES epoxy/gl.h HINTS ${PKG_Epoxy_INCLUDEDIR} ${PKG_Epoxy_INCLUDE_DIRS})
find_library(Epoxy_LIBRARIES  NAMES epoxy      HINTS ${PKG_Epoxy_LIBDIR} ${PKG_Epoxy_LIBRARIES_DIRS})
find_file(Epoxy_GLX_HEADER NAMES epoxy/glx.h HINTS ${Epoxy_INCLUDE_DIRS} DOC "whether GLX is available")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Epoxy DEFAULT_MSG Epoxy_LIBRARIES Epoxy_INCLUDE_DIRS)

mark_as_advanced(Epoxy_INCLUDE_DIRS Epoxy_LIBRARIES Epoxy_HAS_GLX)
