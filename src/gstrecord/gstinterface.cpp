// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gstinterface.h"
#include "../utils/log.h"

#include <QDebug>
#include <QLibraryInfo>
#include <QDir>

gstInterface::p_gst_message_parse_error gstInterface::m_gst_message_parse_error = nullptr;
gstInterface::p_g_free gstInterface::m_g_free = nullptr;
gstInterface::p_g_error_copy gstInterface::m_g_error_copy = nullptr;
gstInterface::p_g_main_loop_quit gstInterface::m_g_main_loop_quit = nullptr;
gstInterface::p_g_main_loop_run gstInterface::m_g_main_loop_run = nullptr;
gstInterface::p_g_main_loop_new gstInterface::m_g_main_loop_new = nullptr;
gstInterface::p_g_malloc gstInterface::m_g_malloc = nullptr;

gstInterface::p_gst_init gstInterface::m_gst_init = nullptr;
gstInterface::p_gst_element_set_state gstInterface::m_gst_element_set_state = nullptr;
gstInterface::p_gst_object_unref gstInterface::m_gst_object_unref = nullptr;
gstInterface::p_gst_bin_get_by_name gstInterface::m_gst_bin_get_by_name = nullptr;
gstInterface::p_gst_pipeline_get_bus gstInterface::m_gst_pipeline_get_bus = nullptr;
gstInterface::p_gst_bus_add_watch gstInterface::m_gst_bus_add_watch = nullptr;
gstInterface::p_gst_buffer_new_wrapped gstInterface::m_gst_buffer_new_wrapped = nullptr;
gstInterface::p_gst_clock_get_time gstInterface::m_gst_clock_get_time = nullptr;
gstInterface::p_gst_buffer_unref gstInterface::m_gst_buffer_unref = nullptr;
gstInterface::p_gst_mini_object_unref gstInterface::m_gst_mini_object_unref = nullptr;
gstInterface::p_gst_element_send_event gstInterface::m_gst_element_send_event = nullptr;
gstInterface::p_gst_event_new_eos gstInterface::m_gst_event_new_eos = nullptr;
gstInterface::p_gst_bus_timed_pop_filtered gstInterface::m_gst_bus_timed_pop_filtered = nullptr;
gstInterface::p_gst_parse_launch gstInterface::m_gst_parse_launch = nullptr;
gstInterface::p_gst_bin_get_type gstInterface::m_gst_bin_get_type = nullptr;

gstInterface::p_g_type_check_instance_cast gstInterface::m_g_type_check_instance_cast = nullptr;
gstInterface::p_g_signal_emit_by_name gstInterface::m_g_signal_emit_by_name = nullptr;
gstInterface::p_g_object_set gstInterface::m_g_object_set = nullptr;


bool gstInterface::m_isInitFunction = false;
QLibrary gstInterface::m_libgstreamer;
QLibrary gstInterface::m_libglib;
QLibrary gstInterface::m_libgobject;

gstInterface::gstInterface()
{
    qCDebug(dsrApp) << "gstInterface constructor called.";
}
QString gstInterface::libPath(const QString &sLib)
{
    qCDebug(dsrApp) << "libPath method called with sLib:" << sLib;
    qInfo() << "gstreamer lib name is " << sLib;
    QDir dir;
    QString path  = QLibraryInfo::location(QLibraryInfo::LibrariesPath);
    dir.setPath(path);
    QStringList list = dir.entryList(QStringList() << (sLib + "*"), QDir::NoDotAndDotDot | QDir::Files); //filter name with strlib
    if (list.isEmpty()) {
        qWarning() << "list is empty!";
        qCDebug(dsrApp) << "Library list is empty for:" << sLib;
    }
    if (list.contains(sLib)) {
        qCDebug(dsrApp) << "Exact library found:" << sLib;
        return sLib;
    } else {
        qCDebug(dsrApp) << "Library list:" << list;
        list.sort();
    }
    qCDebug(dsrApp) << "Returning last library in sorted list:" << list.last();
    //Q_ASSERT(list.size() > 0);
    return list.last();
}
void gstInterface::initFunctions()
{
    qCDebug(dsrApp) << "initFunctions method called.";
    if (m_isInitFunction) {
        qCDebug(dsrApp) << "Functions already initialized. Skipping.";
        return;
    }

    m_libgstreamer.setFileName(libPath("libgstreamer-1.0.so"));
    m_libglib.setFileName(libPath("libglib-2.0.so"));
    m_libgobject.setFileName(libPath("libgobject-2.0.so"));
    qCDebug(dsrApp) << "Library file names set.";

    qCDebug(dsrApp) << "libgstreamer-1.0 is load? " << m_libgstreamer.load();
    qCDebug(dsrApp) << "libglib-2.0 is load? " << m_libglib.load();
    qCDebug(dsrApp) << "libgobject-2.0 is load? " << m_libgobject.load();

    m_gst_message_parse_error = reinterpret_cast<p_gst_message_parse_error>(m_libglib.resolve("gst_message_parse_error")); // -lglib-2.0
    m_g_free = reinterpret_cast<p_g_free>(m_libglib.resolve("g_free")); // -lglib-2.0
    m_g_error_copy = reinterpret_cast<p_g_error_copy>(m_libglib.resolve("g_error_copy")); // -lglib-2.0
    m_g_main_loop_quit = reinterpret_cast<p_g_main_loop_quit>(m_libglib.resolve("g_main_loop_quit")); // -lglib-2.0
    m_g_main_loop_run = reinterpret_cast<p_g_main_loop_run>(m_libglib.resolve("g_main_loop_run")); // -lglib-20
    m_g_main_loop_new = reinterpret_cast<p_g_main_loop_new>(m_libglib.resolve("g_main_loop_new")); // -lglib-2.0
    m_g_malloc = reinterpret_cast<p_g_malloc>(m_libglib.resolve("g_malloc")); // -lglib-2.0

    m_gst_init = reinterpret_cast<p_gst_init>(m_libgstreamer.resolve("gst_init")); // -lgstreamer-1.0
    m_gst_element_set_state = reinterpret_cast<p_gst_element_set_state>(m_libgstreamer.resolve("gst_element_set_state")); // -lgstreamer-1.0
    m_gst_object_unref = reinterpret_cast<p_gst_object_unref>(m_libgstreamer.resolve("gst_object_unref")); // -lgstreamer-1.0
    m_gst_bin_get_by_name = reinterpret_cast<p_gst_bin_get_by_name>(m_libgstreamer.resolve("gst_bin_get_by_name")); // -lgstreamer-1.0
    m_gst_pipeline_get_bus = reinterpret_cast<p_gst_pipeline_get_bus>(m_libgstreamer.resolve("gst_pipeline_get_bus")); // -lgstreamer-1.0
    m_gst_bus_add_watch = reinterpret_cast<p_gst_bus_add_watch>(m_libgstreamer.resolve("gst_bus_add_watch")); // -lgstreamer-1.0
    m_gst_buffer_new_wrapped = reinterpret_cast<p_gst_buffer_new_wrapped>(m_libgstreamer.resolve("gst_buffer_new_wrapped")); // -lgstreamer-1.0
    m_gst_clock_get_time = reinterpret_cast<p_gst_clock_get_time>(m_libgstreamer.resolve("gst_clock_get_time")); // -lgstreamer-1.0
    m_gst_buffer_unref = reinterpret_cast<p_gst_buffer_unref>(m_libgstreamer.resolve("gst_buffer_unref")); // -lgstreamer-1.0
    m_gst_mini_object_unref = reinterpret_cast<p_gst_mini_object_unref>(m_libgstreamer.resolve("gst_mini_object_unref")); // -lgstreamer-1.0
    m_gst_element_send_event = reinterpret_cast<p_gst_element_send_event>(m_libgstreamer.resolve("gst_element_send_event")); // -lgstreamer-1.0
    m_gst_event_new_eos = reinterpret_cast<p_gst_event_new_eos>(m_libgstreamer.resolve("gst_event_new_eos")); // -lgstreamer-1.0
    m_gst_bus_timed_pop_filtered = reinterpret_cast<p_gst_bus_timed_pop_filtered>(m_libgstreamer.resolve("gst_bus_timed_pop_filtered")); // -lgstreamer-1.0
    m_gst_parse_launch = reinterpret_cast<p_gst_parse_launch>(m_libgstreamer.resolve("gst_parse_launch")); // -lgstreamer-1.0
    m_gst_bin_get_type = reinterpret_cast<p_gst_bin_get_type>(m_libgstreamer.resolve("gst_bin_get_type")); // -lgstreamer-1.0

    m_g_type_check_instance_cast = reinterpret_cast<p_g_type_check_instance_cast>(m_libgobject.resolve("g_type_check_instance_cast")); //-lgobject-2.0
    m_g_object_set = reinterpret_cast<p_g_object_set>(m_libgobject.resolve("g_object_set")); //-lgobject-2.0
    m_g_signal_emit_by_name = reinterpret_cast<p_g_signal_emit_by_name>(m_libgobject.resolve("g_signal_emit_by_name")); // -lgobject-2.0
    qCDebug(dsrApp) << "All Glib, GStreamer, and GObject functions resolved.";

    qCDebug(dsrApp) << "gstreamer-1.0 function is load";

}

void gstInterface::unloadFunctions()
{
    qCDebug(dsrApp) << "unloadFunctions method called.";
    if (m_isInitFunction) {
        m_libgstreamer.unload();
        qCDebug(dsrApp) << "libgstreamer-1.0 unloaded.";
        m_libglib.unload();
        qCDebug(dsrApp) << "libglib-2.0 unloaded.";
        m_libgobject.unload();
        qCDebug(dsrApp) << "libgobject-2.0 unloaded.";
    }
    qCDebug(dsrApp) << "unloadFunctions method finished.";
}


