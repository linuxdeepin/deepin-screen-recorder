// Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONFIGSETTINGS_H
#define CONFIGSETTINGS_H

#include <QObject>
#include <QSettings>
#include <QMutex>
#include <utility>

enum SaveAction : unsigned int;
class ConfigSettings : public QObject
{
    Q_OBJECT
public:
    static ConfigSettings *instance();

    /*
    void setTemporarySaveAction(const std::pair<bool, SaveAction> temporarySaveAction);
    inline std::pair<bool, SaveAction> getTemporarySaveAction()
    {
        return m_temporarySaveOp;
    }
    */
    void setValue(const QString &group, const QString &key,
                  QVariant val);
    QVariant value(const QString &group, const QString &key,
                   const QVariant &defaultValue = QVariant());
    QStringList keys(const QString &group);

    QVariant getDefaultValue(const QString &group, const QString &key);

signals:
    void colorChanged();
    void shapeConfigChanged(const QString &shape,  const QString &key, int index);
    void straightLineConfigChanged(bool isStraightLine);

private:
    explicit ConfigSettings(QObject *parent = nullptr);
    ~ConfigSettings();

    static ConfigSettings *m_configSettings;
    //std::pair<bool, SaveAction> m_temporarySaveOp;
    QSettings *m_settings;
    QMutex m_mutex;
};
#endif // CONFIGSETTINGS_H
