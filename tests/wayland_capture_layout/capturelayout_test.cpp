#include "capturelayout.h"
#include <QImage>
#include <QPainter>
#include <cassert>
#include <cstdio>

int main()
{
    const QRect desktop(0, 0, 3840, 1280);
    const QRect first(0, 0, 1920, 1280), second(1920, 0, 1920, 1080);
    const QRect firstTarget = CaptureLayout::mapRect(first, desktop.topLeft(), 1.5);
    const QRect secondTarget = CaptureLayout::mapRect(second, desktop.topLeft(), 1.5);
    assert(firstTarget == QRect(0, 0, 2880, 1920));
    assert(secondTarget == QRect(2880, 0, 2880, 1620));
    QImage a(2880, 1920, QImage::Format_RGB32), b(1920, 1080, QImage::Format_RGB32);
    a.fill(Qt::red); b.fill(Qt::green);
    QImage canvas(5760, 1920, QImage::Format_RGB32); canvas.fill(Qt::black);
    QPainter painter(&canvas);
    painter.drawImage(firstTarget, a, a.rect());
    painter.drawImage(secondTarget, b, b.rect()); painter.end();
    assert(canvas.pixelColor(2500, 500) == QColor(Qt::red));
    assert(canvas.pixelColor(2879, 500) == QColor(Qt::red));
    assert(canvas.pixelColor(2880, 500) == QColor(Qt::green));
    assert(canvas.pixelColor(5000, 1500) == QColor(Qt::green));
    assert(canvas.pixelColor(5000, 1700) == QColor(Qt::black));
    const QRect cross = CaptureLayout::mapRect(QRect(1600, 200, 800, 600), QPoint(), 1.5);
    assert(cross == QRect(2400, 300, 1200, 900));
    const auto crop = canvas.copy(cross);
    assert(crop.pixelColor(479, 20) == QColor(Qt::red));
    assert(crop.pixelColor(480, 20) == QColor(Qt::green));
    // Adjacent fractional edges must coincide even where logical widths are odd.
    const auto left = CaptureLayout::mapRect(QRect(-101, -40, 101, 99), QPoint(-101, -40), 1.25);
    const auto right = CaptureLayout::mapRect(QRect(0, -40, 103, 77), QPoint(-101, -40), 1.25);
    assert(left.x() + left.width() == right.x());
    const auto upper = CaptureLayout::mapRect(QRect(0, -200, 100, 200), QPoint(0, -200), 1.5);
    const auto lower = CaptureLayout::mapRect(QRect(0, 0, 80, 100), QPoint(0, -200), 1.5);
    assert(upper.y() + upper.height() == lower.y());
    // DMA pitch includes padding; the copied image must preserve every row.
    uchar bytes[64] = {};
    for (int y = 0; y < 2; ++y) {
        auto row = reinterpret_cast<QRgb *>(bytes + y * 32);
        for (int x = 0; x < 3; ++x) row[x] = qRgb(y * 100, x * 50, 20);
    }
    QImage packed = QImage(bytes, 3, 2, 32, QImage::Format_RGB32).copy();
    assert(packed.pixelColor(2, 1) == QColor(100, 100, 20));
    std::puts("PASS: mixed-scale seams, crop, negative origin, vertical layout, fractional edges and padded DMA stride");
}
