#include "FrameRenderer.h"


namespace baresprite
{
FrameRenderer::FrameRenderer()
{
}

FrameRenderer::~FrameRenderer() = default;


void FrameRenderer::Render(const Frame &frame, int _checkerSize, HDC hdcMem)
{
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = frame.width;
    bmi.bmiHeader.biHeight = -frame.height; // Top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    int logicalW = frame.width;
    int logicalH = frame.height;
    int screenW = logicalW * _checkerSize;
    int screenH = logicalH * _checkerSize;

    // Создаём временный DC для кадра
    HDC hdcFrame = CreateCompatibleDC(hdcMem);
    HBITMAP hFrameBmp = CreateCompatibleBitmap(hdcMem, logicalW, logicalH);
    HGDIOBJ oldFrameBmp = SelectObject(hdcFrame, hFrameBmp);

    // Рисуем кадр во временный DC
    SetDIBitsToDevice(hdcFrame, 0, 0, logicalW, logicalH, 0, 0, 0, logicalH, frame.pixels.get(), &bmi, DIB_RGB_COLORS);

    // Настраиваем альфа-смешивание
    BLENDFUNCTION blend = {};
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.SourceConstantAlpha = 255;  // Полная непрозрачность
    blend.AlphaFormat = AC_SRC_ALPHA; // Используем альфа-канал из пикселей

    // Растягиваем с альфа-смешиванием (прозрачные пиксели не перекроют шахматку)
    AlphaBlend(hdcMem, 0, 0, screenW, screenH, hdcFrame, 0, 0, logicalW, logicalH, blend);

    // Очистка временного DC
    SelectObject(hdcFrame, oldFrameBmp);
    DeleteObject(hFrameBmp);
    DeleteDC(hdcFrame);
}

} // namespace baresprite