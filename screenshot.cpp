#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include "json.hpp"
#include "screenshot.hpp" // 包含头文件

using json = nlohmann::json;

// 删除文件
void DeleteFileIfExists(const std::string& filePath) {
    if (std::remove(filePath.c_str()) != 0) {
        std::cerr << "无法删除文件 " << filePath << "." << std::endl;
    }
}

// 保存 HBITMAP 为 BMP
bool SaveBitmapToFile(HBITMAP hBitmap, const std::string& filePath) {
    BITMAP bmp;
    BITMAPINFO bmpInfo;
    memset(&bmpInfo, 0, sizeof(BITMAPINFO));
    bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);

    HDC hdcMem = CreateCompatibleDC(NULL);
    GetObject(hBitmap, sizeof(BITMAP), &bmp);
    bmpInfo.bmiHeader.biWidth = bmp.bmWidth;
    bmpInfo.bmiHeader.biHeight = bmp.bmHeight;
    bmpInfo.bmiHeader.biPlanes = 1;
    bmpInfo.bmiHeader.biBitCount = 32;
    bmpInfo.bmiHeader.biCompression = BI_RGB;

    DWORD bmpSize = ((bmpInfo.bmiHeader.biWidth * bmpInfo.bmiHeader.biBitCount + 31) / 32) * 4 * abs(bmpInfo.bmiHeader.biHeight);
    BYTE* bmpData = new BYTE[bmpSize];

    bool success = false;
    if (GetDIBits(hdcMem, hBitmap, 0, abs(bmpInfo.bmiHeader.biHeight), bmpData, &bmpInfo, DIB_RGB_COLORS)) {
        std::ofstream file(filePath, std::ios::binary);
        if (file) {
            BITMAPFILEHEADER bfh;
            bfh.bfType = 0x4D42; // 'BM'
            bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
            bfh.bfSize = bfh.bfOffBits + bmpSize;
            bfh.bfReserved1 = 0;
            bfh.bfReserved2 = 0;

            file.write(reinterpret_cast<char*>(&bfh), sizeof(BITMAPFILEHEADER));
            file.write(reinterpret_cast<char*>(&bmpInfo.bmiHeader), sizeof(BITMAPINFOHEADER));
            file.write(reinterpret_cast<char*>(bmpData), bmpSize);
            file.close();
            success = true;
        }
    }

    delete[] bmpData;
    DeleteDC(hdcMem);
    return success;
}

// 捕获窗口的内容
bool CaptureWindow(HWND hWnd, const std::string& filePath) {
    RECT rc;
    GetWindowRect(hWnd, &rc);

    HDC hdcWindow = GetDC(hWnd);
    HDC hdcMem = CreateCompatibleDC(hdcWindow);
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcWindow, rc.right - rc.left, rc.bottom - rc.top);
    HGDIOBJ oldBitmap = SelectObject(hdcMem, hBitmap);

    bool success = false;
    if (BitBlt(hdcMem, 0, 0, rc.right - rc.left, rc.bottom - rc.top, hdcWindow, 0, 0, SRCCOPY)) {
        success = SaveBitmapToFile(hBitmap, filePath);
    }

    SelectObject(hdcMem, oldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hdcMem);
    ReleaseDC(hWnd, hdcWindow);

    return success;
}

// 读取配置文件中的 homework 项
bool ReadHomeworkConfig(const std::string& configFilePath) {
    std::ifstream configFile(configFilePath);
    if (!configFile.is_open()) {
        std::cerr << "无法打开配置文件。" << std::endl;
        return false;
    }

    json config;
    configFile >> config;

    return config.value("homework", false);
}

// 修改后的函数签名，替换 main
void CaptureScreenshot(bool shouldCapture) {
    if (!shouldCapture) {
        std::cout << "不进行截图，配置中的 homework 值为 false。" << std::endl;
        return;
    }

    std::wstring windowTitle = L"作业";
    HWND hWnd = FindWindow(NULL, windowTitle.c_str());

    if (hWnd) {
        std::cout << "找到窗口，开始截图。" << std::endl;
        if (CaptureWindow(hWnd, "screenshot.bmp")) {
            std::cout << "截图已保存为 screenshot.bmp。" << std::endl;

            // 创建一个文件来指示截图完成
            std::ofstream doneFile("screenshot_done.txt");
            doneFile << "done";
            doneFile.close();
        }
        else {
            std::cout << "保存 screenshot.bmp 失败。" << std::endl;
        }
    }
    else {
        std::cout << "未找到指定的窗口。" << std::endl;
    }
}
