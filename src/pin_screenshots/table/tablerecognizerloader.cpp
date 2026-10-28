// Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "tablerecognizerloader.h"

#include <QDebug>

#ifdef HAVE_DTK6TABLERECOGNIZER
#include <chrono>
#endif

#include "../../utils/log.h"

TableRecognizerLoader::TableRecognizerLoader(QObject *parent)
    : QObject(parent)
#ifdef HAVE_DTK6TABLERECOGNIZER
    , m_recognizer(new Dtk::TableRecognizer::DTableRecognizer(this))
#endif
{
#ifdef HAVE_DTK6TABLERECOGNIZER
    connect(m_recognizer, &Dtk::TableRecognizer::DTableRecognizer::recognitionDone, this,
            [this](const Dtk::TableRecognizer::DTableResult &result) {
                qCDebug(dsrApp) << "[TABLE] recognition done. success:" << result.success
                                << "source:" << result.source
                                << "error:" << static_cast<int>(result.error)
                                << "message:" << result.errorMessage
                                << "html size:" << result.html.size();
                emit recognitionFinished(result.success, result.html, result.error);
            });
#else
    qCDebug(dsrApp) << "[TABLE] stub loader: dtk6tablerecognizer not available";
#endif
}

bool TableRecognizerLoader::recognize(const QImage &image, int timeoutMs)
{
#ifdef HAVE_DTK6TABLERECOGNIZER
    if (image.isNull()) {
        emit recognitionFinished(false, QString(), Dtk::TableRecognizer::TableError::InvalidImage);
        return false;
    }
    qCDebug(dsrApp) << "[TABLE] recognition started, timeout(ms):" << timeoutMs;
    m_recognizer->recognizeAsync(image, std::chrono::milliseconds(timeoutMs));
    return true;
#else
    Q_UNUSED(image)
    Q_UNUSED(timeoutMs)
    qCWarning(dsrApp) << "[TABLE] recognition not available: dtk6tablerecognizer not installed";
    emit recognitionFinished(false, QString(), Dtk::TableRecognizer::TableError::NotAvailable);
    return false;
#endif
}
