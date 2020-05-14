#include "pch.h"
#include "UIManager.h"

using namespace DokeviAIEditor;

extern std::map<std::string, AINode*(*)()> available_nodes;

int UIManager::GetWindowWidth() const
{
	RECT rect;

	if (GetWindowRect(_windowHandle, &rect))
	{
		return rect.right - rect.left;
	}

	return -1;
}

int UIManager::GetWindowHeight() const
{
	RECT rect;

	if (GetWindowRect(_windowHandle, &rect))
	{
		return rect.bottom - rect.top;
	}

	return -1;
}

void UIManager::SetWindowsHandle(HWND hHandle) 
{ 
	_windowHandle = hHandle; 
}

void UIManager::DrawFolderView()
{
	_finddata_t fd;

	char path[MAX_PATH] = { 0, };

	::GetCurrentDirectoryA(MAX_PATH, path);

	std::string address = ".\\scripts\\*.json";
	std::string absoluteAddress = path;
	absoluteAddress += address.substr(1, address.size() - 1);

	intptr_t handle = _findfirst(absoluteAddress.c_str(), &fd);

	const int halfHeight = (GetWindowHeight() - menuBarHeight) / 2;

	ImGui::SetNextWindowPos(ImVec2(GetWindowWidth() - 315, menuBarHeight));
	ImGui::SetNextWindowSize(ImVec2(300, halfHeight));

	if (ImGui::Begin("AI Folder View"))
	{
		ImGui::SetNextTreeNodeOpen(true);

		static char findFileName[32] = "";

		if (ImGui::TreeNode("Files"))
		{
			if (handle != -1)
			{
				int result = 1;

				while (result != -1)
				{
					if (strcmp(fd.name, ".") != 0 && strcmp(fd.name, "..") != 0)
					{
						if (ImGui::TreeNode(fd.name))
						{
							strcpy_s(findFileName, fd.name);
							ImGui::TreePop();
						}
					}

					result = _findnext(handle, &fd);
				}
			}

			ImGui::TreePop();
		}

		ImGui::Separator();

		ImGui::InputText("File", findFileName, 32);

		ImGui::SetNextItemWidth(GetWindowWidth() - 315);

		if (ImGui::Button("Open File", ImVec2(280, 30)))
		{
			AIAgent* newAgent = new AIAgent();
			newAgent->_fileName = findFileName;
			newAgent->LoadDataFromJsonFile();
			_curEditedAI = newAgent;
		}

		if (ImGui::Button("New File", ImVec2(280, 30)))
		{
			AIAgent* newAgent = new AIAgent();
			newAgent->SetDefaultState();
			newAgent->_fileName = findFileName;
			_curEditedAI = newAgent;
		}

		if (ImGui::Button("Delete File", ImVec2(280, 30)))
		{
			if (_curEditedAI != nullptr && _curEditedAI->_fileName == findFileName)
			{
				delete _curEditedAI;
				_curEditedAI = nullptr;
			}
		}
	}

	ImGui::End();

	_findclose(handle);
}

void UIManager::DrawNodeTree(AINode* inNode)
{
	ImGui::SetNextTreeNodeOpen(true);

	if (ImGui::TreeNode(inNode->title))
	{
		if (ImGui::IsItemHovered())
		{
			_selectNode = inNode;
		}

		for (auto connection : inNode->connections)
		{
			if (connection.input_node == inNode)
				continue;

			DrawNodeTree((AINode*)connection.input_node);
		}

		ImGui::TreePop();
	}
}

void UIManager::DrawNodeView(AIAgent* inAIAgent)
{
	const int halfHeight = (GetWindowHeight() - menuBarHeight) / 2;

	ImGui::SetNextWindowPos(ImVec2(GetWindowWidth() - 315, menuBarHeight + halfHeight));
	ImGui::SetNextWindowSize(ImVec2(300, halfHeight));

	if (_curEditedAI == nullptr)
	{
		if (ImGui::Begin("AI Inspector"))
		{
		}

		ImGui::End();

		return;
	}

	if (ImGui::Begin("AI Inspector"))
	{
		ImGui::SetNextTreeNodeOpen(true);

		if (ImGui::TreeNode("Nodes"))
		{
			DrawNodeTree(inAIAgent->_rootNode);
			
			ImGui::TreePop();
		}

		ImGui::Separator();

		ImGui::Text("[Node Inspector]");
		
		if (_selectNode)
		{
			ImGui::LabelText("Type", _selectNode->title);

			if (_selectNode->name != "")
				ImGui::LabelText("Function", _selectNode->name.c_str());
		}
	}

	ImGui::End();
}

void UIManager::DrawAIInstance(AIAgent* inAIAgent)
{
	static ImNodes::CanvasState canvas{};

	const ImGuiStyle& style = ImGui::GetStyle();

	ImGui::SetNextWindowPos(ImVec2(0, menuBarHeight));
	ImGui::SetNextWindowSize(ImVec2(GetWindowWidth() - 315, GetWindowHeight() - menuBarHeight));

	std::string windowName = "AI Instance - " + (inAIAgent == nullptr ? "empty" : inAIAgent->_fileName);

	if (_curEditedAI == nullptr)
	{
		if (ImGui::Begin(windowName.c_str(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
		{
			ImNodes::BeginCanvas(&canvas);

			ImNodes::EndCanvas();
		}
		ImGui::End();

		return;
	}

	if (ImGui::Begin(windowName.c_str(), nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
	{
		ImGui::BeginChildFrame(1, ImVec2(173, 75));
		
		ImGui::LabelText("File", inAIAgent->_fileName.c_str());

		if (ImGui::ButtonEx("Save", ImVec2(164, 20)))
		{
			inAIAgent->SaveDataToJsonFIle();
		}

		if (ImGui::ButtonEx("End", ImVec2(164, 20)))
		{

		}
		ImGui::EndChildFrame();

		ImNodes::BeginCanvas(&canvas);

		for (auto it = inAIAgent->_aiNodes.begin(); it != inAIAgent->_aiNodes.end();)
		{
			AINode* node = *it;

			if (ImNodes::Ez::BeginNode(node, node->title, &node->pos, &node->selected))
			{
				ImNodes::Ez::InputSlots(node->input_slots.data(), node->input_slots.size());

				std::string title = node->title;

				if (title == "decoratorIf" || title == "decoratorWhile" || title == "execution")
				{
					ImGui::Text("Content of %s", title.c_str());

					char newName[33] = "";
					strcpy_s(newName, node->name.c_str());

					ImGui::SetNextItemWidth(100);
					ImGui::InputTextWithHint("AI Handler", "Function", newName, 128);

					if (newName != node->name)
					{
						node->name = newName;
					}
				}
				else if (title == "doubleValue")
				{
					double temp = 0;
					char temp2[128] = "";
					strcpy_s(temp2, node->name.c_str());

					ImGui::SetNextItemWidth(100);
					ImGui::InputText("Name", temp2, 128);
					ImGui::SetNextItemWidth(100);
					ImGui::InputDouble("Value", &temp);
				}
				else if (title == "stringValue")
				{
					char temp[128] = "temp";
					char temp2[128] = "";
					strcpy_s(temp2, node->name.c_str());

					ImGui::SetNextItemWidth(100);
					ImGui::InputText("Name", temp2, 128);
					ImGui::SetNextItemWidth(100);
					ImGui::InputText("Text", temp, 30);
				}
				else
				{
					ImGui::Text("Content of %s", title.c_str());
				}

				ImNodes::Ez::OutputSlots(node->output_slots.data(), node->output_slots.size());

				Connection new_connection;
				if (ImNodes::GetNewConnection(&new_connection.input_node, &new_connection.input_slot,
					&new_connection.output_node, &new_connection.output_slot))
				{
					new_connection.color = ((AINode*)new_connection.output_node)->color;
					((AINode*)new_connection.input_node)->connections.push_back(new_connection);
					((AINode*)new_connection.output_node)->connections.push_back(new_connection);
				}

				for (const Connection& connection : node->connections)
				{
					if (connection.output_node != node)
						continue;

					ImColor defaultColor = canvas.colors[ImNodes::ColConnection];
					canvas.colors[ImNodes::ColConnection] = connection.color;

					if (!ImNodes::Connection(connection.input_node, connection.input_slot, connection.output_node,
						connection.output_slot))
					{
						((AINode*)connection.input_node)->DeleteConnection(connection);
						((AINode*)connection.output_node)->DeleteConnection(connection);
					}

					canvas.colors[ImNodes::ColConnection] = defaultColor;
				}
			}
			
			ImNodes::Ez::EndNode();

			if (node->selected && (ImGui::IsKeyPressedMap(ImGuiKey_Delete) || ImGui::IsKeyPressedMap(ImGuiKey_X)))
			{
				for (auto& connection : node->connections)
				{
					if (connection.output_node == node)
					{
						((AINode*)connection.input_node)->DeleteConnection(connection);
					}
					else
					{
						((AINode*)connection.output_node)->DeleteConnection(connection);
					}
				}
				
				node->connections.clear();

				it = inAIAgent->_aiNodes.erase(it);
				delete node;
			}
			else
				++it;
		}

		const ImGuiIO& io = ImGui::GetIO();
		if (ImGui::IsMouseReleased(1) && ImGui::IsWindowHovered() && !ImGui::IsMouseDragging(1))
		{
			ImGui::FocusWindow(ImGui::GetCurrentWindow());
			ImGui::OpenPopup("NodesContextMenu");
		}

		if (ImGui::BeginPopup("NodesContextMenu"))
		{
			for (const auto& desc : available_nodes)
			{
				if (ImGui::MenuItem(desc.first.c_str()))
				{
					inAIAgent->_aiNodes.push_back(desc.second());
					ImNodes::AutoPositionNode(inAIAgent->_aiNodes.back());
				}
			}
			ImGui::Separator();
			if (ImGui::MenuItem("Reset Zoom"))
				canvas.zoom = 1;

			if (ImGui::IsAnyMouseDown() && !ImGui::IsWindowHovered())
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}

		ImNodes::EndCanvas();
	}
	ImGui::End();
}

void UIManager::DrawMainMenuBar()
{
	if (ImGui::BeginMainMenuBar())
	{
		if (ImGui::BeginMenu("System"))
		{
			if (ImGui::MenuItem("About DokeviAIEditor", "CTRL+I")) {}
			if (ImGui::MenuItem("Exit", "CTRL+E")) {}

			ImGui::EndMenu();
		}

		ImGui::EndMainMenuBar();
	}
}

void UIManager::Render()
{
	DrawMainMenuBar();
	DrawFolderView();

	DrawAIInstance(_curEditedAI);
	DrawNodeView(_curEditedAI);
}