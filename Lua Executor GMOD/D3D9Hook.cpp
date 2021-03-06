#include "D3D9Hook.h"

#define INRANGE(x,a,b)    (x >= a && x <= b) 
#define getBits( x )    (INRANGE((x&(~0x20)),'A','F') ? ((x&(~0x20)) - 'A' + 0xa) : (INRANGE(x,'0','9') ? x - '0' : 0))
#define getByte( x )    (getBits(x[0]) << 4 | getBits(x[1]))

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
typedef HRESULT(__stdcall * EndScene)(IDirect3DDevice9* pDevice);
typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);

EndScene oEndScene;
WNDPROC oWndProc;

static char* errorMsg;

DWORD FindPattern(std::string moduleName, std::string pattern)
{
	const char* pat = pattern.c_str();
	DWORD firstMatch = 0;
	DWORD rangeStart = (DWORD)GetModuleHandleA(moduleName.c_str());
	MODULEINFO miModInfo; GetModuleInformation(GetCurrentProcess(), (HMODULE)rangeStart, &miModInfo, sizeof(MODULEINFO));
	DWORD rangeEnd = rangeStart + miModInfo.SizeOfImage;
	for (DWORD pCur = rangeStart; pCur < rangeEnd; pCur++)
	{
		if (!*pat)
			return firstMatch;

		if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == getByte(pat))
		{
			if (!firstMatch)
				firstMatch = pCur;

			if (!pat[2])
				return firstMatch;

			if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?')
				pat += 3;

			else
				pat += 2;    //one ?
		}
		else
		{
			pat = pattern.c_str();
			firstMatch = 0;
		}
	}
	return NULL;
}

void CustomTheme()
{
	ImGuiStyle * style = &ImGui::GetStyle();
	ImGuiIO &io = ImGui::GetIO();

	style->Colors[ImGuiCol_Text] = ImVec4(1.f, 1.f, 1.f, 1.f);
	style->Colors[ImGuiCol_Button] = ImVec4(0.2f, 0.69f, 0.22f, 1.f);
	style->Colors[ImGuiCol_ButtonActive] = ImVec4(0.2f, 0.69f, 0.22f, 1.f);
	style->Colors[ImGuiCol_ButtonHovered] = ImVec4(0.2f, 0.0f, 0.22f, 1.f);
	style->Colors[ImGuiCol_TitleBg] = ImVec4(0.2f, 0.69f, 0.22f, 1.f);
	style->Colors[ImGuiCol_TitleBgActive] = ImVec4(0.2f, 0.69f, 0.22f, 1.f);
	style->Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1f, 0.1f, 1.f);
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.4f, 0.4f, 0.4f, 1.f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.2f, 0.69f, 0.22f, 1.f);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.2f, 0.69f, 0.22f, 1.f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.2f, 0.69f, 0.22f, 1.f);

	io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\Ruda-Bold.ttf", 16);
}

HRESULT __stdcall Hooked_EndScene(IDirect3DDevice9 * pDevice)
{
	static bool init = true;
	static std::string filePath;
	static TextEditor editor;
	static ImGui::FileBrowser fileDialog;
	if (init)
	{
		init = false;
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();

		ImGui_ImplWin32_Init(FindWindowA(NULL, "Garry's Mod"));
		ImGui_ImplDX9_Init(pDevice);
	
		editor.SetLanguageDefinition(TextEditor::LanguageDefinition::Lua());

		TextEditor::ErrorMarkers markers;
		markers.insert(std::make_pair<int, std::string>(6, "Example error here:\nInclude file not found: \"TextEditor.h\""));
		markers.insert(std::make_pair<int, std::string>(41, "Another example error"));
		editor.SetErrorMarkers(markers);

		fileDialog.SetTitle("Look for lua files!");
		fileDialog.SetTypeFilters({ ".lua", ".txt" });
		filePath = "Example: Drive\testscript.lua";

		CustomTheme();
	}

	if (srcD3D9Hook->GetMenuOpen())
	{
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Lua Executor ~ By I SUCK AT LIFE#7728", 0, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize);
	
		editor.Render("Lua Script Editor", ImVec2(600, 400), true);

		ImGui::Spacing();

		if (ImGui::Button("Execute Script"))
		{
			GLuaExecutor->ExecuteString((char*)editor.GetText().c_str());
			errorMsg = (char*)"String successfully executed!";
		}

		ImGui::SameLine();

		if (ImGui::Button("Execute File Script"))
		{
			if (!std::filesystem::exists(filePath))
			{
				errorMsg = (char*)"File does not exist!";
			}
			else {
				GLuaExecutor->ExecuteFile((char*)filePath.c_str());
				errorMsg = (char*)"File successfully executed!";
			}
		}

		ImGui::SameLine();

		if (ImGui::Button("Fix Lua Executor!"))
		{
			GLuaExecutor->InitLuaExecutor();
		}

		if (ImGui::Button("Search For Files"))
		{
			fileDialog.Open();
		}

		ImGui::SameLine();

		ImGui::InputText("File Path (FULL)", (char*)filePath.c_str(), sizeof(std::string));

		ImGui::Text(errorMsg);

		ImGui::End();

		fileDialog.Display();
		
		if (fileDialog.HasSelected())
		{
			filePath = fileDialog.GetSelected().string();
			fileDialog.ClearSelected();
		}

		ImGui::EndFrame();
		ImGui::Render();

		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}


	return oEndScene(pDevice);
}

LRESULT __stdcall WndProc(const HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (srcD3D9Hook->GetMenuOpen())
	{
		ImGui_ImplWin32_WndProcHandler(hwnd, uMsg, wParam, lParam);
		return true;
	}

	return CallWindowProc(oWndProc, hwnd, uMsg, wParam, lParam);
}

void D3D9Hook::KeyHandleThread()
{
	if (GetAsyncKeyState(VK_INSERT) & 0x0001) ToggleMenuOpen();
}

DWORD __stdcall D3D9Hook::stKeyHandleThread()
{
	while (true)
	{
		srcD3D9Hook->KeyHandleThread();
		Sleep(1);
	}

	return 0;
}

void D3D9Hook::InitializeHook()
{
	oWndProc = (WNDPROC)SetWindowLongPtr(FindWindow(NULL, "Garry's Mod"), -4, (LONG_PTR)WndProc);

	static DWORD DirectXDevice = NULL;

	while (!DirectXDevice)
		DirectXDevice = **(DWORD**)(FindPattern("shaderapidx9.dll", "A1 ?? ?? ?? ?? 50 8B 08 FF 51 0C") + 0x1);

	void ** pVTable = *reinterpret_cast<void***>(DirectXDevice);
	oEndScene = (EndScene)DetourFunction((PBYTE)pVTable[42], (PBYTE)Hooked_EndScene);

	errorMsg = (char*)"DirectX hooked successfully!";
}

void D3D9Hook::ToggleMenuOpen()
{
	menuOpen = !menuOpen;
}

bool D3D9Hook::GetMenuOpen()
{
	return menuOpen;
}
