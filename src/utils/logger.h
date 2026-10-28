// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DSR_LOGGER_H
#define DSR_LOGGER_H

#include <QObject>
#include <dtkcore_global.h>

#ifdef DTKCORE_CLASS_DConfigFile
DCORE_BEGIN_NAMESPACE
class DConfig;
DCORE_END_NAMESPACE
#endif

/**
 * @brief MLogger 动态日志规则监听器
 *
 * 参考 deepin-reader 的 MLogger 实现：启动时读取 DConfig 的 log_rules
 * 并监听其变更，实现运行中动态调整本应用日志级别（deepin-debug-config
 * 调试模式适配）。
 *
 * 注意：实例必须尽早在 main() 中创建（在任何日志打印、DApplication
 * 实例化之前），且生命周期需覆盖整个 main，以保证 valueChanged
 * 监听持续有效。
 */
class MLogger : public QObject
{
    Q_OBJECT
public:
    explicit MLogger(QObject *parent = nullptr);
    ~MLogger();

    inline QString rules() const { return m_rules; }
    void setRules(const QString &rules);

private:
    void appendRules(const QString &rules);

private:
    QString m_rules;
#ifdef DTKCORE_CLASS_DConfigFile
    Dtk::Core::DConfig *m_config;
#endif
};

#endif   // DSR_LOGGER_H
