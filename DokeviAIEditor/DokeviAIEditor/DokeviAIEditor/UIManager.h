#pragma once

#include "ISingleton.h"
#include "AINodeManager.h"

namespace DokeviAIEditor
{
	class UIManager : public ISingleton<UIManager>
	{
	public:
		AIAgent* _curEditedAI;
		HWND _windowHandle;

		AINode* _selectNode = nullptr;

		std::map<UsingFunctionNodeType, std::vector<std::string>> _nodeFunctionMap;

	private:
		void DrawFolderView();
		void DrawMainMenuBar();

		void DrawNodeTree(AINode* inNode);
		void DrawNodeView(AIAgent* inAIAgent);
		void DrawAIInstance(AIAgent* inAIAgent);

		int GetWindowWidth() const;
		int GetWindowHeight() const;

	public:
		UIManager() : _curEditedAI(nullptr) {}

		void SetWindowsHandle(HWND inHandle);
		void Render();
	};
}