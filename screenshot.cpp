#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include "json.hpp"

using json = nlohmann::json;

// 删除指定的文件
void DeleteFileIfExists(const std::string& filePath) {
    if (std::remove(filePath.c_str()) != 0) {
        std::cerr << "无法删除文件 " << filePath << "." << std::endl;
    }
}

// 使用系统命令行调用 PngOut 进行 PNG 转换
void ConvertBmpToPng(const std::string& bmpFilePath, const std::string& pngFilePath) {
    // 删除目标文件（如果存在）
    DeleteFileIfExists(pngFilePath);

    // 使用 PngOut 进行转换，并强制覆盖，设置最低压缩级别
    std::string command = "pngout \"" + bmpFilePath + "\" \"" + pngFilePath + "\" -force -s0";
    int result = system(command.c_str());

    if (result == 0) {
        std::cout << "截图已保存为 " << pngFilePath << "." << std::endl;
    }
    else {
        std::cerr << "保存 " << pngFilePath << " 失败。" << std::endl;
    }
}

// 保存 HBITMAP 为 BMP 文件
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
    bmpInfo.bmiHeader.biBitCount = bmp.bmBitsPixel;
    bmpInfo.bmiHeader.biCompression = BI_RGB;

    DWORD bmpSize = ((bmp.bmWidth * bmp.bmBitsPixel + 31) / 32) * 4 * bmp.bmHeight;
    BYTE* bmpData = new BYTE[bmpSize];

    if (GetDIBits(hdcMem, hBitmap, 0, bmp.bmHeight, bmpData, &bmpInfo, DIB_RGB_COLORS)) {
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
        }
        delete[] bmpData;
        DeleteDC(hdcMem);
        return true;
    }

    delete[] bmpData;
    DeleteDC(hdcMem);
    return false;
}

// 捕获指定窗口的内容
bool CaptureWindow(HWND hWnd, const std::string& filePath) {
    RECT rc;
    GetWindowRect(hWnd, &rc);

    HDC hdcWindow = GetDC(hWnd);  // 获取窗口的设备上下文
    HDC hdcMem = CreateCompatibleDC(hdcWindow);  // 创建与窗口设备上下文兼容的内存设备上下文
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcWindow, rc.right - rc.left, rc.bottom - rc.top);  // 创建兼容的位图
    HGDIOBJ oldBitmap = SelectObject(hdcMem, hBitmap);  // 选择位图对象到内存设备上下文中

    // 使用 PrintWindow 捕获窗口内容
    if (PrintWindow(hWnd, hdcMem, PW_CLIENTONLY)) {
        bool success = SaveBitmapToFile(hBitmap, filePath);  // 保存为 BMP 文件

        SelectObject(hdcMem, oldBitmap);  // 恢复之前的位图对象
        DeleteObject(hBitmap);  // 删除位图对象
        DeleteDC(hdcMem);  // 删除内存设备上下文
        ReleaseDC(hWnd, hdcWindow);  // 释放窗口的设备上下文

        return success;
    }

    // 如果捕获失败，清理资源
    SelectObject(hdcMem, oldBitmap);  // 恢复之前的位图对象
    DeleteObject(hBitmap);  // 删除位图对象
    DeleteDC(hdcMem);  // 删除内存设备上下文
    ReleaseDC(hWnd, hdcWindow);  // 释放窗口的设备上下文

    return false;
}

// 读取配置文件中的 homework 项
int ReadHomeworkConfig(const std::string& configFilePath) {
    std::ifstream configFile(configFilePath);
    if (!configFile.is_open()) {
        std::cerr << "无法打开配置文件。" << std::endl;
        return 0;
    }

    json config;
    configFile >> config;

    return config.value("homework", 0);
}

int main() {
    std::string configFilePath = "config.json";
    int homework = ReadHomeworkConfig(configFilePath);

    if (homework == 0) {
        std::cout << "不进行截图，配置中的 homework 值为 0。" << std::endl;
        return 0;
    }

    // 硬编码窗口标题
    std::wstring windowTitle = L"作业";
    HWND hWnd = FindWindow(NULL, windowTitle.c_str());

    if (hWnd) {
        std::cout << "找到窗口，开始截图。" << std::endl;
        if (CaptureWindow(hWnd, "screenshot.bmp")) {
            std::cout << "截图已保存为 screenshot.bmp。" << std::endl;
            ConvertBmpToPng("screenshot.bmp", "screenshot.png");
        }
        else {
            std::cout << "保存 screenshot.bmp 失败。" << std::endl;
        }
    }
    else {
        std::cout << "未找到指定的窗口。" << std::endl;
    }
    return 0;
}
