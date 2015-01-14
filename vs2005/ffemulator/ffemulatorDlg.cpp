// ffemulatorDlg.cpp : implementation file
//
#include "stdafx.h"
#include "ffemulator.h"
#include "ffemulatorDlg.h"
#include "ffndbdebugDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 内部常量定义
#define SCREEN_WIDTH    GetSystemMetrics(SM_CXSCREEN)
#define SCREEN_HEIGHT   GetSystemMetrics(SM_CYSCREEN)

// CffemulatorDlg dialog
CffemulatorDlg::CffemulatorDlg(CWnd* pParent /*=NULL*/)
    : CDialog(CffemulatorDlg::IDD, pParent)
{
    m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CffemulatorDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialog::DoDataExchange(pDX);
}

BOOL CffemulatorDlg::PreTranslateMessage(MSG* pMsg)
{
    if (TranslateAccelerator(GetSafeHwnd(), m_hAcc, pMsg)) return TRUE;

    if (pMsg->message == WM_KEYDOWN)
    {
        switch (pMsg->wParam)
        {
        case 'E': nes_joypad(&m_nes, 0, NES_KEY_UP     , 1); return TRUE;
        case 'D': nes_joypad(&m_nes, 0, NES_KEY_DOWN   , 1); return TRUE;
        case 'S': nes_joypad(&m_nes, 0, NES_KEY_LEFT   , 1); return TRUE;
        case 'F': nes_joypad(&m_nes, 0, NES_KEY_RIGHT  , 1); return TRUE;
        case 'J': nes_joypad(&m_nes, 0, NES_KEY_A      , 1); return TRUE;
        case 'K': nes_joypad(&m_nes, 0, NES_KEY_B      , 1); return TRUE;
        case 'U': nes_joypad(&m_nes, 0, NES_KEY_TURBO_A, 1); return TRUE;
        case 'I': nes_joypad(&m_nes, 0, NES_KEY_TURBO_B, 1); return TRUE;
        case 'B': nes_joypad(&m_nes, 0, NES_KEY_SELECT , 1); return TRUE;
        case 'N': nes_joypad(&m_nes, 0, NES_KEY_START  , 1); return TRUE;
        }
    }

    if (pMsg->message == WM_KEYUP)
    {
        switch (pMsg->wParam)
        {
        case 'E': nes_joypad(&m_nes, 0, NES_KEY_UP     , 0); return TRUE;
        case 'D': nes_joypad(&m_nes, 0, NES_KEY_DOWN   , 0); return TRUE;
        case 'S': nes_joypad(&m_nes, 0, NES_KEY_LEFT   , 0); return TRUE;
        case 'F': nes_joypad(&m_nes, 0, NES_KEY_RIGHT  , 0); return TRUE;
        case 'J': nes_joypad(&m_nes, 0, NES_KEY_A      , 0); return TRUE;
        case 'K': nes_joypad(&m_nes, 0, NES_KEY_B      , 0); return TRUE;
        case 'U': nes_joypad(&m_nes, 0, NES_KEY_TURBO_A, 0); return TRUE;
        case 'I': nes_joypad(&m_nes, 0, NES_KEY_TURBO_B, 0); return TRUE;
        case 'B': nes_joypad(&m_nes, 0, NES_KEY_SELECT , 0); return TRUE;
        case 'N': nes_joypad(&m_nes, 0, NES_KEY_START  , 0); return TRUE;
        }
    }

    return CDialog::PreTranslateMessage(pMsg);
}

BEGIN_MESSAGE_MAP(CffemulatorDlg, CDialog)
    ON_WM_DESTROY()
    ON_WM_CTLCOLOR()
    ON_WM_ACTIVATE()
    ON_COMMAND(ID_FILE_OPEN_ROM, &CffemulatorDlg::OnOpenRom)
    ON_COMMAND(ID_EXIT, &CffemulatorDlg::OnExit)
    ON_COMMAND(ID_CONTROL_RESET, &CffemulatorDlg::OnControlReset)
    ON_COMMAND(ID_CONTROL_PAUSEPLAY, &CffemulatorDlg::OnControlPauseplay)
    ON_COMMAND(ID_TOOLS_FFNDB, &CffemulatorDlg::OnToolsFfndb)
    ON_COMMAND(ID_HELP_ABOUT, &CffemulatorDlg::OnHelpAbout)
    ON_COMMAND(ID_FILE_SAVE_GAME, &CffemulatorDlg::OnFileSaveGame)
    ON_COMMAND(ID_FILE_SAVE_GAME_AS, &CffemulatorDlg::OnFileSaveGameAs)
    ON_COMMAND(ID_FILE_LOAD_GAME, &CffemulatorDlg::OnFileLoadGame)
END_MESSAGE_MAP()

// CffemulatorDlg message handlers
BOOL CffemulatorDlg::OnInitDialog()
{
    CDialog::OnInitDialog();

    // Set the icon for this dialog. The framework does this automatically
    // when the application's main window is not a dialog
    SetIcon(m_hIcon, TRUE);      // Set big icon
    SetIcon(m_hIcon, FALSE);     // Set small icon

    // load accelerators
    m_hAcc = LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_FFEMU_ACC)); 

    RECT rect = {0};
    int  x, y, w, h;
    MoveWindow(0, 0, NES_WIDTH, NES_HEIGHT, FALSE);
    GetClientRect(&rect);
    w = NES_WIDTH  + (NES_WIDTH  - rect.right );
    h = NES_HEIGHT + (NES_HEIGHT - rect.bottom);
    x = (SCREEN_WIDTH  - w) / 2;
    y = (SCREEN_HEIGHT - h) / 2;
    x = x > 0 ? x : 0;
    y = y > 0 ? y : 0;
    MoveWindow(x, y, w, h, FALSE);

    // init nes
    nes_init(&m_nes, "", (DWORD)GetSafeHwnd());

    return TRUE;  // return TRUE  unless you set the focus to a control
}

void CffemulatorDlg::OnDestroy()
{
    CDialog::OnDestroy();

    // destroy ffndb dialog
    CDialog *dlg = (CDialog*)FindWindow(NULL, "ffndb");
    if (dlg) dlg->DestroyWindow();

    // free nes
    nes_free(&m_nes);
}

void CffemulatorDlg::LoadNesRom()
{
    // pause nes thread if running
    int running = nes_getrun(&m_nes);
    if (running) nes_setrun(&m_nes, 0);

    CFileDialog dlg(TRUE, NULL, NULL, 0, "nes rom file (*.nes)|*.nes||");
    if (dlg.DoModal() == IDOK)
    {
        char file[MAX_PATH] = {0};
        SetWindowText(CString("ffemulator - ") + dlg.GetFileName());
        strncpy(file, dlg.GetPathName(), MAX_PATH);

        // free & clear nes context first
        nes_free(&m_nes);
        memset(&m_nes, 0, sizeof(NES));

        // init nes & run
        nes_init  (&m_nes, file, (DWORD)GetSafeHwnd());
        nes_setrun(&m_nes, 1);
    }

    // resume running
    if (running) nes_setrun(&m_nes, 1);
}

HBRUSH CffemulatorDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
    HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
    // TODO:  Change any attributes of the DC here
    // TODO:  Return a different brush if the default is not desired
    if (pWnd == this) return (HBRUSH)GetStockObject(BLACK_BRUSH);
    else return hbr;
}

void CffemulatorDlg::OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized)
{
    CDialog::OnActivate(nState, pWndOther, bMinimized);

    // TODO: Add your message handler code here
    switch (nState)
    {
    case WA_ACTIVE:
    case WA_CLICKACTIVE:
        SetFocus();
        break;
    }
}

void CffemulatorDlg::OnOpenRom()
{
    //++ if ffndb dialog is opend, close it first ++//
    CDialog *dlg = (CDialog*)FindWindow(NULL, "ffndb");
    if (dlg) dlg->PostMessage(WM_CLOSE);
    //-- if ffndb dialog is opend, close it first --//

    LoadNesRom();
}

void CffemulatorDlg::OnExit()
{
    OnOK();
}

void CffemulatorDlg::OnControlReset()
{
    nes_setrun(&m_nes, 1);
    nes_reset (&m_nes);
    nes_textout(&m_nes, 0, 222, "reset", 2000);
}

void CffemulatorDlg::OnControlPauseplay()
{
    if (nes_getrun(&m_nes))
    {
        nes_textout(&m_nes, 0, 222, "paused", -1); Sleep(16);
        nes_setrun (&m_nes, 0);
    }
    else
    {
        nes_setrun (&m_nes, 1);
        nes_textout(&m_nes, 0, 222, "running", 2000);
    }
}

void CffemulatorDlg::OnToolsFfndb()
{
    CDialog *dlg = (CDialog*)FindWindow(NULL, "ffndb");
    if (!dlg) {
        RECT rect = {0};
        GetWindowRect(&rect);
        MoveWindow(0, 0, rect.right - rect.left, rect.bottom - rect.top);

        dlg = new CffndbdebugDlg(this, &m_nes);
        dlg->Create(IDD_FFNDBDEBUG_DIALOG, GetDesktopWindow());
        dlg->CenterWindow();
        dlg->ShowWindow(SW_SHOW);
    }
    else SwitchToThisWindow(dlg->GetSafeHwnd(), TRUE);
}

void CffemulatorDlg::OnHelpAbout()
{
    CDialog dlg(IDD_DIALOG_ABOUT);
    dlg.DoModal();
}

void CffemulatorDlg::OnFileSaveGame()
{
    if (m_strGameSaveFile.Compare("") == 0)
    {
        // pause nes thread if running
        int running = nes_getrun(&m_nes);
        if (running) nes_setrun(&m_nes, 0);

        CFileDialog dlg(FALSE, ".sav", NULL, 0, "nes save file (*.sav)|*.sav||");
        if (dlg.DoModal() == IDOK) m_strGameSaveFile = dlg.GetPathName();

        // resume running
        if (running) nes_setrun(&m_nes, 1);
    }

    char file[MAX_PATH] = {0};
    strncpy(file, m_strGameSaveFile, MAX_PATH);
    nes_save_game(&m_nes, file);
}

void CffemulatorDlg::OnFileSaveGameAs()
{
    // pause nes thread if running
    int running = nes_getrun(&m_nes);
    if (running) nes_setrun(&m_nes, 0);

    CFileDialog dlg(FALSE, ".sav", NULL, 0, "nes save file (*.sav)|*.sav||");
    if (dlg.DoModal() == IDOK)
    {
        char file[MAX_PATH] = {0};
        m_strGameSaveFile = dlg.GetPathName();
        strncpy(file, m_strGameSaveFile, MAX_PATH);
        nes_save_game(&m_nes, file);
    }

    // resume running
    if (running) nes_setrun(&m_nes, 1);
}

void CffemulatorDlg::OnFileLoadGame()
{
    // pause nes thread if running
    int running = nes_getrun(&m_nes);
    if (running) nes_setrun(&m_nes, 0);

    CFileDialog dlg(TRUE, ".sav", NULL, 0, "nes save file (*.sav)|*.sav||");
    if (dlg.DoModal() == IDOK)
    {
        char file[MAX_PATH] = {0};
        strncpy(file, dlg.GetPathName(), MAX_PATH);
        nes_load_game(&m_nes, file); // load
    }

    // resume running
    if (running) nes_setrun(&m_nes, 1);
}


