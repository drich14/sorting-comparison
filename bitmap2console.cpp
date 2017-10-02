#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <fstream>
#include "bitmap2console.h"
#include <conio.h>
#include <math.h>
using namespace std;

HANDLE hConsoleb2c;

CPIC::CPIC(int w, int h) {
    hConsoleb2c = GetStdHandle(STD_OUTPUT_HANDLE);
    colors = new unsigned char[(w * h) + 1];
    width = w;
    height = h;
}

void CPIC::save(ofstream & of) {
    of.write((char * ) colors, width * height * sizeof(unsigned char));
    of.write((char * ) & width, sizeof(int));
    of.write((char * ) & height, sizeof(int));
}

void CPIC::load(ifstream & inf) {
    inf.read((char * ) colors, width * height * sizeof(unsigned char));
    inf.read((char * ) & width, sizeof(int));
    inf.read((char * ) & height, sizeof(int));
}

void CPIC::setColors(unsigned char * c) {
    memcpy(colors, c, width * height);
}

void CPIC::display(void) {
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            SetConsoleTextAttribute(hConsoleb2c, colors[width * y + x]);
            cout << " ";
            SetConsoleTextAttribute(hConsoleb2c, 7);
        }
        cout << endl;
    }
}

CPIC * bitmap2console(char * path) {
    HBITMAP hBmp = (HBITMAP) LoadImage(GetModuleHandle(NULL), _T(path), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    if (!hBmp) { // failed to load bitmap
        cout << "Failed to load bmp" << endl;
        return NULL;
    }

    //getting the size of the picture
    BITMAP bm;
    GetObject(hBmp, sizeof(bm), & bm);
    int width(bm.bmWidth),
        height(bm.bmHeight);

    //creating a bitmapheader for getting the dibits
    BITMAPINFOHEADER bminfoheader;::ZeroMemory( & bminfoheader, sizeof(BITMAPINFOHEADER));
    bminfoheader.biSize = sizeof(BITMAPINFOHEADER);
    bminfoheader.biWidth = width;
    bminfoheader.biHeight = -height;
    bminfoheader.biPlanes = 1;
    bminfoheader.biBitCount = 32;
    bminfoheader.biCompression = BI_RGB;

    bminfoheader.biSizeImage = width * 4 * height;
    bminfoheader.biClrUsed = 0;
    bminfoheader.biClrImportant = 0;

    //create a buffer and let the GetDIBits fill in the buffer
    unsigned char * pPixels = new unsigned char[(width * 4 * height)];
    if (!GetDIBits(CreateCompatibleDC(0), hBmp, 0, height, pPixels, (BITMAPINFO * ) & bminfoheader, DIB_RGB_COLORS)) // load pixel info
    {
        //return if fails but first delete the resources
        DeleteObject(hBmp);
        delete[] pPixels; // delete the array of objects

        cout << "Failed to GetDiBits" << endl;
        return NULL;
    }
    unsigned char * colors = new unsigned char[width * height + 1];
    int rgb[16][3] = {
        {
            0,
            0,
            0
        },
        {
            0,
            0,
            128
        },
        {
            0,
            128,
            0
        },
        {
            0,
            128,
            128
        },
        {
            128,
            0,
            0
        },
        {
            128,
            0,
            128
        },
        {
            128,
            128,
            0
        },
        {
            192,
            192,
            192
        },
        {
            128,
            128,
            128
        },
        {
            0,
            0,
            255
        },
        {
            0,
            255,
            0
        },
        {
            0,
            255,
            255
        },
        {
            255,
            0,
            0
        },
        {
            255,
            0,
            255
        },
        {
            255,
            255,
            0
        },
        {
            255,
            255,
            255
        }
    };
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int diff = 255 * 255 * 3;
            int low = diff;
            colors[width * y + x] = 0;
            for (int i = 0; i < 16; i++) {
                diff = pow(rgb[i][0] - pPixels[(width * y + x) * 4 + 2], 2)
					+ pow(rgb[i][1] - pPixels[(width * y + x) * 4 + 1], 2)
					+ pow(rgb[i][2] - pPixels[(width * y + x) * 4 + 0], 2);
                if (diff < low) {
                    colors[width * y + x] = 16 * i;
                    low = diff;
                }
            }
        }
    }
    CPIC * cpic = new CPIC(width, height);
    cpic - > setColors(colors);
    DeleteObject(hBmp);
    delete[] pPixels;
    delete[] colors;
    return cpic;
}

void set_window_size(int lines, int columns) {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(
            GetStdHandle(STD_OUTPUT_HANDLE), & csbi
        )) {
        // Make sure the new size isn't too big
        if (lines > csbi.dwSize.Y) lines = csbi.dwSize.Y;
        if (columns > csbi.dwSize.X) columns = csbi.dwSize.X;

        // Adjust window origin if necessary
        if ((csbi.srWindow.Top + lines) > csbi.dwSize.Y) csbi.srWindow.Top = csbi.dwSize.Y - lines - 1;
        if ((csbi.srWindow.Left + columns) > csbi.dwSize.Y) csbi.srWindow.Left = csbi.dwSize.X - columns - 1;

        // Calculate new size
        csbi.srWindow.Bottom = csbi.srWindow.Top + lines - 1;
        csbi.srWindow.Right = csbi.srWindow.Left + columns - 1;

        SetConsoleWindowInfo(
            GetStdHandle(STD_OUTPUT_HANDLE),
            true, & csbi.srWindow
        );
    }
}
