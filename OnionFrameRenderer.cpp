#include "OnionFrameRenderer.h"
#include "Frame.h"

namespace baresprite
{

OnionFrameRenderer::OnionFrameRenderer()
{
}

OnionFrameRenderer::~OnionFrameRenderer() = default;

/// <summary>
/// Render onion skinning
/// </summary>
/// <param name="frameIndex"></param>
/// <param name="opacity"></param>
/// <param name="frame"></param>
/// <param name="checkerSize"></param>
/// <param name="hdcMem"></param>
void OnionFrameRenderer::Render(int frameIndex, float opacity, const Frame &frame, int checkerSize, HDC hdcMem)
{
    if (!frame.pixels)
        return;

    int srcW = frame.width;
    int srcH = frame.height;
    int dstW = srcW * checkerSize;
    int dstH = srcH * checkerSize;

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = srcW;
    bmi.bmiHeader.biHeight = -srcH;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    HDC hdcSrc = CreateCompatibleDC(hdcMem);
    HBITMAP hBmp = CreateCompatibleBitmap(hdcMem, srcW, srcH);
    HGDIOBJ hOld = SelectObject(hdcSrc, hBmp);

    SetDIBitsToDevice(hdcSrc, 0, 0, srcW, srcH, 0, 0, 0, srcH, frame.pixels.get(), &bmi, DIB_RGB_COLORS);

    BLENDFUNCTION blend = {};
    blend.BlendOp = AC_SRC_OVER;
    blend.BlendFlags = 0;
    blend.SourceConstantAlpha = static_cast<BYTE>(opacity * 255.0f);
    blend.AlphaFormat = AC_SRC_ALPHA;

    AlphaBlend(hdcMem, 0, 0, dstW, dstH, hdcSrc, 0, 0, srcW, srcH, blend);

    SelectObject(hdcSrc, hOld);
    DeleteObject(hBmp);
    DeleteDC(hdcSrc);
}

} // namespace baresprite