#pragma once
#include "pch.h"

namespace DokeviAIEditor
{
	ImColor MakeRandomColor();

	struct Connection
	{
		void* input_node = nullptr;
		const char* input_slot = nullptr;
		void* output_node = nullptr;
		const char* output_slot = nullptr;

		bool operator==(const Connection& other) const
		{
			return input_node == other.input_node &&
				input_slot == other.input_slot &&
				output_node == other.output_node &&
				output_slot == other.output_slot;
		}

		bool operator!=(const Connection& other) const
		{
			return !operator ==(other);
		}

		ImColor color;
	};

	struct AINode
	{
		const char* title = nullptr;
		bool selected = false;

		ImVec2 pos{};

		std::string name = "";

		ImColor color;

		std::string stringData;
		double doubleData;

		std::vector<Connection> connections{};
		std::vector<ImNodes::Ez::SlotInfo> input_slots{};
		std::vector<ImNodes::Ez::SlotInfo> output_slots{};

		explicit AINode(const char* title,
			const std::vector<ImNodes::Ez::SlotInfo>&& input_slots,
			const std::vector<ImNodes::Ez::SlotInfo>&& output_slots)
		{
			this->title = title;
			this->input_slots = input_slots;
			this->output_slots = output_slots;
			this->color = MakeRandomColor();
		}

		void DeleteConnection(const Connection& connection)
		{
			for (auto it = connections.begin(); it != connections.end(); ++it)
			{
				if (connection == *it)
				{
					connections.erase(it);
					break;
				}
			}
		}
	};

	class AIAgent {
	public:
		AINode* _rootNode;
		std::vector<AINode*> _aiNodes;
		std::string _fileName;

	public:
		AIAgent();
		~AIAgent();

		void SetDefaultState();

		AINode* MakeNewNode(AINode* inParent, std::string inNodeName, NodeSlotTypes inSlotType, ImVec2 inAddPos);

		ImVec2 GetUniquePosition(ImVec2 inCurPos) const;

		void ParseValues(AINode* inParent, ImVec2 inAddPos, std::map<std::string, double> inDoubleArray, std::map<std::string, std::string> inStrArray);
		void ParseAINodes(AINode* inParent, rapidjson::Value& inJsonObject, ImVec2 inAddPos);

		void LoadDataFromJsonFile();
		void SaveDataToJsonFIle();
	};
}