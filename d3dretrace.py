##########################################################################
#
# Copyright 2011 Jose Fonseca
# All Rights Reserved.
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
#
##########################################################################/


"""GL retracer generator."""


import specs.stdapi as stdapi
from specs.d3d9 import d3d9
from retrace import Retracer


class D3DRetracer(Retracer):

    table_name = 'd3dretrace::d3d9_callbacks'

    def invokeInterfaceMethod(self, interface, method):
        if interface.name == 'IDirect3D9' and method.name == 'CreateDevice':
            print 'HWND hWnd = createWindow(pPresentationParameters->BackBufferWidth, pPresentationParameters->BackBufferHeight);'
            print 'pPresentationParameters->hDeviceWindow = hWnd;'
            print 'hFocusWindow = hWnd;'

        Retracer.invokeInterfaceMethod(self, interface, method)

        if str(method.type) == 'HRESULT':
            print r'    if (__result != S_OK) {'
            print r'        retrace::warning(call) << "failed\n";'
            print r'    }'

        if interface.name == 'IDirect3DVertexBuffer9' and method.name == 'Lock':
            print '        if (!SizeToLock) {'
            print '            D3DVERTEXBUFFER_DESC Desc;'
            print '            _this->GetDesc(&Desc);'
            print '            SizeToLock = Desc.Size;'
            print '        }'


if __name__ == '__main__':
    print r'''
#include <string.h>

#include <iostream>

#include "d3d9imports.hpp"
#include "d3dretrace.hpp"


// XXX: Don't duplicate this code.

static LRESULT CALLBACK
WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    MINMAXINFO *pMMI;
    switch (uMsg) {
    case WM_GETMINMAXINFO:
        // Allow to create a window bigger than the desktop
        pMMI = (MINMAXINFO *)lParam;
        pMMI->ptMaxSize.x = 60000;
        pMMI->ptMaxSize.y = 60000;
        pMMI->ptMaxTrackSize.x = 60000;
        pMMI->ptMaxTrackSize.y = 60000;
        break;
    default:
        break;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


static HWND
createWindow(int width, int height) {
    static bool first = TRUE;
    RECT rect;

    if (first) {
        WNDCLASS wc;
        memset(&wc, 0, sizeof wc);
        wc.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
        wc.lpfnWndProc = WndProc;
        wc.lpszClassName = "d3dretrace";
        wc.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
        RegisterClass(&wc);
        first = FALSE;
    }

    DWORD dwExStyle;
    DWORD dwStyle;
    HWND hWnd;

    dwExStyle = 0;
    dwStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_OVERLAPPEDWINDOW;

    int x = 0, y = 0;

    rect.left = x;
    rect.top = y;
    rect.right = rect.left + width;
    rect.bottom = rect.top + height;

    AdjustWindowRectEx(&rect, dwStyle, FALSE, dwExStyle);

    hWnd = CreateWindowEx(dwExStyle,
                          "d3dretrace", /* wc.lpszClassName */
                          NULL,
                          dwStyle,
                          0, /* x */
                          0, /* y */
                          rect.right - rect.left, /* width */
                          rect.bottom - rect.top, /* height */
                          NULL,
                          NULL,
                          NULL,
                          NULL);
    ShowWindow(hWnd, SW_SHOW);
    return hWnd;
}

'''
    retracer = D3DRetracer()
    retracer.retraceApi(d3d9)
