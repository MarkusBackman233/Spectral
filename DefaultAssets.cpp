#include "DefaultAssets.h"
#include <vector>
#include <string>
#include "Texture.h"
#include "TextureManager.h"
#include <comdef.h>

#include "RenderManager.h"
#include "resource.h"
#include "Vector2.h"
#include "iRender.h"

void DefaultAssets::LoadDefaults()
{
    LoadBitMap("TemplateGrid_albedo.bmp", IDB_BITMAP1);
    LoadBitMap("TemplateGrid_normal.bmp", IDB_BITMAP2);
}

void DefaultAssets::LoadBitMap(const std::string& filename, int resourceId)
{
    auto instance = RenderManager::GetInstance()->m_hInstance;
    HBITMAP hBMP = (HBITMAP)LoadBitmapW(instance, MAKEINTRESOURCEW(resourceId));
    //HRSRC hResInfo = FindResource(instance, L"IDB_PNG1", L"RT_BITMAP");
    //
    BITMAP bm;

    GetObject(hBMP, sizeof(BITMAP), &bm);


    BITMAPINFOHEADER bi;
    ZeroMemory(&bi, sizeof(BITMAPINFOHEADER));
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = bm.bmWidth;
    bi.biHeight = bm.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 32; // Assuming 32-bit bitmap
    bi.biCompression = BI_RGB;

    size_t bitmapSize = bm.bmWidth * bm.bmHeight * 4; // 4 bytes per pixel
    std::vector<BYTE> pixels(bitmapSize);

    HDC hdc = GetDC(NULL);
    HDC hMemDC = CreateCompatibleDC(hdc);
    SelectObject(hMemDC, hBMP);
    GetDIBits(hMemDC, hBMP, 0, bm.bmHeight, pixels.data(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);
    DeleteDC(hMemDC);
    ReleaseDC(NULL, hdc);

    for (size_t i = 0; i < bitmapSize ; i += 4)
    {
        auto temp = pixels[i];
        pixels[i] = pixels[i+2];
        pixels[i+2] = temp;

        pixels[i + 3] = 255;
    }

    auto defaultAlbedo = std::make_shared<Texture>();
    defaultAlbedo->LoadTexture(pixels.data(), Math::Vector2i(bm.bmWidth, bm.bmHeight));
    TextureManager::GetInstance()->AddTexture(filename, defaultAlbedo);
}
