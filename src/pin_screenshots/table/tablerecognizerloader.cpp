// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tablerecognizerloader.h"

#include <QDebug>

#include <chrono>

#include "../../utils/log.h"

TableRecognizerLoader::TableRecognizerLoader(QObject *parent)
    : QObject(parent)
    , m_recognizer(new Dtk::TableRecognizer::DTableRecognizer(this))
{
    connect(m_recognizer, &Dtk::TableRecognizer::DTableRecognizer::recognitionDone, this,
            [this](const Dtk::TableRecognizer::DTableResult &result) {
                qCDebug(dsrApp) << "[TABLE] recognition done. success:" << result.success
                                << "source:" << result.source
                                << "error:" << static_cast<int>(result.error)
                                << "message:" << result.errorMessage
                                << "html size:" << result.html.size();
                emit recognitionFinished(result.success, result.html, result.error);
            });
}

bool TableRecognizerLoader::recognize(const QImage &image, int timeoutMs)
{
    if (image.isNull()) {
        emit recognitionFinished(false, QString(), Dtk::TableRecognizer::TableError::InvalidImage);
        return false;
    }
    qCDebug(dsrApp) << "[TABLE] recognition started, timeout(ms):" << timeoutMs;
    m_recognizer->recognizeAsync(image, std::chrono::milliseconds(timeoutMs));
    return true;
}
