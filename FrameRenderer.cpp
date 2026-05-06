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

    // Create a temporary DC for the frame
    HDC hdcFrame = CreateCompatibleDC(hdcMem);
    HBITMAP hFrameBmp = CreateCompatibleBitmap(hdcMem, logicalW, logicalH);
    HGDIOBJ oldFrameBmp = SelectObject(hdcFrame, hFrameBmp);

    // Draw a frame into a temporary DC
    SetDIBitsToDevice(hdcFrame, 0, 0, logicalW, logicalH, 0, 0, 0, logicalH, frame.pixels.get(), &bmi, DIB_RGB_COLORS);

    // Setting up alpha blending
    BLENDFUNCTION blend = {};
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.SourceConstantAlpha = 255;  // Complete opacity
    blend.AlphaFormat = AC_SRC_ALPHA; // Using an alpha channel from pixels

    // Stretch with alpha blending (transparent pixels won't overlap the checkerboard)
    AlphaBlend(hdcMem, 0, 0, screenW, screenH, hdcFrame, 0, 0, logicalW, logicalH, blend);

    // Clearing the temporary DC
    SelectObject(hdcFrame, oldFrameBmp);
    DeleteObject(hFrameBmp);
    DeleteDC(hdcFrame);
}

} // namespace baresprite