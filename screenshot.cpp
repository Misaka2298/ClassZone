#include <windows.h>
#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include "json.hpp"

using json = nlohmann::json;

// ɾ��ָ�����ļ�
void DeleteFileIfExists(const std::string& filePath) {
    if (std::remove(filePath.c_str()) != 0) {
        std::cerr << "�޷�ɾ���ļ� " << filePath << "." << std::endl;
    }
}

// ʹ��ϵͳ�����е��� PngOut ���� PNG ת��
void ConvertBmpToPng(const std::string& bmpFilePath, const std::string& pngFilePath) {
    // ɾ��Ŀ���ļ���������ڣ�
    DeleteFileIfExists(pngFilePath);

    // ʹ�� PngOut ����ת������ǿ�Ƹ��ǣ��������ѹ������
    std::string command = "pngout \"" + bmpFilePath + "\" \"" + pngFilePath + "\" -force -s0";
    int result = system(command.c_str());

    if (result == 0) {
        std::cout << "��ͼ�ѱ���Ϊ " << pngFilePath << "." << std::endl;
    }
    else {
        std::cerr << "���� " << pngFilePath << " ʧ�ܡ�" << std::endl;
    }
}

// ���� HBITMAP Ϊ BMP �ļ�
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

// ����ָ�����ڵ�����
bool CaptureWindow(HWND hWnd, const std::string& filePath) {
    RECT rc;
    GetWindowRect(hWnd, &rc);

    HDC hdcWindow = GetDC(hWnd);  // ��ȡ���ڵ��豸������
    HDC hdcMem = CreateCompatibleDC(hdcWindow);  // �����봰���豸�����ļ��ݵ��ڴ��豸������
    HBITMAP hBitmap = CreateCompatibleBitmap(hdcWindow, rc.right - rc.left, rc.bottom - rc.top);  // �������ݵ�λͼ
    HGDIOBJ oldBitmap = SelectObject(hdcMem, hBitmap);  // ѡ��λͼ�����ڴ��豸��������

    // ʹ�� PrintWindow ���񴰿�����
    if (PrintWindow(hWnd, hdcMem, PW_CLIENTONLY)) {
        bool success = SaveBitmapToFile(hBitmap, filePath);  // ����Ϊ BMP �ļ�

        SelectObject(hdcMem, oldBitmap);  // �ָ�֮ǰ��λͼ����
        DeleteObject(hBitmap);  // ɾ��λͼ����
        DeleteDC(hdcMem);  // ɾ���ڴ��豸������
        ReleaseDC(hWnd, hdcWindow);  // �ͷŴ��ڵ��豸������

        return success;
    }

    // �������ʧ�ܣ�������Դ
    SelectObject(hdcMem, oldBitmap);  // �ָ�֮ǰ��λͼ����
    DeleteObject(hBitmap);  // ɾ��λͼ����
    DeleteDC(hdcMem);  // ɾ���ڴ��豸������
    ReleaseDC(hWnd, hdcWindow);  // �ͷŴ��ڵ��豸������

    return false;
}

// ��ȡ�����ļ��е� homework ��
int ReadHomeworkConfig(const std::string& configFilePath) {
    std::ifstream configFile(configFilePath);
    if (!configFile.is_open()) {
        std::cerr << "�޷��������ļ���" << std::endl;
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
        std::cout << "�����н�ͼ�������е� homework ֵΪ 0��" << std::endl;
        return 0;
    }

    // Ӳ���봰�ڱ���
    std::wstring windowTitle = L"��ҵ";
    HWND hWnd = FindWindow(NULL, windowTitle.c_str());

    if (hWnd) {
        std::cout << "�ҵ����ڣ���ʼ��ͼ��" << std::endl;
        if (CaptureWindow(hWnd, "screenshot.bmp")) {
            std::cout << "��ͼ�ѱ���Ϊ screenshot.bmp��" << std::endl;
            ConvertBmpToPng("screenshot.bmp", "screenshot.png");
        }
        else {
            std::cout << "���� screenshot.bmp ʧ�ܡ�" << std::endl;
        }
    }
    else {
        std::cout << "δ�ҵ�ָ���Ĵ��ڡ�" << std::endl;
    }
    return 0;
}
