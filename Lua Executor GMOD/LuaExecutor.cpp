#include "LuaExecutor.h"

#define LUA_MULTRET -1

void LuaExecutor::InitLuaExecutor()
{
	printf("Initializing Lua Functions...\n");
	HMODULE luaShared = GetModuleHandleA("lua_shared.dll");

	if (!luaShared)
	{
		printf("Error, Failed to get access to lua_shared.lua!\n");
		return;
	}
	printf("Success!\n");

	printf("Loading Interface function...\n");
	CreateInterface CreateInterface_src = (CreateInterface)GetProcAddress(luaShared, "CreateInterface");
	ILuaShared = (CLuaShared*)CreateInterface_src("LUASHARED003", NULL);
	printf("Success!\n");

	printf("Loading LuaL_LoadFile...\n");
	luaL_loadFile = (_luaL_loadFile)GetProcAddress(luaShared, "luaL_loadfile");

	if (!luaL_loadFile)
	{
		printf("Error, Failed to get access to luaL_loadfile!\n");
		return;
	}
	printf("Success!\n");

	printf("Loading LuaL_LoadString...\n");
	luaL_loadString = (_luaL_loadString)GetProcAddress(luaShared, "luaL_loadstring");

	if (!luaL_loadString)
	{
		printf("Error, Failed to get access to luaL_loadString!\n");
		return;
	}
	printf("Success!\n");

	printf("Loading Lua_pCall...\n");
	luaL_pCall = (_luaL_pCall)GetProcAddress(luaShared, "lua_pcall");

	if (!luaL_pCall)
	{
		printf("Error, Failed to get access to lua_pcall!\n");
		return;
	}
	printf("Success!\n");

	printf("Initialization success!\n");
	LuaLoaded = true;
}

void LuaExecutor::ExecuteFile(char * fileToExecute)
{
	if (!LuaLoaded)
	{
		MessageBoxA(NULL, "Cant execute file when nothing is set!", "Error", 0);
		return;
	}

	printf("Loading CLuaInterface...\n");
	CLuaInterface = ILuaShared->GetLuaInterface(LUAINTERFACE_CLIENT);

	if (!CLuaInterface)
	{
		printf("Error, Failed to get CLua Interface!\n");
		return;
	}
	printf("Success!\n");

	printf("Loading GLuaState...\n");
	pGLuaState = *(DWORD*)(CLuaInterface + 0x4);

	if (!pGLuaState)
	{
		printf("Error, Failed to get lua_State!\n");
		return;
	}
	printf("Success!\n");

	if (*(DWORD*)(CLuaInterface + 0x2C))
	{
		printf("Error, CLua Interface failed!");
		return;
	}

	luaL_loadFile((void*)pGLuaState, fileToExecute);
	luaL_pCall((void*)pGLuaState, 0, LUA_MULTRET, 0);
}

void LuaExecutor::ExecuteString(char * stringToExecute)
{
	if (!LuaLoaded)
	{
		MessageBoxA(NULL, "Cant execute string when nothing is set!", "Error", 0);
		return;
	}

	printf("Loading CLuaInterface...\n");
	CLuaInterface = ILuaShared->GetLuaInterface(LUAINTERFACE_CLIENT);

	if (!CLuaInterface)
	{
		printf("Error, Failed to get CLua Interface!\n");
		return;
	}
	printf("Success!\n");

	printf("Loading GLuaState...\n");
	pGLuaState = *(DWORD*)(CLuaInterface + 0x4);

	if (!pGLuaState)
	{
		printf("Error, Failed to get lua_State!\n");
		return;
	}
	printf("Success!\n");

	if (*(DWORD*)(CLuaInterface + 0x2C))
	{
		printf("Error, CLua Interface failed!");
		return;
	}

	luaL_loadString((void*)pGLuaState, stringToExecute);
	luaL_pCall((void*)pGLuaState, 0, LUA_MULTRET, 0);
}
