// Project1.cpp : 애플리케이션에 대한 진입점을 정의합니다.
//

#include "framework.h"
#include "Project1.h"
#include <d3d9.h>
#include<d3dx9.h>

#define MAX_LOADSTRING 100
#define Deg2Rad 0.017453293f

HRESULT InitD3D(HWND);
HRESULT InitVertexBuffer();
HRESULT InitIndexBuffer();
D3DXMATRIXA16 g_matEarth, g_matMoon, g_matSun;

void CleanUp();
void Render();
void SetupMatrices();
void Update();
void DrawMesh(const D3DXMATRIXA16& matrix);

// 전역 변수:
HINSTANCE hInst;                                // 현재 인스턴스입니다.
WCHAR szTitle[MAX_LOADSTRING];                  // 제목 표시줄 텍스트입니다.
WCHAR szWindowClass[MAX_LOADSTRING];            // 기본 창 클래스 이름입니다.
LPDIRECT3D9 g_pD3D = NULL;
LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;
LPDIRECT3DVERTEXBUFFER9 g_pVB = NULL;
LPDIRECT3DINDEXBUFFER9 g_pIB = NULL;

struct CUSTOMVERTEX
{
    float x, y, z;
    DWORD color;
};
#define D3DFVF_CUSTOM (D3DFVF_XYZ | D3DFVF_DIFFUSE)

struct INDEX
{
    WORD _0, _1, _2;
};


// 이 코드 모듈에 포함된 함수의 선언을 전달합니다:
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 여기에 코드를 입력합니다.

    // 전역 문자열을 초기화합니다.
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_PROJECT1, szWindowClass, MAX_LOADSTRING);

    WNDCLASSEXW wcex = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, hInstance, 0, 0,0, NULL, szWindowClass, NULL };
    RegisterClassExW(&wcex);

    // 애플리케이션 초기화를 수행합니다:
    HWND hWnd = CreateWindowW(szWindowClass, L"D3D", WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, CW_USEDEFAULT, 0, 
        CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
   
    if (!hWnd)return FALSE;

    if (SUCCEEDED(InitD3D(hWnd)))
    {
        if (SUCCEEDED(InitVertexBuffer()))
        {
            if (SUCCEEDED(InitIndexBuffer()))
            {
                ShowWindow(hWnd, nCmdShow);
                UpdateWindow(hWnd);

                MSG msg;
                ZeroMemory(&msg, sizeof(msg));
                ULONGLONG frameTIme, limitFrameTime = GetTickCount64();

                while (WM_QUIT != msg.message)
                {
                    if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                    else
                    {
                        Update();
                        Render();
                    }
                }
            }
        }
    }

    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        CleanUp();
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

HRESULT InitD3D(HWND hWnd)
{
    if (NULL == (g_pD3D = Direct3DCreate9(D3D_SDK_VERSION))) return E_FAIL;

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));

    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

    DWORD level;
    for (auto type = (int)D3DMULTISAMPLE_16_SAMPLES; 0 < type; type--)
    {
        if (SUCCEEDED(g_pD3D->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, D3DFMT_D16, FALSE, (D3DMULTISAMPLE_TYPE)type, &level)))
        {
            d3dpp.MultiSampleQuality = level - 1;
            d3dpp.MultiSampleType = (D3DMULTISAMPLE_TYPE)type;
            break;
        }
    }

    if (FAILED(g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pd3dDevice)))
    {
        return E_FAIL;
    }

    g_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
    g_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
    g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
    g_pd3dDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, (0 < d3dpp.MultiSampleType));
    return S_OK;
}

void CleanUp()
{
    if (NULL != g_pVB) g_pVB->Release();
    if (NULL != g_pIB) g_pIB->Release();
    if (NULL != g_pd3dDevice) g_pd3dDevice->Release();
    if (NULL != g_pD3D) g_pD3D->Release();
}

void Render()
{
    if (SUCCEEDED(g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 255), 1, 0)))
    {
        SetupMatrices();
        if (SUCCEEDED(g_pd3dDevice->BeginScene()))
        {
            DrawMesh(g_matSun);
            DrawMesh(g_matEarth);
            DrawMesh(g_matMoon);
            
            //g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
            //g_pd3dDevice->SetFVF(D3DFVF_CUSTOM);
            //// 인덱스 버퍼를 지정.
            //g_pd3dDevice->SetIndices(g_pIB);
            //// 정점 버퍼 8개로 12개의 폴리곤을 그린다.
            //g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12);
            g_pd3dDevice->EndScene();
        }
        g_pd3dDevice->Present(NULL, NULL, NULL, NULL);
    }
}

HRESULT InitVertexBuffer()
{
    CUSTOMVERTEX vertices[] =
    {
        { -1, 1, 1, 0xffff0000 }, // v0
        { 1, 1, 1, 0xff00ff00 }, // v1
        { 1, 1, -1, 0xff0000ff }, // v2
        { -1, 1, -1, 0xffffff00 }, // v3
        { -1, -1, 1, 0xff00ffff }, // v4
        { 1, -1, 1, 0xffff00ff }, // v5
        { 1, -1, -1, 0xff000000 }, // v6
        { -1, -1, -1, 0xffffffff }, // v7
    };

    if (FAILED(g_pd3dDevice->CreateVertexBuffer(sizeof(vertices), 0, D3DFVF_CUSTOM, D3DPOOL_DEFAULT, &g_pVB, NULL)))
        return E_FAIL;
    
    LPVOID pVertices;
    if (FAILED(g_pVB->Lock(0, sizeof(vertices), (void**)&pVertices, 0))) 
        return E_FAIL;
    memcpy(pVertices, vertices, sizeof(vertices));
    g_pVB->Unlock();
    return S_OK;
}

HRESULT InitIndexBuffer()
{
    INDEX indices[] =
    {
        { 0, 1, 2 }, { 0, 2, 3 }, // 윗면.
        { 4, 6, 5 }, { 4, 7, 6 }, // 아랫면.
        { 0, 3, 7 }, { 0, 7, 4 }, // 왼쪽면.
        { 1, 5, 6 }, { 1, 6, 2 }, // 오른쪽면.
        { 3, 2, 6 }, { 3, 6, 7 }, // 앞면.
        { 0, 4, 5 }, { 0, 5, 1 }, // 뒷면.
    };
   
    if (FAILED(g_pd3dDevice->CreateIndexBuffer(sizeof(indices), 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &g_pIB, NULL))) return E_FAIL;
    LPVOID pIndices;
    if (FAILED(g_pIB->Lock(0, sizeof(indices), (void**)&pIndices, 0))) return E_FAIL;
    memcpy(pIndices, indices, sizeof(indices));
    g_pIB->Unlock();
    return S_OK;
}

void SetupMatrices()
{
    //D3DXMATRIXA16 matWorld;
    //D3DXMatrixIdentity(&matWorld);
    //g_pd3dDevice->SetTransform(D3DTS_WORLD, &matWorld);

    D3DXVECTOR3 vEyePt(0.0f, 10.0f, -15.0f); //월드 좌표의 카메라 위치
    D3DXVECTOR3 vLookAtPt(0.0f, 0.0f, 0.0f); //월드 좌표의 카메라가 바라보는 위치
    D3DXVECTOR3 vUpVector(0.0f, 0.1f, 0.0f); //월드 좌표의 하늘 방향을 알기 위한 upVector

    D3DXMATRIXA16 matView;
    D3DXMatrixLookAtLH(&matView, &vEyePt, &vLookAtPt, &vUpVector);
    g_pd3dDevice->SetTransform(D3DTS_VIEW, &matView);

    D3DXMATRIXA16 matProj;
    D3DXMatrixPerspectiveFovLH(&matProj, 45 * Deg2Rad, 1.77f, 1.0f, 100.0f);
    g_pd3dDevice->SetTransform(D3DTS_PROJECTION, &matProj);
}

void Update()
{
    auto angle = GetTickCount64() * 0.001f;
    D3DXMATRIXA16 matSunTr; // , matSunRo;
    D3DXMATRIXA16 matEarthTr, matEarthRo, matEarthSc;
    D3DXMATRIXA16 matMoonTr, matMoonRo, matMoonSc;
    D3DXMATRIXA16 matPointTr, matPointRo;

    D3DXMatrixIdentity(&matSunTr);   
    g_matSun = matSunTr; 

    D3DXMatrixTranslation(&matEarthTr, 8, 0, 0);
    D3DXMatrixRotationY(&matEarthRo, angle);
    D3DXMatrixScaling(&matEarthSc, 0.5f, 0.5f, 0.5f);
    g_matEarth = matEarthSc * matEarthRo * matEarthTr * matEarthRo * g_matSun;

    D3DXMatrixRotationY(&matMoonRo, angle * 0.5f);
    D3DXMatrixScaling(&matMoonSc, 0.2f, 0.2f, 0.2f);
    D3DXMatrixTranslation(&matMoonTr, 5, 0, 0);
    g_matMoon = matMoonSc * matMoonTr * matMoonRo * g_matEarth;
}

void DrawMesh(const D3DXMATRIXA16& matrix)
{
    g_pd3dDevice->SetTransform(D3DTS_WORLD, &matrix);
    g_pd3dDevice->SetStreamSource(0, g_pVB, 0, sizeof(CUSTOMVERTEX));
    g_pd3dDevice->SetFVF(D3DFVF_CUSTOM);
    g_pd3dDevice->SetIndices(g_pIB);
    g_pd3dDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, 8, 0, 12);
}