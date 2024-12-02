#pragma once
#include "nvse/PluginAPI.h"
#include "nvse/CommandTable.h"
#include "nvse/GameAPI.h"
#include "nvse/ParamInfos.h"
#include "nvse/ParamInfosNVSE.h"
#include "nvse/GameObjects.h"
#include "nvse/Hooks_DirectInput8Create.h"
#include <string>
#include "ShellNVSE.h"
#include "EventHandlers.h"
#include "GameUI.h"
#include "SafeWrite.h"

IDebugLog		gLog("ShellNVSE.log");
constexpr UInt32 g_PluginVersion = 1;

PluginHandle	g_pluginHandle = kPluginHandle_Invalid;

NVSEMessagingInterface* g_messagingInterface{};
NVSEInterface* g_nvseInterface{};
NVSECommandTableInterface g_cmdTableInterface;

NiCamera* g_mainCamera = nullptr;

//UI 11-15-2023
InterfaceManager* g_interfaceManager;
DIHookControl* g_keyInterface = nullptr;

// RUNTIME = Is not being compiled as a GECK plugin.

NVSEScriptInterface* g_script{};
NVSEStringVarInterface* g_stringInterface{};

NVSEDataInterface* g_dataInterface{};
NVSESerializationInterface* g_serializationInterface{};
NVSEConsoleInterface* g_consoleInterface{};
NVSEEventManagerInterface* g_eventInterface{};

#define WantLambdaFunctions 0 // set to 1 if you want these PluginAPI functions
#if WantLambdaFunctions
_LambdaDeleteAllForScript LambdaDeleteAllForScript{};
_LambdaSaveVariableList LambdaSaveVariableList{};
_LambdaUnsaveVariableList LambdaUnsaveVariableList{};
_IsScriptLambda IsScriptLambda{};
#endif

#define WantScriptFunctions 0 // set to 1 if you want these PluginAPI functions
#if WantScriptFunctions
_HasScriptCommand HasScriptCommand{};
_DecompileScript DecompileScript{};
#endif

NVSEArrayVarInterface* g_arrayInterface{};
NVSEArrayVarInterface g_arrayVar;

bool (*ExtractArgsEx)(COMMAND_ARGS_EX, ...);
bool (*ExtractFormatStringArgs)(UInt32 fmtStringPos, char* buffer, COMMAND_ARGS_EX, UInt32 maxParams, ...);  // From JIP_NVSE.H
NVSEArrayVarInterface* g_arrInterface = nullptr;
NVSEArrayVar* (*CreateArray)(const NVSEArrayElement* data, UInt32 size, Script* callingScript);
NVSEArrayVar* (*CreateStringMap)(const char** keys, const NVSEArrayElement* values, UInt32 size, Script* callingScript);
NVSEArrayVar* (*CreateMap)(const double* keys, const NVSEArrayElement* values, UInt32 size, Script* callingScript);
bool (*AssignArrayResult)(NVSEArrayVar* arr, double* dest);
void (*SetElement)(NVSEArrayVar* arr, const NVSEArrayElement& key, const NVSEArrayElement& value);
void (*AppendElement)(NVSEArrayVar* arr, const NVSEArrayElement& value);
UInt32(*GetArraySize)(NVSEArrayVar* arr);
NVSEArrayVar* (*LookupArrayByID)(UInt32 id);
bool (*GetElement)(NVSEArrayVar* arr, const NVSEArrayElement& key, NVSEArrayElement& outElement);
bool (*GetArrayElements)(NVSEArrayVar* arr, NVSEArrayElement* elements, NVSEArrayElement* keys);

const CommandInfo* (*GetCmdByName)(const char* name);

NVSEStringVarInterface g_strInterface;
bool (*AssignString)(COMMAND_ARGS, const char* newValue);
const char* (*GetStringVar)(UInt32 stringID);
NVSEMessagingInterface* g_msg = nullptr;
NVSEScriptInterface* g_scriptInterface = nullptr;

bool (*FunctionCallScript)(Script* funcScript, TESObjectREFR* callingObj, TESObjectREFR* container, NVSEArrayElement* result, UInt8 numArgs, ...);
bool (*FunctionCallScriptAlt)(Script* funcScript, TESObjectREFR* callingObj, UInt8 numArgs, ...);
Script* (*pCompileScript)(const char* scriptText);
Script* (*pCompileExpression)(const char* scriptText);

ExpressionEvaluatorUtils s_expEvalUtils;

bool IsGamePaused()
{
	bool isMainOrPauseMenuOpen = *(Menu**)0x11DAAC0; // g_startMenu, credits to lStewieAl
	auto* console = ConsoleManager::GetSingleton();

	return isMainOrPauseMenuOpen || console->IsConsoleOpen();
}

// Singletons
TileMenu** g_tileMenuArray = nullptr;
UInt32 g_screenWidth = 0;
UInt32 g_screenHeight = 0;
ActorValueOwner* g_playerAVOwner = nullptr;
DataHandler* g_dataHandler = nullptr;
TESObjectWEAP* g_fistsWeapon = nullptr;
TESObjectREFR* DevKitDummyMarker = nullptr;

/****************
 * Here we include the code + definitions for our script functions,
 * which are packed in header files to avoid lengthening this file.
 * Notice that these files don't require #include statements for globals/macros like ExtractArgsEx.
 * This is because the "fn_.h" files are only used here,
 * and they are included after such globals/macros have been defined.
 ***************/

 // Shortcut macro to register a script command (assigning it an Opcode)............................................................................
#define RegisterScriptCommand(name) nvse->RegisterCommand(&kCommandInfo_ ##name); //Default return type (return a number)
#define REG_CMD(name) nvse->RegisterCommand(&kCommandInfo_##name);  //Short version of RegisterScriptCommand, from JIP.
#define REG_TYPED_CMD(name, type) nvse->RegisterTypedCommand(&kCommandInfo_##name,kRetnType_##type);  //from JG
#define REG_CMD_STR(name) nvse->RegisterTypedCommand(&kCommandInfo_##name, kRetnType_String); //From JIPLN
#define REG_CMD_ARR(name) nvse->RegisterTypedCommand(&kCommandInfo_##name, kRetnType_Array); //From JIPLN
#define REG_CMD_FORM(name) nvse->RegisterTypedCommand(&kCommandInfo_##name, kRetnType_Form);
#define REG_CMD_AMB(name) nvse->RegisterTypedCommand(&kCommandInfo_##name, kRetnType_Ambiguous);
//...................................................................................................................................................

// This is a message handler for nvse events
// With this, plugins can listen to messages such as whenever the game loads
void MessageHandler(NVSEMessagingInterface::Message* msg)
{
	switch (msg->type)
	{
	case NVSEMessagingInterface::kMessage_PostLoad: break;
	case NVSEMessagingInterface::kMessage_ExitGame: break;
	case NVSEMessagingInterface::kMessage_ExitToMainMenu: break;
	case NVSEMessagingInterface::kMessage_LoadGame: break;
	case NVSEMessagingInterface::kMessage_SaveGame: break;
#if EDITOR
	case NVSEMessagingInterface::kMessage_ScriptEditorPrecompile: break;
#endif
	case NVSEMessagingInterface::kMessage_PreLoadGame: break;
	case NVSEMessagingInterface::kMessage_ExitGame_Console: break;
	case NVSEMessagingInterface::kMessage_PostLoadGame: break;
	case NVSEMessagingInterface::kMessage_PostPostLoad: break;
	case NVSEMessagingInterface::kMessage_RuntimeScriptError: break;
	case NVSEMessagingInterface::kMessage_DeleteGame: break;
	case NVSEMessagingInterface::kMessage_RenameGame: break;
	case NVSEMessagingInterface::kMessage_RenameNewGame: break;
	case NVSEMessagingInterface::kMessage_NewGame: break;
	case NVSEMessagingInterface::kMessage_DeleteGameName: break;
	case NVSEMessagingInterface::kMessage_RenameGameName: break;
	case NVSEMessagingInterface::kMessage_RenameNewGameName: break;
	case NVSEMessagingInterface::kMessage_DeferredInit: 
		Console_Print("Hello World!");
		break;
	case NVSEMessagingInterface::kMessage_ClearScriptDataCache: break;
	case NVSEMessagingInterface::kMessage_MainGameLoop: break;
	case NVSEMessagingInterface::kMessage_ScriptCompile: break;
	default: break;
	}
}

bool NVSEPlugin_Query(const NVSEInterface* nvse, PluginInfo* info)
{
	_MESSAGE("query");

	// fill out the info structure
	info->infoVersion = PluginInfo::kInfoVersion;
	info->name = "shellNVSE";
	info->version = g_PluginVersion;

	return true;
}

bool NVSEPlugin_Load(NVSEInterface* nvse)
{

	_MESSAGE("load");

	g_pluginHandle = nvse->GetPluginHandle();

	// save the NVSE interface in case we need it later
	g_nvseInterface = nvse;

	// register to receive messages from NVSE
	g_messagingInterface = static_cast<NVSEMessagingInterface*>(nvse->QueryInterface(kInterface_Messaging));
	g_messagingInterface->RegisterListener(g_pluginHandle, "NVSE", MessageHandler);

	if (nvse->isEditor)
	{
		_MESSAGE("shellNVSE Loaded successfully (Editor).\shellNVSE Plug Version: %u\n", g_PluginVersion);
	}
	else
	{
		_MESSAGE("shellNVSE Loaded successfully (In-Game).\shellNVSE Plug Version: %u\n", g_PluginVersion);
	}

	// register to receive messages from NVSE
	((NVSEMessagingInterface*)nvse->QueryInterface(kInterface_Messaging))->RegisterListener(nvse->GetPluginHandle(), "NVSE", MessageHandler);

	if (!nvse->isEditor)
	{

		g_scriptInterface = (NVSEScriptInterface*)nvse->QueryInterface(kInterface_Script);
		ExtractArgsEx = g_scriptInterface->ExtractArgsEx;
		ExtractFormatStringArgs = g_scriptInterface->ExtractFormatStringArgs;
		FunctionCallScript = g_scriptInterface->CallFunction;
		FunctionCallScriptAlt = g_scriptInterface->CallFunctionAlt;
		pCompileScript = g_scriptInterface->CompileScript;
		pCompileExpression = g_scriptInterface->CompileExpression;

		g_cmdTableInterface = *(NVSECommandTableInterface*)nvse->QueryInterface(kInterface_CommandTable);
		GetCmdByName = g_cmdTableInterface.GetByName;

		g_strInterface = *(NVSEStringVarInterface*)nvse->QueryInterface(kInterface_StringVar);
		GetStringVar = g_strInterface.GetString;
		AssignString = g_strInterface.Assign;

		g_arrInterface = (NVSEArrayVarInterface*)nvse->QueryInterface(kInterface_ArrayVar);
		CreateArray = g_arrInterface->CreateArray;
		CreateMap = g_arrInterface->CreateMap;
		CreateStringMap = g_arrInterface->CreateStringMap;
		AssignArrayResult = g_arrInterface->AssignCommandResult;
		SetElement = g_arrInterface->SetElement;
		AppendElement = g_arrInterface->AppendElement;
		GetArraySize = g_arrInterface->GetArraySize;
		LookupArrayByID = g_arrInterface->LookupArrayByID;
		GetElement = g_arrInterface->GetElement;
		GetArrayElements = g_arrInterface->GetElements;

		nvse->InitExpressionEvaluatorUtils(&s_expEvalUtils);

		g_eventInterface = (NVSEEventManagerInterface*)nvse->QueryInterface(kInterface_EventManager);

		g_keyInterface = (DIHookControl*)nvse->QueryInterface(NVSEDataInterface::kNVSEData_DIHookControl);

		#if WantLambdaFunctions
				LambdaDeleteAllForScript = (_LambdaDeleteAllForScript)nvseData->GetFunc(NVSEDataInterface::kNVSEData_LambdaDeleteAllForScript);
				LambdaSaveVariableList = (_LambdaSaveVariableList)nvseData->GetFunc(NVSEDataInterface::kNVSEData_LambdaSaveVariableList);
				LambdaUnsaveVariableList = (_LambdaUnsaveVariableList)nvseData->GetFunc(NVSEDataInterface::kNVSEData_LambdaUnsaveVariableList);
				IsScriptLambda = (_IsScriptLambda)nvseData->GetFunc(NVSEDataInterface::kNVSEData_IsScriptLambda);
		#endif

		#if WantScriptFunctions
				HasScriptCommand = (_HasScriptCommand)nvseData->GetFunc(NVSEDataInterface::kNVSEData_HasScriptCommand);
				DecompileScript = (_DecompileScript)nvseData->GetFunc(NVSEDataInterface::kNVSEData_DecompileScript);
		#endif
;
		PluginHandle pluginHandle = nvse->GetPluginHandle();

	}

	//	See https://geckwiki.com/index.php?title=NVSE_Opcode_Base

	UInt32 const ShellNVSEPluginOpcodeBase = 0; //Replace

	// register commands
	nvse->SetOpcodeBase(ShellNVSEPluginOpcodeBase);
		
		
	return true;
}

