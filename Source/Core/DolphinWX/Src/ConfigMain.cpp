// Copyright (C) 2003 Dolphin Project.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, version 2.0.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License 2.0 for more details.

// A copy of the GPL 2.0 should have been included with the program.
// If not, see http://www.gnu.org/licenses/

// Official SVN repository and contact information can be found at
// http://code.google.com/p/dolphin-emu/

#include <string> // System
#include <vector>

#include "Core.h" // Core
#include "HW/EXI.h"
#include "HW/SI.h"

#include "Globals.h" // Local
#include "ConfigMain.h"
#include "PluginManager.h"
#include "ConfigManager.h"
#include "SysConf.h"
#include "Frame.h"
#include "HotkeyDlg.h"

#include "..\..\Common\Src\OpenCL.h"

#ifdef __APPLE__
#include <Cocoa/Cocoa.h>
#endif

extern CFrame* main_frame;

// Strings for Device Selections
#define DEV_NONE_STR		"<Nothing>"
#define DEV_DUMMY_STR		"Dummy"

#define SIDEV_STDCONT_STR	"Standard Controller"
#define SIDEV_GBA_STR		"GBA"
#define SIDEV_AM_BB_STR		"AM-Baseboard"

#define EXIDEV_MEMCARD_STR	"Memory Card"
#define EXIDEV_MIC_STR		"Mic"
#define EXIDEV_BBA_STR		"BBA"
#define EXIDEV_AM_BB_STR	"AM-Baseboard"

BEGIN_EVENT_TABLE(CConfigMain, wxDialog)

EVT_CLOSE(CConfigMain::OnClose)
EVT_BUTTON(wxID_OK, CConfigMain::OnOk)


EVT_CHECKBOX(ID_CPUTHREAD, CConfigMain::CoreSettingsChanged)
EVT_CHECKBOX(ID_IDLESKIP, CConfigMain::CoreSettingsChanged)
EVT_CHECKBOX(ID_ENABLECHEATS, CConfigMain::CoreSettingsChanged)
EVT_CHOICE(ID_FRAMELIMIT, CConfigMain::CoreSettingsChanged)
EVT_CHECKBOX(ID_FRAMELIMIT_USEFPSFORLIMITING, CConfigMain::CoreSettingsChanged)

EVT_CHECKBOX(ID_ALWAYS_HLE_BS2, CConfigMain::CoreSettingsChanged)
EVT_CHECKBOX(ID_ENABLE_OPENCL, CConfigMain::CoreSettingsChanged)
EVT_RADIOBOX(ID_CPUENGINE, CConfigMain::CoreSettingsChanged)
EVT_CHECKBOX(ID_LOCKTHREADS, CConfigMain::CoreSettingsChanged)
EVT_CHECKBOX(ID_DSPTHREAD, CConfigMain::CoreSettingsChanged)


EVT_CHOICE(ID_DISPLAY_FULLSCREENRES, CConfigMain::DisplaySettingsChanged)
EVT_TEXT(ID_DISPLAY_WINDOWWIDTH, CConfigMain::DisplaySettingsChanged)
EVT_TEXT(ID_DISPLAY_WINDOWHEIGHT, CConfigMain::DisplaySettingsChanged)
EVT_CHECKBOX(ID_DISPLAY_FULLSCREEN, CConfigMain::DisplaySettingsChanged)
EVT_CHECKBOX(ID_DISPLAY_HIDECURSOR, CConfigMain::DisplaySettingsChanged)
EVT_CHECKBOX(ID_DISPLAY_RENDERTOMAIN, CConfigMain::DisplaySettingsChanged)

EVT_CHECKBOX(ID_INTERFACE_CONFIRMSTOP, CConfigMain::DisplaySettingsChanged)
EVT_CHECKBOX(ID_INTERFACE_USEPANICHANDLERS, CConfigMain::DisplaySettingsChanged)
EVT_RADIOBOX(ID_INTERFACE_THEME, CConfigMain::DisplaySettingsChanged)
EVT_CHOICE(ID_INTERFACE_LANG, CConfigMain::DisplaySettingsChanged)
EVT_BUTTON(ID_HOTKEY_CONFIG, CConfigMain::DisplaySettingsChanged)


EVT_CHOICE(ID_GC_SRAM_LNG, CConfigMain::GCSettingsChanged)

EVT_CHOICE(ID_GC_EXIDEVICE_SLOTA, CConfigMain::GCSettingsChanged)
EVT_BUTTON(ID_GC_EXIDEVICE_SLOTA_PATH, CConfigMain::GCSettingsChanged)
EVT_CHOICE(ID_GC_EXIDEVICE_SLOTB, CConfigMain::GCSettingsChanged)
EVT_BUTTON(ID_GC_EXIDEVICE_SLOTB_PATH, CConfigMain::GCSettingsChanged)
EVT_CHOICE(ID_GC_EXIDEVICE_SP1, CConfigMain::GCSettingsChanged)
EVT_CHOICE(ID_GC_SIDEVICE0, CConfigMain::GCSettingsChanged)
EVT_CHOICE(ID_GC_SIDEVICE1, CConfigMain::GCSettingsChanged)
EVT_CHOICE(ID_GC_SIDEVICE2, CConfigMain::GCSettingsChanged)
EVT_CHOICE(ID_GC_SIDEVICE3, CConfigMain::GCSettingsChanged)


EVT_CHOICE(ID_WII_BT_BAR, CConfigMain::WiiSettingsChanged)

EVT_CHECKBOX(ID_WII_IPL_SSV, CConfigMain::WiiSettingsChanged)
EVT_CHECKBOX(ID_WII_IPL_PGS, CConfigMain::WiiSettingsChanged)
EVT_CHECKBOX(ID_WII_IPL_E60, CConfigMain::WiiSettingsChanged)
EVT_CHOICE(ID_WII_IPL_AR, CConfigMain::WiiSettingsChanged)
EVT_CHOICE(ID_WII_IPL_LNG, CConfigMain::WiiSettingsChanged)

EVT_CHECKBOX(ID_WII_SD_CARD, CConfigMain::WiiSettingsChanged)
EVT_CHECKBOX(ID_WII_KEYBOARD, CConfigMain::WiiSettingsChanged)


EVT_LISTBOX(ID_ISOPATHS, CConfigMain::ISOPathsSelectionChanged)
EVT_CHECKBOX(ID_RECURSIVEISOPATH, CConfigMain::RecursiveDirectoryChanged)
EVT_BUTTON(ID_ADDISOPATH, CConfigMain::AddRemoveISOPaths)
EVT_BUTTON(ID_REMOVEISOPATH, CConfigMain::AddRemoveISOPaths)

EVT_FILEPICKER_CHANGED(ID_DEFAULTISO, CConfigMain::DefaultISOChanged)
EVT_DIRPICKER_CHANGED(ID_DVDROOT, CConfigMain::DVDRootChanged)
EVT_FILEPICKER_CHANGED(ID_APPLOADERPATH, CConfigMain::ApploaderPathChanged)


EVT_CHOICE(ID_GRAPHIC_CB, CConfigMain::OnSelectionChanged)
EVT_BUTTON(ID_GRAPHIC_CONFIG, CConfigMain::OnConfig)

EVT_CHOICE(ID_DSP_CB, CConfigMain::OnSelectionChanged)
EVT_BUTTON(ID_DSP_CONFIG, CConfigMain::OnConfig)

EVT_CHOICE(ID_WIIMOTE_CB, CConfigMain::OnSelectionChanged)
EVT_BUTTON(ID_WIIMOTE_CONFIG, CConfigMain::OnConfig)

END_EVENT_TABLE()

CConfigMain::CConfigMain(wxWindow* parent, wxWindowID id, const wxString& title, const wxPoint& position, const wxSize& size, long style)
	: wxDialog(parent, id, title, position, size, style)
{
	// Control refreshing of the ISOs list
	bRefreshList = false;

	AddResolutions();
	CreateGUIControls();

	// Update selected ISO paths
	for(u32 i = 0; i < SConfig::GetInstance().m_ISOFolder.size(); i++)
	{
		ISOPaths->Append(wxString(SConfig::GetInstance().m_ISOFolder[i].c_str(), *wxConvCurrent));
	}
}

CConfigMain::~CConfigMain()
{
}

// Used to restrict changing of some options while emulator is running
void CConfigMain::UpdateGUI()
{
	if(Core::GetState() != Core::CORE_UNINITIALIZED)
	{
		// Disable the Core stuff on GeneralPage
		CPUThread->Disable();
		SkipIdle->Disable();
		EnableCheats->Disable();
		
		AlwaysHLE_BS2->Disable();
		EnableOpenCL->Disable();
		CPUEngine->Disable();
		LockThreads->Disable();
		DSPThread->Disable();
		

		// Disable stuff on DisplayPage
		FullscreenResolution->Disable();
		RenderToMain->Disable();
		

		// Disable stuff on GamecubePage
		GCSystemLang->Disable();
		

		// Disable stuff on WiiPage
		WiiSensBarPos->Disable();
		WiiScreenSaver->Disable();
		WiiProgressiveScan->Disable();
		WiiEuRGB60->Disable();
		WiiAspectRatio->Disable();
		WiiSystemLang->Disable();


		// Disable stuff on PathsPage
		PathsPage->Disable();


		// Disable stuff on PluginsPage
		GraphicSelection->Disable();
		DSPSelection->Disable();
		WiimoteSelection->Disable();
	}
}

void CConfigMain::InitializeGUILists()
{
	// General page
	// Framelimit
	arrayStringFor_Framelimit.Add(wxT("Off"));
	arrayStringFor_Framelimit.Add(wxT("Auto"));
	for (int i = 10; i <= 120; i += 5)	// from 10 to 120
		arrayStringFor_Framelimit.Add(wxString::Format(wxT("%i"), i));

	// Emulator Engine
	arrayStringFor_CPUEngine.Add(wxT("Interpreter (VERY slow)"));
	arrayStringFor_CPUEngine.Add(wxT("JIT Recompiler (recommended)"));
	arrayStringFor_CPUEngine.Add(wxT("JITIL experimental recompiler"));
	
	
	// Display page
	// Resolutions
	if (arrayStringFor_FullscreenResolution.empty())
		arrayStringFor_FullscreenResolution.Add(wxT("<No resolutions found>"));

	// Themes
	arrayStringFor_Themes.Add(wxT("Boomy"));
	arrayStringFor_Themes.Add(wxT("Vista"));
	arrayStringFor_Themes.Add(wxT("X-Plastik"));
	arrayStringFor_Themes.Add(wxT("KDE"));

	// GUI language arrayStrings
	arrayStringFor_InterfaceLang.Add(wxT("English"));
	arrayStringFor_InterfaceLang.Add(wxT("German"));
	arrayStringFor_InterfaceLang.Add(wxT("French"));
	arrayStringFor_InterfaceLang.Add(wxT("Spanish"));
	arrayStringFor_InterfaceLang.Add(wxT("Italian"));
	arrayStringFor_InterfaceLang.Add(wxT("Dutch"));
	
	
	// Gamecube page
	// GC Language arrayStrings
	arrayStringFor_GCSystemLang = arrayStringFor_InterfaceLang;
	
	
	// Wii page
	// Sensorbar Position
	arrayStringFor_WiiSensBarPos.Add(wxT("Bottom"));
	arrayStringFor_WiiSensBarPos.Add(wxT("Top"));
	
	// Aspect ratio
	arrayStringFor_WiiAspectRatio.Add(wxT("4:3"));
	arrayStringFor_WiiAspectRatio.Add(wxT("16:9"));
	
	// Wii Language arrayStrings
	arrayStringFor_WiiSystemLang = arrayStringFor_InterfaceLang;
	arrayStringFor_WiiSystemLang.Insert(wxT("Japanese"), 0);
	arrayStringFor_WiiSystemLang.Add(wxT("Simplified Chinese"));
	arrayStringFor_WiiSystemLang.Add(wxT("Traditional Chinese"));
	arrayStringFor_WiiSystemLang.Add(wxT("Korean"));
	
}

void CConfigMain::InitializeGUIValues()
{
	// General - Basic
	CPUThread->SetValue(SConfig::GetInstance().m_LocalCoreStartupParameter.bCPUThread);
	SkipIdle->SetValue(SConfig::GetInstance().m_LocalCoreStartupParameter.bSkipIdle);
	EnableCheats->SetValue(SConfig::GetInstance().m_LocalCoreStartupParameter.bEnableCheats);
	Framelimit->SetSelection(SConfig::GetInstance().m_Framelimit);
	UseFPSForLimiting->SetValue(SConfig::GetInstance().b_UseFPS);

	// General - Advanced
	AlwaysHLE_BS2->SetValue(SConfig::GetInstance().m_LocalCoreStartupParameter.bHLE_BS2);
	EnableOpenCL->SetValue(SConfig::GetInstance().m_LocalCoreStartupParameter.bEnableOpenCL);
	CPUEngine->SetSelection(SConfig::GetInstance().m_LocalCoreStartupParameter.iCPUCore);
	LockThreads->SetValue(SConfig::GetInstance().m_LocalCoreStartupParameter.bLockThreads);
	DSPThread->SetValue(SConfig::GetInstance().m_LocalCoreStartupParameter.bDSPThread);


	// Display - Display
	FullscreenResolution->SetStringSelection(wxString::FromAscii(SConfig::GetInstance().m_LocalCoreStartupParameter.strFullscreenResolution.c_str()));
	WindowWidth->SetValue(SConfig::GetInstance().m_LocalCoreStartupParameter.iRenderWindowWidth);
	WindowHeight->SetValue(SConfig::GetInstance().m_LocalCoreStartupParameter.iRenderWindowHeight);
	Fullscreen->SetValue(SConfig::GetInstance().m_LocalCoreStartupParameter.bFullscreen);
	HideCursor->SetValue(SConfig::GetInstance().m_LocalCoreStartupParameter.bHideCursor);
	RenderToMain->SetValue(SConfig::GetInstance().m_LocalCoreStartupParameter.bRenderToMain);

	// Display - Interface
	ConfirmStop->SetValue(SConfig::GetInstance().m_LocalCoreStartupParameter.bConfirmStop);
	UsePanicHandlers->SetValue(SConfig::GetInstance().m_LocalCoreStartupParameter.bUsePanicHandlers);
	Theme->SetSelection(SConfig::GetInstance().m_LocalCoreStartupParameter.iTheme);
	// need redesign
	InterfaceLang->SetSelection(SConfig::GetInstance().m_InterfaceLanguage);


	// Gamecube - IPL
	GCSystemLang->SetSelection(SConfig::GetInstance().m_LocalCoreStartupParameter.SelectedLanguage);

	// Gamecube - Devices
	// Not here. They use some locals over in CreateGUIControls for initialization,
	// which is why they are still there.


	// Wii - Wiimote
	WiiSensBarPos->SetSelection(SConfig::GetInstance().m_SYSCONF->GetData<u8>("BT.BAR"));
	
	// Wii - Misc
	WiiScreenSaver->SetValue(!!SConfig::GetInstance().m_SYSCONF->GetData<u8>("IPL.SSV"));
	WiiProgressiveScan->SetValue(!!SConfig::GetInstance().m_SYSCONF->GetData<u8>("IPL.PGS"));
	WiiEuRGB60->SetValue(!!SConfig::GetInstance().m_SYSCONF->GetData<u8>("IPL.E60"));
	WiiAspectRatio->SetSelection(SConfig::GetInstance().m_SYSCONF->GetData<u8>("IPL.AR"));
	WiiSystemLang->SetSelection(SConfig::GetInstance().m_SYSCONF->GetData<u8>("IPL.LNG"));
	
	// Wii - Devices
	WiiSDCard->SetValue(SConfig::GetInstance().m_WiiSDCard);
	WiiKeyboard->SetValue(SConfig::GetInstance().m_WiiKeyboard);


	// Paths
	RecursiveISOPath->SetValue(SConfig::GetInstance().m_RecursiveISOFolder);
	DefaultISO->SetPath(wxString(SConfig::GetInstance().m_LocalCoreStartupParameter.m_strDefaultGCM.c_str(), *wxConvCurrent));
	DVDRoot->SetPath(wxString(SConfig::GetInstance().m_LocalCoreStartupParameter.m_strDVDRoot.c_str(), *wxConvCurrent));
	ApploaderPath->SetPath(wxString(SConfig::GetInstance().m_LocalCoreStartupParameter.m_strApploader.c_str(), *wxConvCurrent));


	// Plugins
	FillChoiceBox(GraphicSelection, PLUGIN_TYPE_VIDEO, SConfig::GetInstance().m_LocalCoreStartupParameter.m_strVideoPlugin);
	FillChoiceBox(DSPSelection, PLUGIN_TYPE_DSP, SConfig::GetInstance().m_LocalCoreStartupParameter.m_strDSPPlugin);
	FillChoiceBox(WiimoteSelection, PLUGIN_TYPE_WIIMOTE, SConfig::GetInstance().m_LocalCoreStartupParameter.m_strWiimotePlugin);
}

void CConfigMain::InitializeGUITooltips()
{
	// General - Basic
	CPUThread->SetToolTip(wxT("This splits the Video and CPU threads, so they can be run on separate cores.")
		wxT("\nCauses major speed improvements on PCs with more than one core,")
		wxT("\nbut can also cause occasional crashes/glitches."));
	Framelimit->SetToolTip(wxT("If you set Framelimit higher than game full speed (NTSC:60, PAL:50),\nyou also have to disable Audio Throttle in DSP to make it effective."));

	// General - Advanced
	DSPThread->SetToolTip(wxT("Run DSPLLE on a dedicated thread (not recommended)."));
	EnableOpenCL->SetToolTip(wxT("Allow videocard to accelerate texture decoding."));


	// Display - Display
	FullscreenResolution->SetToolTip(wxT("Select resolution for fullscreen mode"));
	WindowWidth->SetToolTip(wxT("Window width for windowed mode"));
	WindowHeight->SetToolTip(wxT("Window height for windowed mode"));
	Fullscreen->SetToolTip(wxT("Start the rendering window in fullscreen mode."));
	HideCursor->SetToolTip(wxT("Hide the cursor when it is over the rendering window")
		wxT("\n and the rendering window has focus."));
	RenderToMain->SetToolTip(wxT("Render to main window."));

	// Display - Interface
	ConfirmStop->SetToolTip(wxT("Show a confirmation box before stopping a game."));
	UsePanicHandlers->SetToolTip(wxT("Show a message box when a potentially serious error has occured.")
		wxT(" Disabling this may avoid annoying and non-fatal messages, but it may also mean that Dolphin")
		wxT(" suddenly crashes without any explanation at all."));

	// Display - Themes: Copyright notice
	Theme->SetItemToolTip(0, wxT("Created by Milosz Wlazlo [miloszwl@miloszwl.com, miloszwl.deviantart.com]"));
	Theme->SetItemToolTip(1, wxT("Created by VistaIcons.com"));
	Theme->SetItemToolTip(2, wxT("Created by black_rider and published on ForumW.org > Web Developments"));
	Theme->SetItemToolTip(3, wxT("Created by KDE-Look.org"));

	InterfaceLang->SetToolTip(wxT("For the time being this will only change the text shown in")
		wxT("\nthe game list of PAL GC games."));


	// Gamecube - Devices
	GCEXIDevice[2]->SetToolTip(wxT("Serial Port 1 - This is the port which devices such as the net adapter use"));


	// Wii - Devices
	WiiKeyboard->SetToolTip(wxT("This could cause slow down in Wii Menu and some games."));
}

void CConfigMain::CreateGUIControls()
{
	InitializeGUILists();
	
	// Create the notebook and pages
	Notebook = new wxNotebook(this, ID_NOTEBOOK, wxDefaultPosition, wxDefaultSize);
	GeneralPage = new wxPanel(Notebook, ID_GENERALPAGE, wxDefaultPosition, wxDefaultSize);
	DisplayPage = new wxPanel(Notebook, ID_DISPLAYPAGE, wxDefaultPosition, wxDefaultSize);
	GamecubePage = new wxPanel(Notebook, ID_GAMECUBEPAGE, wxDefaultPosition, wxDefaultSize);
	WiiPage = new wxPanel(Notebook, ID_WIIPAGE, wxDefaultPosition, wxDefaultSize);
	PathsPage = new wxPanel(Notebook, ID_PATHSPAGE, wxDefaultPosition, wxDefaultSize);
	PluginsPage = new wxPanel(Notebook, ID_PLUGINPAGE, wxDefaultPosition, wxDefaultSize);

	Notebook->AddPage(GeneralPage, wxT("General"));
	Notebook->AddPage(DisplayPage, wxT("Display"));
	Notebook->AddPage(GamecubePage, wxT("Gamecube"));
	Notebook->AddPage(WiiPage, wxT("Wii"));
	Notebook->AddPage(PathsPage, wxT("Paths"));
	Notebook->AddPage(PluginsPage, wxT("Plugins"));

	// General page
	// Core Settings - Basic
	sbBasic = new wxStaticBoxSizer(wxVERTICAL, GeneralPage, wxT("Basic Settings"));
	CPUThread = new wxCheckBox(GeneralPage, ID_CPUTHREAD, wxT("Enable Dual Core (speedup)"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	SkipIdle = new wxCheckBox(GeneralPage, ID_IDLESKIP, wxT("Enable Idle Skipping (speedup)"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	EnableCheats = new wxCheckBox(GeneralPage, ID_ENABLECHEATS, wxT("Enable Cheats"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	// Framelimit
	wxStaticText* FramelimitText = new wxStaticText(GeneralPage, ID_FRAMELIMIT_TEXT, wxT("Framelimit :"), wxDefaultPosition, wxDefaultSize);
	Framelimit = new wxChoice(GeneralPage, ID_FRAMELIMIT, wxDefaultPosition, wxDefaultSize, arrayStringFor_Framelimit, 0, wxDefaultValidator);
	UseFPSForLimiting = new wxCheckBox(GeneralPage, ID_FRAMELIMIT_USEFPSFORLIMITING, wxT("Use FPS  For Limiting"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);

	// Core Settings - Advanced
	sbAdvanced = new wxStaticBoxSizer(wxVERTICAL, GeneralPage, wxT("Advanced Settings"));
	AlwaysHLE_BS2 = new wxCheckBox(GeneralPage, ID_ALWAYS_HLE_BS2, wxT("HLE the IPL (recommended)"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	EnableOpenCL = new wxCheckBox(GeneralPage, ID_ENABLE_OPENCL, wxT("Enable OpenCL"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
#if !(defined(HAVE_OPENCL) && HAVE_OPENCL)
		EnableOpenCL->Enable(false);
#endif
	CPUEngine = new wxRadioBox(GeneralPage, ID_CPUENGINE, wxT("CPU Emulator Engine"), wxDefaultPosition, wxDefaultSize, arrayStringFor_CPUEngine, 0, wxRA_SPECIFY_ROWS);
	LockThreads = new wxCheckBox(GeneralPage, ID_LOCKTHREADS, wxT("Lock threads to cores"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	DSPThread = new wxCheckBox(GeneralPage, ID_DSPTHREAD, wxT("DSPLLE on thread"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);

	// Populate the settings
	sbBasic->Add(CPUThread, 0, wxALL, 5);
	sbBasic->Add(SkipIdle, 0, wxALL, 5);
	sbBasic->Add(EnableCheats, 0, wxALL, 5);
	wxBoxSizer* sFramelimit = new wxBoxSizer(wxHORIZONTAL);
	sFramelimit->Add(FramelimitText, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	sFramelimit->Add(Framelimit, 0, wxALL | wxEXPAND, 5);
	sFramelimit->Add(UseFPSForLimiting, 0, wxALL | wxEXPAND, 5);
	sbBasic->Add(sFramelimit, 0, wxALL | wxEXPAND, 5);

	sbAdvanced->Add(AlwaysHLE_BS2, 0, wxALL, 5);
	sbAdvanced->Add(EnableOpenCL, 0, wxALL, 5);
	sbAdvanced->Add(CPUEngine, 0, wxALL, 5);
	sbAdvanced->Add(LockThreads, 0, wxALL, 5);
	sbAdvanced->Add(DSPThread, 0, wxALL, 5);

	// Populate the General page
	sGeneralPage = new wxBoxSizer(wxVERTICAL);
	sGeneralPage->Add(sbBasic, 0, wxEXPAND | wxALL, 5);
	sGeneralPage->Add(sbAdvanced, 0, wxEXPAND | wxALL, 5);

	GeneralPage->SetSizer(sGeneralPage);
	sGeneralPage->Layout();
	
	
	
	// Display page
	// General display settings
	sbDisplay = new wxStaticBoxSizer(wxVERTICAL, DisplayPage, wxT("Emulator Display Settings"));
	wxStaticText* FullscreenResolutionText = new wxStaticText(DisplayPage, wxID_ANY, wxT("Fullscreen Display Resolution:"), wxDefaultPosition, wxDefaultSize, 0);
	FullscreenResolution = new wxChoice(DisplayPage, ID_DISPLAY_FULLSCREENRES, wxDefaultPosition, wxDefaultSize, arrayStringFor_FullscreenResolution, 0, wxDefaultValidator, arrayStringFor_FullscreenResolution[0]);
	wxStaticText *WindowSizeText = new wxStaticText(DisplayPage, wxID_ANY, wxT("Window Size:"), wxDefaultPosition, wxDefaultSize, 0);
	WindowWidth = new wxSpinCtrl(DisplayPage, ID_DISPLAY_WINDOWWIDTH, wxEmptyString, wxDefaultPosition, wxSize(70, -1));
	WindowWidth->SetRange(0,3280);
	wxStaticText *WindowXText = new wxStaticText(DisplayPage, wxID_ANY, wxT("x"), wxDefaultPosition, wxDefaultSize, 0);
	WindowHeight = new wxSpinCtrl(DisplayPage, ID_DISPLAY_WINDOWHEIGHT, wxEmptyString, wxDefaultPosition, wxSize(70, -1));
	WindowHeight->SetRange(0,2048);
	Fullscreen = new wxCheckBox(DisplayPage, ID_DISPLAY_FULLSCREEN, wxT("Start Renderer in Fullscreen"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	HideCursor = new wxCheckBox(DisplayPage, ID_DISPLAY_HIDECURSOR, wxT("Hide Mouse Cursor"));
	RenderToMain = new wxCheckBox(DisplayPage, ID_DISPLAY_RENDERTOMAIN, wxT("Render to Main Window"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);

	// Interface settings
	sbInterface = new wxStaticBoxSizer(wxVERTICAL, DisplayPage, wxT("Interface Settings"));
	ConfirmStop = new wxCheckBox(DisplayPage, ID_INTERFACE_CONFIRMSTOP, wxT("Confirm On Stop"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	UsePanicHandlers = new wxCheckBox(DisplayPage, ID_INTERFACE_USEPANICHANDLERS, wxT("Use Panic Handlers"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	
	// Themes - this should really be a wxChoice...
	Theme = new wxRadioBox(DisplayPage, ID_INTERFACE_THEME, wxT("Theme"), wxDefaultPosition, wxDefaultSize, arrayStringFor_Themes, 1, wxRA_SPECIFY_ROWS);

	// Interface Language
	// At the moment this only changes the language displayed in m_gamelistctrl
	// If someone wants to control the whole GUI's language, it should be set here too
	wxStaticText* InterfaceLangText = new wxStaticText(DisplayPage, ID_INTERFACE_LANG_TEXT, wxT("Game List Language:"), wxDefaultPosition, wxDefaultSize);
	InterfaceLang = new wxChoice(DisplayPage, ID_INTERFACE_LANG, wxDefaultPosition, wxDefaultSize, arrayStringFor_InterfaceLang, 0, wxDefaultValidator);

	// Hotkey configuration
	HotkeyConfig = new wxButton(DisplayPage, ID_HOTKEY_CONFIG, wxT("Hotkeys"), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT, wxDefaultValidator);

	// Populate the settings
	wxBoxSizer* sDisplayRes = new wxBoxSizer(wxHORIZONTAL);
	sDisplayRes->Add(FullscreenResolutionText, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	sDisplayRes->Add(FullscreenResolution, 0, wxEXPAND | wxALL, 5);
	sbDisplay->Add(sDisplayRes, 0, wxALL, 5);
	wxBoxSizer* sDisplaySize = new wxBoxSizer(wxHORIZONTAL);
	sDisplaySize->Add(WindowSizeText, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	sDisplaySize->Add(WindowWidth, 0, wxEXPAND | wxALL, 5);
	sDisplaySize->Add(WindowXText, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	sDisplaySize->Add(WindowHeight, 0, wxEXPAND | wxALL, 5);
	sbDisplay->Add(sDisplaySize, 0, wxALL, 5);
	sbDisplay->Add(Fullscreen, 0, wxEXPAND | wxALL, 5);
	sbDisplay->Add(HideCursor, 0, wxALL, 5);
	sbDisplay->Add(RenderToMain, 0, wxEXPAND | wxALL, 5);

	sbInterface->Add(ConfirmStop, 0, wxALL, 5);
	sbInterface->Add(UsePanicHandlers, 0, wxALL, 5);
	sbInterface->Add(Theme, 0, wxEXPAND | wxALL, 5);
	wxBoxSizer* sInterface = new wxBoxSizer(wxHORIZONTAL);
	sInterface->Add(InterfaceLangText, 0, wxALIGN_CENTER_VERTICAL | wxALL, 5);
	sInterface->Add(InterfaceLang, 0, wxEXPAND | wxALL, 5);
	sInterface->AddStretchSpacer();
	sInterface->Add(HotkeyConfig, 0, wxALIGN_RIGHT | wxALL, 5);
	sbInterface->Add(sInterface, 0, wxEXPAND | wxALL, 5);

	// Populate the Display page
	sDisplayPage = new wxBoxSizer(wxVERTICAL);
	sDisplayPage->Add(sbDisplay, 0, wxEXPAND | wxALL, 5);
	sDisplayPage->Add(sbInterface, 0, wxEXPAND | wxALL, 5);

	DisplayPage->SetSizer(sDisplayPage);
	sDisplayPage->Layout();



	// Gamecube page
	// IPL settings
	sbGamecubeIPLSettings = new wxStaticBoxSizer(wxVERTICAL, GamecubePage, wxT("IPL Settings"));
	wxStaticText* GCSystemLangText = new wxStaticText(GamecubePage, ID_GC_SRAM_LNG_TEXT, wxT("System Language:"), wxDefaultPosition, wxDefaultSize);
	GCSystemLang = new wxChoice(GamecubePage, ID_GC_SRAM_LNG, wxDefaultPosition, wxDefaultSize, arrayStringFor_GCSystemLang, 0, wxDefaultValidator);
	// Device settings
	// EXI Devices
	wxStaticBoxSizer *sbGamecubeDeviceSettings = new wxStaticBoxSizer(wxVERTICAL, GamecubePage, wxT("Device Settings"));
	wxStaticText* GCEXIDeviceText[3];
	GCEXIDeviceText[0] = new wxStaticText(GamecubePage, ID_GC_EXIDEVICE_SLOTA_TEXT, wxT("Slot A"), wxDefaultPosition, wxDefaultSize);
	GCEXIDeviceText[1] = new wxStaticText(GamecubePage, ID_GC_EXIDEVICE_SLOTB_TEXT, wxT("Slot B"), wxDefaultPosition, wxDefaultSize);
	GCEXIDeviceText[2] = new wxStaticText(GamecubePage, ID_GC_EXIDEVICE_SP1_TEXT,	wxT("SP1   "), wxDefaultPosition, wxDefaultSize);
	const wxString SlotDevices[] = {wxT(DEV_NONE_STR), wxT(DEV_DUMMY_STR), wxT(EXIDEV_MEMCARD_STR)
	#if HAVE_PORTAUDIO
		, wxT(EXIDEV_MIC_STR)
	#endif
	};
	static const int numSlotDevices = sizeof(SlotDevices)/sizeof(wxString);
	const wxString SP1Devices[] = { wxT(DEV_NONE_STR), wxT(DEV_DUMMY_STR), wxT(EXIDEV_BBA_STR), wxT(EXIDEV_AM_BB_STR) };
	static const int numSP1Devices = sizeof(SP1Devices)/sizeof(wxString);
	GCEXIDevice[0] = new wxChoice(GamecubePage, ID_GC_EXIDEVICE_SLOTA, wxDefaultPosition, wxDefaultSize, numSlotDevices, SlotDevices, 0, wxDefaultValidator);
	GCEXIDevice[1] = new wxChoice(GamecubePage, ID_GC_EXIDEVICE_SLOTB, wxDefaultPosition, wxDefaultSize, numSlotDevices, SlotDevices, 0, wxDefaultValidator);
	GCEXIDevice[2] = new wxChoice(GamecubePage, ID_GC_EXIDEVICE_SP1, wxDefaultPosition, wxDefaultSize, numSP1Devices, SP1Devices, 0, wxDefaultValidator);
	GCMemcardPath[0] = new wxButton(GamecubePage, ID_GC_EXIDEVICE_SLOTA_PATH, wxT("..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT, wxDefaultValidator);
	GCMemcardPath[1] = new wxButton(GamecubePage, ID_GC_EXIDEVICE_SLOTB_PATH, wxT("..."), wxDefaultPosition, wxDefaultSize, wxBU_EXACTFIT, wxDefaultValidator);
	// Can't move this one without making the 4 const's etc. above class members/fields,
	for (int i = 0; i < 3; ++i)
	{
		bool isMemcard = false;
		switch (SConfig::GetInstance().m_EXIDevice[i])
		{
		case EXIDEVICE_NONE:
			GCEXIDevice[i]->SetStringSelection(SlotDevices[0]);
			break;
		case EXIDEVICE_MEMORYCARD_A:
		case EXIDEVICE_MEMORYCARD_B:
			isMemcard = GCEXIDevice[i]->SetStringSelection(SlotDevices[2]);
			break;
		case EXIDEVICE_MIC:
			GCEXIDevice[i]->SetStringSelection(SlotDevices[3]);
			break;
		case EXIDEVICE_ETH:
			GCEXIDevice[i]->SetStringSelection(SP1Devices[2]);
			break;
		case EXIDEVICE_AM_BASEBOARD:
			GCEXIDevice[i]->SetStringSelection(SP1Devices[3]);
			break;
		case EXIDEVICE_DUMMY:
		default:
			GCEXIDevice[i]->SetStringSelection(SlotDevices[1]);
			break;
		}
		if (!isMemcard && i < 2)
			GCMemcardPath[i]->Disable();
	}
	//SI Devices
	wxStaticText* GCSIDeviceText[4];
	GCSIDeviceText[0] = new wxStaticText(GamecubePage, ID_GC_SIDEVICE_TEXT, wxT("Port 1"), wxDefaultPosition, wxDefaultSize);
	GCSIDeviceText[1] = new wxStaticText(GamecubePage, ID_GC_SIDEVICE_TEXT, wxT("Port 2"), wxDefaultPosition, wxDefaultSize);
	GCSIDeviceText[2] = new wxStaticText(GamecubePage, ID_GC_SIDEVICE_TEXT, wxT("Port 3"), wxDefaultPosition, wxDefaultSize);
	GCSIDeviceText[3] = new wxStaticText(GamecubePage, ID_GC_SIDEVICE_TEXT, wxT("Port 4"), wxDefaultPosition, wxDefaultSize);
	const wxString SIPort1Devices[] = {wxT(DEV_NONE_STR),wxT(SIDEV_STDCONT_STR),wxT(SIDEV_GBA_STR),wxT(SIDEV_AM_BB_STR)};
	static const int numSIPort1Devices = sizeof(SIPort1Devices)/sizeof(wxString);
	const wxString SIDevices[] = {wxT(DEV_NONE_STR),wxT(SIDEV_STDCONT_STR),wxT(SIDEV_GBA_STR)};
	static const int numSIDevices = sizeof(SIDevices)/sizeof(wxString);
	GCSIDevice[0] = new wxChoice(GamecubePage, ID_GC_SIDEVICE0, wxDefaultPosition, wxDefaultSize, numSIPort1Devices, SIPort1Devices, 0, wxDefaultValidator);
	GCSIDevice[1] = new wxChoice(GamecubePage, ID_GC_SIDEVICE1, wxDefaultPosition, wxDefaultSize, numSIDevices, SIDevices, 0, wxDefaultValidator);
	GCSIDevice[2] = new wxChoice(GamecubePage, ID_GC_SIDEVICE2, wxDefaultPosition, wxDefaultSize, numSIDevices, SIDevices, 0, wxDefaultValidator);
	GCSIDevice[3] = new wxChoice(GamecubePage, ID_GC_SIDEVICE3, wxDefaultPosition, wxDefaultSize, numSIDevices, SIDevices, 0, wxDefaultValidator);
	// Can't move this one without making the 2 const's etc. above class members/fields.
	for (int i = 0; i < 4; ++i)
	{
		switch (SConfig::GetInstance().m_SIDevice[i])
		{
		case SI_GC_CONTROLLER:
			GCSIDevice[i]->SetStringSelection(SIDevices[1]);
			break;
		case SI_GBA:
			GCSIDevice[i]->SetStringSelection(SIDevices[2]);
			break;
		case SI_AM_BASEBOARD:
			GCSIDevice[i]->SetStringSelection(SIDevices[3]);
			break;
		default:
			GCSIDevice[i]->SetStringSelection(SIDevices[0]);
			break;
		}
	}

	// Populate the settings
	sGamecubeIPLSettings = new wxGridBagSizer();
	sGamecubeIPLSettings->Add(GCSystemLangText, wxGBPosition(0, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL|wxALL, 5);
	sGamecubeIPLSettings->Add(GCSystemLang, wxGBPosition(0, 1), wxDefaultSpan, wxALL, 5);
	sbGamecubeIPLSettings->Add(sGamecubeIPLSettings);
	wxBoxSizer *sEXIDevices[4], *sSIDevices[4];
	for (int i = 0; i < 3; ++i)
	{
		sEXIDevices[i] = new wxBoxSizer(wxHORIZONTAL);
		sEXIDevices[i]->Add(GCEXIDeviceText[i], 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
		sEXIDevices[i]->Add(GCEXIDevice[i], 0, wxALL, 5);
		if (i < 2)
			sEXIDevices[i]->Add(GCMemcardPath[i], 0, wxALL, 5);
		sbGamecubeDeviceSettings->Add(sEXIDevices[i]);
	}
	for (int i = 0; i < 4; ++i)
	{
		sSIDevices[i] = new wxBoxSizer(wxHORIZONTAL);
		sSIDevices[i]->Add(GCSIDeviceText[i], 0, wxALIGN_CENTER_VERTICAL|wxALL, 5);
		sSIDevices[i]->Add(GCSIDevice[i], 0, wxALL, 5);
		sbGamecubeDeviceSettings->Add(sSIDevices[i]);
	}

	// Populate the Gamecube page
	sGamecubePage = new wxBoxSizer(wxVERTICAL);
	sGamecubePage->Add(sbGamecubeIPLSettings, 0, wxEXPAND|wxALL, 5);
	sGamecubePage->Add(sbGamecubeDeviceSettings, 0, wxEXPAND|wxALL, 5);

	GamecubePage->SetSizer(sGamecubePage);
	sGamecubePage->Layout();



	// Wii page
	// Wiimote Settings
	sbWiimoteSettings = new wxStaticBoxSizer(wxHORIZONTAL, WiiPage, wxT("Wiimote Settings"));
	wxStaticText* WiiSensBarPosText = new wxStaticText(WiiPage, ID_WII_BT_BAR_TEXT, wxT("Sensor Bar Position:"), wxDefaultPosition, wxDefaultSize);
	WiiSensBarPos = new wxChoice(WiiPage, ID_WII_BT_BAR, wxDefaultPosition, wxDefaultSize, arrayStringFor_WiiSensBarPos, 0, wxDefaultValidator);

	// Misc Settings
	sbWiiIPLSettings = new wxStaticBoxSizer(wxVERTICAL, WiiPage, wxT("Misc Settings"));
	WiiScreenSaver = new wxCheckBox(WiiPage, ID_WII_IPL_SSV, wxT("Enable Screen Saver (burn-in reduction)"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	WiiProgressiveScan = new wxCheckBox(WiiPage, ID_WII_IPL_PGS, wxT("Enable Progressive Scan"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	WiiEuRGB60 = new wxCheckBox(WiiPage, ID_WII_IPL_E60, wxT("Use EuRGB60 Mode (PAL60)"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	wxStaticText* WiiAspectRatioText = new wxStaticText(WiiPage, ID_WII_IPL_AR_TEXT, wxT("Aspect Ratio:"), wxDefaultPosition, wxDefaultSize);
	WiiAspectRatio = new wxChoice(WiiPage, ID_WII_IPL_AR, wxDefaultPosition, wxDefaultSize, arrayStringFor_WiiAspectRatio, 0, wxDefaultValidator);
	wxStaticText* WiiSystemLangText = new wxStaticText(WiiPage, ID_WII_IPL_LNG_TEXT, wxT("System Language:"), wxDefaultPosition, wxDefaultSize);
	WiiSystemLang = new wxChoice(WiiPage, ID_WII_IPL_LNG, wxDefaultPosition, wxDefaultSize, arrayStringFor_WiiSystemLang, 0, wxDefaultValidator);

	// Device Settings
	sbWiiDeviceSettings = new wxStaticBoxSizer(wxVERTICAL, WiiPage, wxT("Device Settings"));
	WiiSDCard = new wxCheckBox(WiiPage, ID_WII_SD_CARD, wxT("Insert SD Card"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	WiiKeyboard = new wxCheckBox(WiiPage, ID_WII_KEYBOARD, wxT("Connect USB Keyboard"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);

	// Populate the settings
	sWiimoteSettings = new wxGridBagSizer();
	sWiimoteSettings->Add(WiiSensBarPosText, wxGBPosition(0, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL|wxALL, 5);
	sWiimoteSettings->Add(WiiSensBarPos, wxGBPosition(0, 1), wxDefaultSpan, wxALL, 5);
	sbWiimoteSettings->Add(sWiimoteSettings);

	sWiiIPLSettings = new wxGridBagSizer();
	sWiiIPLSettings->Add(WiiScreenSaver, wxGBPosition(0, 0), wxGBSpan(1, 2), wxALL, 5);
	sWiiIPLSettings->Add(WiiProgressiveScan, wxGBPosition(1, 0), wxGBSpan(1, 2), wxALL, 5);
	sWiiIPLSettings->Add(WiiEuRGB60, wxGBPosition(2, 0), wxGBSpan(1, 2), wxALL, 5);
	sWiiIPLSettings->Add(WiiAspectRatioText, wxGBPosition(3, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL|wxALL, 5);
	sWiiIPLSettings->Add(WiiAspectRatio, wxGBPosition(3, 1), wxDefaultSpan, wxALL, 5);
	sWiiIPLSettings->Add(WiiSystemLangText, wxGBPosition(4, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL|wxALL, 5);
	sWiiIPLSettings->Add(WiiSystemLang, wxGBPosition(4, 1), wxDefaultSpan, wxALL, 5);
	sbWiiIPLSettings->Add(sWiiIPLSettings);

	sbWiiDeviceSettings->Add(WiiSDCard, 0, wxALL, 5);
	sbWiiDeviceSettings->Add(WiiKeyboard, 0, wxALL, 5);

	// Populate the Wii page
	sWiiPage = new wxBoxSizer(wxVERTICAL);
	sWiiPage->Add(sbWiimoteSettings, 0, wxEXPAND|wxALL, 5);
	sWiiPage->Add(sbWiiIPLSettings, 0, wxEXPAND|wxALL, 5);
	sWiiPage->Add(sbWiiDeviceSettings, 0, wxEXPAND|wxALL, 5);

	WiiPage->SetSizer(sWiiPage);
	sWiiPage->Layout();


	
	// Paths page
	sbISOPaths = new wxStaticBoxSizer(wxVERTICAL, PathsPage, wxT("ISO Directories"));
	ISOPaths = new wxListBox(PathsPage, ID_ISOPATHS, wxDefaultPosition, wxDefaultSize, arrayStringFor_ISOPaths, wxLB_SINGLE, wxDefaultValidator);
	RecursiveISOPath = new wxCheckBox(PathsPage, ID_RECURSIVEISOPATH, wxT("Search Subfolders"), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);
	AddISOPath = new wxButton(PathsPage, ID_ADDISOPATH, wxT("Add..."), wxDefaultPosition, wxDefaultSize, 0);
	RemoveISOPath = new wxButton(PathsPage, ID_REMOVEISOPATH, wxT("Remove"), wxDefaultPosition, wxDefaultSize, 0);
	RemoveISOPath->Enable(false);

	wxStaticText* DefaultISOText = new wxStaticText(PathsPage, ID_DEFAULTISO_TEXT, wxT("Default ISO:"), wxDefaultPosition, wxDefaultSize);
	DefaultISO = new wxFilePickerCtrl(PathsPage, ID_DEFAULTISO, wxEmptyString, wxT("Choose a default ISO:"),
		wxString::Format(wxT("All GC/Wii images (gcm, iso, gcz)|*.gcm;*.iso;*.gcz|All files (%s)|%s"), wxFileSelectorDefaultWildcardStr, wxFileSelectorDefaultWildcardStr),
		wxDefaultPosition, wxDefaultSize, wxFLP_USE_TEXTCTRL|wxFLP_OPEN);
	wxStaticText* DVDRootText = new wxStaticText(PathsPage, ID_DVDROOT_TEXT, wxT("DVD Root:"), wxDefaultPosition, wxDefaultSize);
	DVDRoot = new wxDirPickerCtrl(PathsPage, ID_DVDROOT, wxEmptyString, wxT("Choose a DVD root directory:"), wxDefaultPosition, wxDefaultSize, wxDIRP_USE_TEXTCTRL);
	wxStaticText* ApploaderPathText = new wxStaticText(PathsPage, ID_APPLOADERPATH_TEXT, wxT("Apploader:"), wxDefaultPosition, wxDefaultSize);
	ApploaderPath = new wxFilePickerCtrl(PathsPage, ID_APPLOADERPATH, wxEmptyString, wxT("Choose file to use as apploader: (applies to discs constructed from directories only)"),
		wxString::Format(wxT("apploader (.img)|*.img|All files (%s)|%s"), wxFileSelectorDefaultWildcardStr, wxFileSelectorDefaultWildcardStr),
		wxDefaultPosition, wxDefaultSize, wxFLP_USE_TEXTCTRL|wxFLP_OPEN);

	// Populate the settings
	sbISOPaths->Add(ISOPaths, 1, wxEXPAND|wxALL, 0);
	wxBoxSizer* sISOButtons = new wxBoxSizer(wxHORIZONTAL);
	sISOButtons->Add(RecursiveISOPath, 0, wxALL|wxALIGN_CENTER, 0);
	sISOButtons->AddStretchSpacer();
	sISOButtons->Add(AddISOPath, 0, wxALL, 0);
	sISOButtons->Add(RemoveISOPath, 0, wxALL, 0);
	sbISOPaths->Add(sISOButtons, 0, wxEXPAND|wxALL, 5);

	sOtherPaths = new wxGridBagSizer();
	sOtherPaths->Add(DefaultISOText, wxGBPosition(0, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL|wxALL, 5);
	sOtherPaths->Add(DefaultISO, wxGBPosition(0, 1), wxDefaultSpan, wxEXPAND|wxALL, 5);
	sOtherPaths->Add(DVDRootText, wxGBPosition(1, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL|wxALL, 5);
	sOtherPaths->Add(DVDRoot, wxGBPosition(1, 1), wxDefaultSpan, wxEXPAND|wxALL, 5);
	sOtherPaths->Add(ApploaderPathText, wxGBPosition(2, 0), wxDefaultSpan, wxALIGN_CENTER_VERTICAL|wxALL, 5);
	sOtherPaths->Add(ApploaderPath, wxGBPosition(2, 1), wxDefaultSpan, wxEXPAND|wxALL, 5);
	sOtherPaths->AddGrowableCol(1);

	// Populate the Paths page
	sPathsPage = new wxBoxSizer(wxVERTICAL);
	sPathsPage->Add(sbISOPaths, 1, wxEXPAND|wxALL, 5);
	sPathsPage->Add(sOtherPaths, 0, wxEXPAND|wxALL, 5);

	PathsPage->SetSizer(sPathsPage);
	sPathsPage->Layout();


	
	// Plugins page
	sbGraphicsPlugin = new wxStaticBoxSizer(wxHORIZONTAL, PluginsPage, wxT("Graphics"));
	GraphicSelection = new wxChoice(PluginsPage, ID_GRAPHIC_CB, wxDefaultPosition, wxDefaultSize, NULL, 0, wxDefaultValidator);
	GraphicConfig = new wxButton(PluginsPage, ID_GRAPHIC_CONFIG, wxT("Config..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);

	sbDSPPlugin = new wxStaticBoxSizer(wxHORIZONTAL, PluginsPage, wxT("DSP"));
	DSPSelection = new wxChoice(PluginsPage, ID_DSP_CB, wxDefaultPosition, wxDefaultSize, NULL, 0, wxDefaultValidator);
	DSPConfig = new wxButton(PluginsPage, ID_DSP_CONFIG, wxT("Config..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);

	sbWiimotePlugin = new wxStaticBoxSizer(wxHORIZONTAL, PluginsPage, wxT("Wiimote"));
	WiimoteSelection = new wxChoice(PluginsPage, ID_WIIMOTE_CB, wxDefaultPosition, wxDefaultSize, NULL, 0, wxDefaultValidator);
	WiimoteConfig = new wxButton(PluginsPage, ID_WIIMOTE_CONFIG, wxT("Config..."), wxDefaultPosition, wxDefaultSize, 0, wxDefaultValidator);

	// Populate the settings
	sbGraphicsPlugin->Add(GraphicSelection, 1, wxEXPAND|wxALL, 5);
	sbGraphicsPlugin->Add(GraphicConfig, 0, wxALL, 5);

	sbDSPPlugin->Add(DSPSelection, 1, wxEXPAND|wxALL, 5);
	sbDSPPlugin->Add(DSPConfig, 0, wxALL, 5);

	sbWiimotePlugin->Add(WiimoteSelection, 1, wxEXPAND|wxALL, 5);
	sbWiimotePlugin->Add(WiimoteConfig, 0, wxALL, 5);

	// Populate the Plugins page
	sPluginsPage = new wxBoxSizer(wxVERTICAL);
	sPluginsPage->Add(sbGraphicsPlugin, 0, wxEXPAND|wxALL, 5);
	sPluginsPage->Add(sbDSPPlugin, 0, wxEXPAND|wxALL, 5);
	sPluginsPage->Add(sbWiimotePlugin, 0, wxEXPAND|wxALL, 5);

	PluginsPage->SetSizer(sPluginsPage);
	sPluginsPage->Layout();

	m_Ok = new wxButton(this, wxID_OK);

	wxBoxSizer* sButtons = new wxBoxSizer(wxHORIZONTAL);
	sButtons->Add(0, 0, 1, wxEXPAND, 5);
	sButtons->Add(m_Ok, 0, wxALL, 5);

	wxBoxSizer* sMain = new wxBoxSizer(wxVERTICAL);
	sMain->Add(Notebook, 1, wxEXPAND|wxALL, 5);
	sMain->Add(sButtons, 0, wxEXPAND, 5);

	InitializeGUIValues();
	InitializeGUITooltips();

	UpdateGUI();

	SetSizer(sMain);
	Layout();

	Fit();
	Center();
}

void CConfigMain::OnClose(wxCloseEvent& WXUNUSED (event))
{
	EndModal((bRefreshList) ? wxID_OK : wxID_CLOSE);
}

void CConfigMain::OnOk(wxCommandEvent& WXUNUSED (event))
{
	Close();
	// Sysconf saves when it gets deleted
	//delete SConfig::GetInstance().m_SYSCONF;

	// Save the config. Dolphin crashes to often to save the settings on closing only
	SConfig::GetInstance().SaveSettings();
}

// Core settings
void CConfigMain::CoreSettingsChanged(wxCommandEvent& event)
{
	switch (event.GetId())
	{
	// Core - Basic
	case ID_CPUTHREAD:
		SConfig::GetInstance().m_LocalCoreStartupParameter.bCPUThread = CPUThread->IsChecked();
		break;
	case ID_IDLESKIP:
		SConfig::GetInstance().m_LocalCoreStartupParameter.bSkipIdle = SkipIdle->IsChecked();
		break;
	case ID_ENABLECHEATS:
		SConfig::GetInstance().m_LocalCoreStartupParameter.bEnableCheats = EnableCheats->IsChecked();
		break;
	case ID_FRAMELIMIT:
		SConfig::GetInstance().m_Framelimit = Framelimit->GetSelection();
		break;
	case ID_FRAMELIMIT_USEFPSFORLIMITING:
		SConfig::GetInstance().b_UseFPS = UseFPSForLimiting->IsChecked();
		break;
	// Core - Advanced
	case ID_ALWAYS_HLE_BS2:
		SConfig::GetInstance().m_LocalCoreStartupParameter.bHLE_BS2 = AlwaysHLE_BS2->IsChecked();
		break;
	case ID_ENABLE_OPENCL:
		SConfig::GetInstance().m_LocalCoreStartupParameter.bEnableOpenCL = EnableOpenCL->IsChecked();
		break;
	case ID_CPUENGINE:
		SConfig::GetInstance().m_LocalCoreStartupParameter.iCPUCore = CPUEngine->GetSelection();
		if (main_frame->g_pCodeWindow)
			main_frame->g_pCodeWindow->GetMenuBar()->Check(IDM_INTERPRETER,
				SConfig::GetInstance().m_LocalCoreStartupParameter.iCPUCore?false:true);
		break;
	case ID_DSPTHREAD:
		SConfig::GetInstance().m_LocalCoreStartupParameter.bDSPThread = DSPThread->IsChecked();
		break;
	case ID_LOCKTHREADS:
		SConfig::GetInstance().m_LocalCoreStartupParameter.bLockThreads = LockThreads->IsChecked();
		break;
	}
}

// Display and Interface settings
void CConfigMain::DisplaySettingsChanged(wxCommandEvent& event)
{
	switch (event.GetId())
	{
	// Display - Display
	case ID_DISPLAY_FULLSCREENRES:
		SConfig::GetInstance().m_LocalCoreStartupParameter.strFullscreenResolution =
			FullscreenResolution->GetStringSelection().mb_str();
#if defined(HAVE_XRANDR) && HAVE_XRANDR
		main_frame->m_XRRConfig->Update();
#endif
		break;
	case ID_DISPLAY_WINDOWWIDTH:
		SConfig::GetInstance().m_LocalCoreStartupParameter.iRenderWindowWidth = WindowWidth->GetValue();
		break;
	case ID_DISPLAY_WINDOWHEIGHT:
		SConfig::GetInstance().m_LocalCoreStartupParameter.iRenderWindowHeight = WindowHeight->GetValue();
		break;
	case ID_DISPLAY_FULLSCREEN:
		SConfig::GetInstance().m_LocalCoreStartupParameter.bFullscreen = Fullscreen->IsChecked();
		break;
	case ID_DISPLAY_HIDECURSOR:
		SConfig::GetInstance().m_LocalCoreStartupParameter.bHideCursor = HideCursor->IsChecked();
		break;
	case ID_DISPLAY_RENDERTOMAIN:
		SConfig::GetInstance().m_LocalCoreStartupParameter.bRenderToMain = RenderToMain->IsChecked();
		break;
	// Display - Interface
	case ID_INTERFACE_CONFIRMSTOP:
		SConfig::GetInstance().m_LocalCoreStartupParameter.bConfirmStop = ConfirmStop->IsChecked();
		break;
	case ID_INTERFACE_USEPANICHANDLERS:
		SConfig::GetInstance().m_LocalCoreStartupParameter.bUsePanicHandlers = UsePanicHandlers->IsChecked();
		SetEnableAlert(UsePanicHandlers->IsChecked());
		break;
	case ID_INTERFACE_THEME:
		SConfig::GetInstance().m_LocalCoreStartupParameter.iTheme = Theme->GetSelection();
		main_frame->InitBitmaps();
		break;
	case ID_INTERFACE_LANG:
		SConfig::GetInstance().m_InterfaceLanguage = (INTERFACE_LANGUAGE)InterfaceLang->GetSelection();
		bRefreshList = true;
		break;
	case ID_HOTKEY_CONFIG:
		{
			HotkeyConfigDialog *m_HotkeyDialog = new HotkeyConfigDialog(this);
			m_HotkeyDialog->ShowModal();
			m_HotkeyDialog->Destroy();
			// Update the GUI in case menu accelerators were changed
			main_frame->UpdateGUI();
		}
		break;
	}
}


// GC settings
// -----------------------
void CConfigMain::GCSettingsChanged(wxCommandEvent& event)
{
	int sidevice = 0;
	int exidevice = 0;
	switch (event.GetId())
	{
	// Gamecube - IPL
	case ID_GC_SRAM_LNG:
		SConfig::GetInstance().m_LocalCoreStartupParameter.SelectedLanguage = GCSystemLang->GetSelection();
		break;
	// Gamecube - Devices
	case ID_GC_EXIDEVICE_SP1:
		exidevice++;
	case ID_GC_EXIDEVICE_SLOTB:
		exidevice++;
	case ID_GC_EXIDEVICE_SLOTA:
		ChooseEXIDevice(std::string(event.GetString().mb_str()), exidevice);
		break;
	case ID_GC_EXIDEVICE_SLOTA_PATH:
		ChooseMemcardPath(SConfig::GetInstance().m_strMemoryCardA, true);
		break;
	case ID_GC_EXIDEVICE_SLOTB_PATH:
		ChooseMemcardPath(SConfig::GetInstance().m_strMemoryCardB, false);
		break;
	case ID_GC_SIDEVICE3:
		sidevice++;
	case ID_GC_SIDEVICE2:
		sidevice++;
	case ID_GC_SIDEVICE1:
		sidevice++;
	case ID_GC_SIDEVICE0:
		ChooseSIDevice(std::string(event.GetString().mb_str()), sidevice);
		break;
	}
}

void CConfigMain::ChooseMemcardPath(std::string& strMemcard, bool isSlotA)
{
	std::string filename = std::string(wxFileSelector(
		wxT("Choose a file to open"),
		wxString::From8BitData(File::GetUserPath(D_GCUSER_IDX)),
		isSlotA ? wxT(GC_MEMCARDA) : wxT(GC_MEMCARDB),
		wxEmptyString,
		wxT("Gamecube Memory Cards (*.raw,*.gcp)|*.raw;*.gcp")).mb_str());

	if (!filename.empty())
	{
		// also check that the path isn't used for the other memcard...
		if (filename.compare(isSlotA ? SConfig::GetInstance().m_strMemoryCardB
		: SConfig::GetInstance().m_strMemoryCardA) != 0)
		{
			strMemcard = filename;

			if (Core::GetState() != Core::CORE_UNINITIALIZED)
			{
				// Change memcard to the new file
				ExpansionInterface::ChangeDevice(
					isSlotA ? 0 : 1, // SlotA: channel 0, SlotB channel 1
					isSlotA ? EXIDEVICE_MEMORYCARD_A : EXIDEVICE_MEMORYCARD_B,
					0);	// SP1 is device 2, slots are device 0
			}
		}
		else
		{
			PanicAlert("Cannot use that file as a memory card.\n"
				"Are you trying to use the same file in both slots?");
		}
	}
}

void CConfigMain::ChooseSIDevice(std::string deviceName, int deviceNum)
{
	TSIDevices tempType;
	if (!deviceName.compare(SIDEV_STDCONT_STR))
		tempType = SI_GC_CONTROLLER;
	else if (!deviceName.compare(SIDEV_GBA_STR))
		tempType = SI_GBA;
	else if (!deviceName.compare(SIDEV_AM_BB_STR))
		tempType = SI_AM_BASEBOARD;
	else
		tempType = SI_NONE;

	SConfig::GetInstance().m_SIDevice[deviceNum] = tempType;

	if (Core::GetState() != Core::CORE_UNINITIALIZED)
	{
		// Change plugged device! :D
		SerialInterface::ChangeDevice(tempType, deviceNum);
	}
}

void CConfigMain::ChooseEXIDevice(std::string deviceName, int deviceNum)
{
	TEXIDevices tempType;

	if (!deviceName.compare(EXIDEV_MEMCARD_STR))
		tempType = deviceNum ? EXIDEVICE_MEMORYCARD_B : EXIDEVICE_MEMORYCARD_A;
	else if (!deviceName.compare(EXIDEV_MIC_STR))
		tempType = EXIDEVICE_MIC;
	else if (!deviceName.compare(EXIDEV_BBA_STR))
		tempType = EXIDEVICE_ETH;
	else if (!deviceName.compare(EXIDEV_AM_BB_STR))
		tempType = EXIDEVICE_AM_BASEBOARD;
	else if (!deviceName.compare(DEV_NONE_STR))
		tempType = EXIDEVICE_NONE;
	else
		tempType = EXIDEVICE_DUMMY;

	// Gray out the memcard path button if we're not on a memcard
	if (tempType == EXIDEVICE_MEMORYCARD_A || tempType == EXIDEVICE_MEMORYCARD_B)
		GCMemcardPath[deviceNum]->Enable();
	else if (deviceNum == 0 || deviceNum == 1)
		GCMemcardPath[deviceNum]->Disable();

	SConfig::GetInstance().m_EXIDevice[deviceNum] = tempType;

	if (Core::GetState() != Core::CORE_UNINITIALIZED)
	{
		// Change plugged device! :D
		ExpansionInterface::ChangeDevice(
			(deviceNum == 1) ? 1 : 0,	// SlotB is on channel 1, slotA and SP1 are on 0
			tempType,					// The device enum to change to
			(deviceNum == 2) ? 2 : 0);	// SP1 is device 2, slots are device 0
	}
}




// Wii settings
// -------------------
void CConfigMain::WiiSettingsChanged(wxCommandEvent& event)
{
	switch (event.GetId())
	{
	// Wii - Wiimote settings
	case ID_WII_BT_BAR:
		SConfig::GetInstance().m_SYSCONF->SetData("BT.BAR", WiiSensBarPos->GetSelection());
		break;
	// SYSCONF settings
	case ID_WII_IPL_SSV:
		SConfig::GetInstance().m_SYSCONF->SetData("IPL.SSV", WiiScreenSaver->IsChecked());
		break;
	case ID_WII_IPL_PGS:
		SConfig::GetInstance().m_SYSCONF->SetData("IPL.PGS", WiiProgressiveScan->IsChecked());
		break;
	case ID_WII_IPL_E60:
		SConfig::GetInstance().m_SYSCONF->SetData("IPL.E60", WiiEuRGB60->IsChecked());
		break;
	case ID_WII_IPL_AR:
		SConfig::GetInstance().m_SYSCONF->SetData("IPL.AR", WiiAspectRatio->GetSelection());
		break;
	case ID_WII_IPL_LNG:
		SConfig::GetInstance().m_SYSCONF->SetData("IPL.LNG", WiiSystemLang->GetSelection());
		break;
	// Wii - Devices
	case ID_WII_SD_CARD:
		SConfig::GetInstance().m_WiiSDCard = WiiSDCard->IsChecked();
		break;
	case ID_WII_KEYBOARD:
		SConfig::GetInstance().m_WiiKeyboard = WiiKeyboard->IsChecked();
		break;
	}
}





// Paths settings
// -------------------
void CConfigMain::ISOPathsSelectionChanged(wxCommandEvent& WXUNUSED (event))
{
	if (!ISOPaths->GetStringSelection().empty())
	{
		RemoveISOPath->Enable(true);
	}
	else
	{
		RemoveISOPath->Enable(false);
	}
}

void CConfigMain::AddRemoveISOPaths(wxCommandEvent& event)
{
	if (event.GetId() == ID_ADDISOPATH)
	{
		wxString dirHome;
		wxGetHomeDir(&dirHome);

		wxDirDialog dialog(this, _T("Choose a directory to add"), dirHome, wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

		if (dialog.ShowModal() == wxID_OK)
		{
			if (ISOPaths->FindString(dialog.GetPath()) != -1)
			{
				wxMessageBox(_("The chosen directory is already in the list"), _("Error"), wxOK);
			}
			else
			{
				bRefreshList = true;
				ISOPaths->Append(dialog.GetPath());
			}
		}
	}
	else
	{
		bRefreshList = true;
		ISOPaths->Delete(ISOPaths->GetSelection());
	}

	// Save changes right away
	SConfig::GetInstance().m_ISOFolder.clear();

	for (unsigned int i = 0; i < ISOPaths->GetCount(); i++)
		SConfig::GetInstance().m_ISOFolder.push_back(std::string(ISOPaths->GetStrings()[i].mb_str()));
}

void CConfigMain::RecursiveDirectoryChanged(wxCommandEvent& WXUNUSED (event))
{
	SConfig::GetInstance().m_RecursiveISOFolder = RecursiveISOPath->IsChecked();
	bRefreshList = true;
}

void CConfigMain::DefaultISOChanged(wxFileDirPickerEvent& WXUNUSED (event))
{
	SConfig::GetInstance().m_LocalCoreStartupParameter.m_strDefaultGCM = DefaultISO->GetPath().mb_str();
}

void CConfigMain::DVDRootChanged(wxFileDirPickerEvent& WXUNUSED (event))
{
	SConfig::GetInstance().m_LocalCoreStartupParameter.m_strDVDRoot = DVDRoot->GetPath().mb_str();
}

void CConfigMain::ApploaderPathChanged(wxFileDirPickerEvent& WXUNUSED (event))
{
	SConfig::GetInstance().m_LocalCoreStartupParameter.m_strApploader = ApploaderPath->GetPath().mb_str();
}


// Plugin settings
void CConfigMain::OnSelectionChanged(wxCommandEvent& WXUNUSED (event))
{
	// Update plugin filenames
	GetFilename(GraphicSelection, SConfig::GetInstance().m_LocalCoreStartupParameter.m_strVideoPlugin);
	GetFilename(DSPSelection, SConfig::GetInstance().m_LocalCoreStartupParameter.m_strDSPPlugin);
	GetFilename(WiimoteSelection, SConfig::GetInstance().m_LocalCoreStartupParameter.m_strWiimotePlugin);
}

void CConfigMain::OnConfig(wxCommandEvent& event)
{
	switch (event.GetId())
	{
		case ID_GRAPHIC_CONFIG:
			CallConfig(GraphicSelection);
			break;
		case ID_DSP_CONFIG:
			CallConfig(DSPSelection);
			break;
		case ID_WIIMOTE_CONFIG:
			CallConfig(WiimoteSelection);
			break;
	}
}

void CConfigMain::CallConfig(wxChoice* _pChoice)
{
	int Index = _pChoice->GetSelection();
	INFO_LOG(CONSOLE, "CallConfig: %i\n", Index);
	if (Index >= 0)
	{
		const CPluginInfo* pInfo = static_cast<CPluginInfo*>(_pChoice->GetClientData(Index));
		if (pInfo != NULL)
			CPluginManager::GetInstance().OpenConfig((HWND) this->GetHandle(), pInfo->GetFilename().c_str(), pInfo->GetPluginInfo().Type);
	}
}

void CConfigMain::FillChoiceBox(wxChoice* _pChoice, int _PluginType, const std::string& _SelectFilename)
{
	_pChoice->Clear();

	int Index = -1;
	const CPluginInfos& rInfos = CPluginManager::GetInstance().GetPluginInfos();

	for (size_t i = 0; i < rInfos.size(); i++)
	{
		const PLUGIN_INFO& rPluginInfo = rInfos[i].GetPluginInfo();

		if (rPluginInfo.Type == _PluginType)
		{
			wxString temp;
			temp = wxString::FromAscii(rInfos[i].GetPluginInfo().Name);
			int NewIndex = _pChoice->Append(temp, (void*)&rInfos[i]);

			if (rInfos[i].GetFilename() == _SelectFilename)
			{
				Index = NewIndex;
			}
		}
	}

	_pChoice->Select(Index);
}

bool CConfigMain::GetFilename(wxChoice* _pChoice, std::string& _rFilename)
{	
	_rFilename.clear();
	int Index = _pChoice->GetSelection();
	if (Index >= 0)
	{
		const CPluginInfo* pInfo = static_cast<CPluginInfo*>(_pChoice->GetClientData(Index));
		_rFilename = pInfo->GetFilename();
		INFO_LOG(CONSOLE, "GetFilename: %i %s\n", Index, _rFilename.c_str());
		return(true);
	}

	return(false);
}

// Search for avaliable resolutions
void CConfigMain::AddResolutions()
{
#ifdef _WIN32
	DWORD iModeNum = 0;
	DEVMODE dmi;
	ZeroMemory(&dmi, sizeof(dmi));
	dmi.dmSize = sizeof(dmi);
	std::vector<std::string> resos;

	while (EnumDisplaySettings(NULL, iModeNum++, &dmi) != 0)
	{
		char res[100];
		sprintf(res, "%dx%d", dmi.dmPelsWidth, dmi.dmPelsHeight);
		std::string strRes(res);
		// Only add unique resolutions
		if (std::find(resos.begin(), resos.end(), strRes) == resos.end())
		{
			resos.push_back(strRes);
			arrayStringFor_FullscreenResolution.Add(wxString::FromAscii(res));
		}
		ZeroMemory(&dmi, sizeof(dmi));
	}
#elif defined(HAVE_XRANDR) && HAVE_XRANDR
	main_frame->m_XRRConfig->AddResolutions(arrayStringFor_FullscreenResolution);
#elif defined(__APPLE__)
	CFDictionaryRef			mode;
	CFArrayRef			array;
	CFIndex				n, i;
	int				w, h;
	std::vector<std::string>	resos;
	
	array = CGDisplayAvailableModes(CGMainDisplayID());
	n = CFArrayGetCount(array);
	
	for (i = 0; i < n; i++)
	{
		mode	= (CFDictionaryRef)CFArrayGetValueAtIndex(array, i);
		
		CFNumberRef anWidth = (CFNumberRef)CFDictionaryGetValue(mode,
			kCGDisplayWidth);
		if (NULL == anWidth ||
			!CFNumberGetValue(anWidth, kCFNumberIntType, &w))
			continue;

		CFNumberRef anHeight =
			(CFNumberRef)CFDictionaryGetValue(mode,
				kCGDisplayHeight);
		if (NULL == anHeight ||
			!CFNumberGetValue(anHeight, kCFNumberIntType, &h))
			continue;
		
		char res[32];
		sprintf(res,"%dx%d", w, h);
		std::string strRes(res);

		// Only add unique resolutions
		if (std::find(resos.begin(), resos.end(), strRes) ==
			resos.end())
		{
			resos.push_back(strRes);
			arrayStringFor_FullscreenResolution.Add(strRes);
		}
	}
#endif
}
