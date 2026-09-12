// BubbleBattle.cpp: 定义应用程序的类行为。
//

#include "pch.h"
#include "framework.h"
#include "afxwinappex.h"
#include "afxdialogex.h"
#include "BubbleBattle.h"
#include "MainFrm.h"

#include "BubbleBattleDoc.h"
#include "BubbleBattleView.h"
#include "BubbleGameUIView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CBubbleBattleApp

BEGIN_MESSAGE_MAP(CBubbleBattleApp, CWinApp)
    ON_COMMAND(ID_APP_ABOUT, &CBubbleBattleApp::OnAppAbout)
    // 基于文件的标准文档命令
    ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
    ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
END_MESSAGE_MAP()

// CBubbleBattleApp 构造

CBubbleBattleApp::CBubbleBattleApp() noexcept
{
    // TODO: 将以下应用程序 ID 字符串替换为唯一的 ID 字符串；
    // 建议的字符串格式为 CompanyName.ProductName.SubProduct.VersionInformation。
    SetAppID(_T("BubbleBattle.AppID.NoVersion"));

    // 将所有重要的初始化放置在 InitInstance 中。
}

// 唯一的 CBubbleBattleApp 对象

CBubbleBattleApp theApp;

// CBubbleBattleApp 初始化

BOOL CBubbleBattleApp::InitInstance()
{
    // 如果清单指定使用 ComCtl32.dll 版本 6 或更高版本，
    // 需要先初始化公共控件。
    INITCOMMONCONTROLSEX InitCtrls;
    InitCtrls.dwSize = sizeof(InitCtrls);
    InitCtrls.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&InitCtrls);

    CWinApp::InitInstance();

    EnableTaskbarInteraction(FALSE);

    // 使用 RichEdit 控件时需要启用下面这一行。
    // AfxInitRichEdit2();

    // 设置保存应用程序配置的注册表项，并加载标准配置。
    SetRegistryKey(_T("应用程序向导生成的本地应用程序"));
    LoadStdProfileSettings(4);

    // 注册应用程序的文档模板。
    CSingleDocTemplate* pDocTemplate = new CSingleDocTemplate(
        IDR_MAINFRAME,
        RUNTIME_CLASS(CBubbleBattleDoc),
        RUNTIME_CLASS(CMainFrame),
        RUNTIME_CLASS(CBubbleGameUIView));

    if (pDocTemplate == nullptr)
        return FALSE;

    AddDocTemplate(pDocTemplate);

    // 分析并处理命令行。
    CCommandLineInfo cmdInfo;
    ParseCommandLine(cmdInfo);

    if (!ProcessShellCommand(cmdInfo))
        return FALSE;

    // 主窗口创建完成后，直接以最大化状态显示。
    // SW_SHOWMAXIMIZED 是本次调整窗口大小的关键修改。
    m_pMainWnd->ShowWindow(SW_SHOWMAXIMIZED);
    m_pMainWnd->UpdateWindow();

    return TRUE;
}

// CBubbleBattleApp 消息处理程序

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
    CAboutDlg() noexcept;

    // 对话框数据
#ifdef AFX_DESIGN_TIME
    enum { IDD = IDD_ABOUTBOX };
#endif

protected:
    virtual void DoDataExchange(CDataExchange* pDX);

protected:
    DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() noexcept
    : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// 用于运行“关于”对话框的应用程序命令。
void CBubbleBattleApp::OnAppAbout()
{
    CAboutDlg aboutDlg;
    aboutDlg.DoModal();
}

// CBubbleBattleApp 消息处理程序
