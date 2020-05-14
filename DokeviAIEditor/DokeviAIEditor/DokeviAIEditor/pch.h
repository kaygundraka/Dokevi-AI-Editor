#pragma once

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#   define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <iostream>

#include <Windows.h>
#include <stdio.h>
#include <io.h>
#include <conio.h>

#include <d3d9.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#include <tchar.h>
#include <time.h>

#include ".\imgui\imgui.h"
#include ".\imgui\imgui_for_dx9\imgui_impl_dx9.h"
#include ".\imgui\imgui_for_dx9\imgui_impl_win32.h"

#include ".\imgui\im_nodes\ImNodesEz.h"

#include ".\rapidjson\document.h"
#include ".\rapidjson\stringbuffer.h"
#include ".\rapidjson\writer.h"
#include ".\rapidjson\istreamwrapper.h"

const int newNodeDistanceX = 180;
const int newNodeDistanceY = 80;
const int menuBarHeight = 18;

namespace DokeviAIEditor
{
	enum NodeSlotTypes
	{
		NodeSlotLink = 1,
		NodeSlotDoubleValue,
		NodeSlotStringValue,
	};

	enum UsingFunctionNodeType
	{
		E_UFNT_DECORATOR_IF = 0,
		E_UFNT_DECORATOR_WHILE,
		E_UFNT_EXECUTION,
		E_UFNT_MAX
	};
}