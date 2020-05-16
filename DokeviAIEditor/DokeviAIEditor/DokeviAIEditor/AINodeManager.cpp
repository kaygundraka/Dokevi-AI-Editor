#include "pch.h"
#include "AINodeManager.h"
#include "UIManager.h"

using namespace rapidjson;
using namespace DokeviAIEditor;

ImColor DokeviAIEditor::MakeRandomColor()
{
	ImColor color;
	color.SetHSV((float)(rand() % 70) / 100.f + 0.3f, (float)(rand() % 70) / 100.f + 0.3f, (float)(rand() % 70) / 100.f + 0.3f);

	return color;
}

std::map<std::string, AINode*(*)()> available_nodes{
	{"sequence", []() -> AINode* { return new AINode("sequence", {
		{"Parent", NodeSlotLink}
	}, {
		{"Child", NodeSlotLink}
	}); }},

	{"selector", []() -> AINode* { return new AINode("selector", {
		{"Parent", NodeSlotLink}
	}, {
		{"Child", NodeSlotLink}
	}); }},

	{"decoratorWhile", []() -> AINode* { return new AINode("decoratorWhile", {
		{"Parent", NodeSlotLink}
	}, {
		{"Child", NodeSlotLink}
	}); }},

	{"decoratorIf", []() -> AINode* { return new AINode("decoratorIf", {
		{"Parent", NodeSlotLink}
	}, {
		{"Child", NodeSlotLink},
		{"Double", NodeSlotDoubleValue},
		{"String", NodeSlotStringValue}
	}); }},

	{"execution", []() -> AINode* { return new AINode("execution", {
		{"Parent", NodeSlotLink}
	}, {
		{"Double", NodeSlotDoubleValue},
		{"String", NodeSlotStringValue}
	}); }},

	{"doubleValue", []() -> AINode* { return new AINode("doubleValue", {
		{"Double", NodeSlotDoubleValue}
	}, {
	}); }},

	{"stringValue", []() -> AINode* { return new AINode("stringValue", {
		{"String", NodeSlotStringValue}
	}, {
	}); }} 
};

AIAgent::AIAgent()
{
	_aiNodes.clear();
	_rootNode = nullptr;
}

AIAgent::~AIAgent()
{
	for (auto iter = _aiNodes.begin(); iter != _aiNodes.end();)
	{
		AINode* deleteNode = *iter;
		iter = _aiNodes.erase(iter);
		delete deleteNode;
	}

	_rootNode = nullptr;
}

void AIAgent::SetDefaultState()
{
	for (auto node : _aiNodes)
	{
		delete node;
	}

	_aiNodes.clear();

	_rootNode = new AINode("Root", {}, { {"Child", NodeSlotLink} });
	_rootNode->color = MakeRandomColor();
	_rootNode->pos.x = 30;
	_rootNode->pos.y = 150;

	_aiNodes.push_back(_rootNode);
}

void AIAgent::LoadDataFromJsonFile()
{
	UIManager::GetInstance()->_nodeFunctionMap.clear();

	SetDefaultState();

	std::string fileAddress = ".\\scripts\\" + _fileName;
	std::ifstream jsonFile(fileAddress);

	if (!jsonFile.is_open())
	{
		return;
	}

	IStreamWrapper isw(jsonFile);

	Document aiDocument;

	aiDocument.ParseStream(isw);

	if (aiDocument.HasParseError())
	{
		return;
	}

	std::string aiName = aiDocument.FindMember("ai")->value.GetString();

	auto& behaviourComponent = aiDocument.FindMember("behaviourTree")->value;

	ParseAINodes(_rootNode, behaviourComponent, ImVec2(newNodeDistanceX, newNodeDistanceY));
}

AINode* AIAgent::MakeNewNode(AINode* inParent, std::string inNodeName, NodeSlotTypes inSlotType, ImVec2 inAddPos)
{
	AINode*	newNode = available_nodes[inNodeName]();

	Connection newConnection;

	auto inputSlotIter = newNode->input_slots.begin();

	while (inputSlotIter != newNode->input_slots.end())
	{
		if (inputSlotIter->kind == inSlotType)
		{
			break;
		}

		inputSlotIter++;
	}

	auto outputSlotIter = inParent->output_slots.begin();

	while (outputSlotIter != inParent->output_slots.end())
	{
		if (outputSlotIter->kind == inSlotType)
		{
			break;
		}

		outputSlotIter++;
	}

	newConnection.input_node = newNode;
	newConnection.input_slot = inputSlotIter->title;
	newConnection.output_node = inParent;
	newConnection.output_slot = outputSlotIter->title;
	newConnection.color = inParent->color;

	inParent->connections.push_back(newConnection);
	newNode->connections.push_back(newConnection);

	newNode->pos = GetUniquePosition(inParent->pos + inAddPos);

	return newNode;
}

ImVec2 AIAgent::GetUniquePosition(ImVec2 inCurPos) const
{
	while (true)
	{
		bool checkValidPos = true;

		for (auto item : _aiNodes)
		{
			if (item->pos.x == inCurPos.x && item->pos.y == inCurPos.y)
			{
				checkValidPos = false;
			}
		}

		if (!checkValidPos)
		{
			inCurPos.y += newNodeDistanceY;
		}
		else
		{
			break;
		}
	}

	return inCurPos;
}

void AIAgent::ParseValues(AINode* inParent, ImVec2 inAddPos, std::map<std::string, double> inDoubleArray, std::map<std::string, std::string> inStrArray)
{
	int i = 0;

	for (auto item : inDoubleArray)
	{
		AINode*	newNode = MakeNewNode(inParent, "doubleValue", NodeSlotDoubleValue, inAddPos + ImVec2((float)newNodeDistanceX, (++i) * (float)newNodeDistanceY));
		newNode->name = item.first;
		newNode->doubleData = item.second;
		_aiNodes.push_back(newNode);
	}

	i = 0;

	for (auto item : inStrArray)
	{
		AINode*	newNode = MakeNewNode(inParent, "stringValue", NodeSlotStringValue, inAddPos + ImVec2((float)newNodeDistanceX, (++i) * (float)newNodeDistanceY));
		newNode->name = item.first;
		newNode->stringData = item.second;
		_aiNodes.push_back(newNode);
	}
}

void AIAgent::ParseAINodes(AINode* inParent, rapidjson::Value& inJsonObject, ImVec2 inAddPos)
{
	std::string name = inJsonObject.FindMember("type")->value.GetString();

	std::map<std::string, double> doubleArray;
	std::map<std::string, std::string> strArray;

	AINode*	newNode = MakeNewNode(inParent, name, NodeSlotLink, inAddPos);

	_aiNodes.push_back(newNode);

	if (inJsonObject.HasMember("name"))
	{
		std::string funcName = inJsonObject.FindMember("name")->value.GetString();

		UsingFunctionNodeType type;

		if (name == "decoratorIf")
		{
			type = E_UFNT_DECORATOR_IF;
		}
		else if (name == "decoratorWhile")
		{
			type = E_UFNT_DECORATOR_WHILE;
		}
		else if (name == "execution")
		{
			type = E_UFNT_EXECUTION;
		}

		UIManager::GetInstance()->_nodeFunctionMap[type].push_back(funcName);
		newNode->name = inJsonObject.FindMember("name")->value.GetString();
	}

	if (name == "sequence" || name == "behaviourTree")
	{
		if (!inJsonObject.HasMember("child"))
		{
			return;
		}

		auto childArray = inJsonObject.FindMember("child")->value.GetArray();

		for (unsigned int i = 0; i < childArray.Size(); i++)
		{
			ParseAINodes(newNode, childArray[i], ImVec2((float)newNodeDistanceX, (i + 1)* (float)newNodeDistanceY));
		}
	}
	else if (name == "decoratorWhile")
	{
		ParseAINodes(newNode, inJsonObject.FindMember("child")->value, ImVec2((float)newNodeDistanceX * 2, 0));

		if (inJsonObject.HasMember("const"))
		{
			auto childArray = inJsonObject.FindMember("const")->value.GetArray();

			for (unsigned int i = 0; i < childArray.Size(); i++)
			{
				if (childArray[i].MemberBegin()->value.IsNumber())
				{
					doubleArray.insert(std::make_pair(
						childArray[i].MemberBegin()->name.GetString(),
						childArray[i].MemberBegin()->value.GetDouble()
					)
					);
				}
				else if (childArray[i].MemberBegin()->value.IsString())
				{
					strArray.insert(std::make_pair(
						childArray[i].MemberBegin()->name.GetString(),
						childArray[i].MemberBegin()->value.GetString()
					)
					);
				}
			}
		}
	}
	else if (name == "decoratorIf")
	{
		ParseAINodes(newNode, inJsonObject.FindMember("child")->value, ImVec2(newNodeDistanceX * 2, 0));

		if (inJsonObject.HasMember("const"))
		{
			auto childArray = inJsonObject.FindMember("const")->value.GetArray();

			for (unsigned int i = 0; i < childArray.Size(); i++)
			{
				if (childArray[i].MemberBegin()->value.IsNumber())
				{
					doubleArray.insert(std::make_pair(
						childArray[i].MemberBegin()->name.GetString(),
						childArray[i].MemberBegin()->value.GetDouble()
					)
					);
				}
				else if (childArray[i].MemberBegin()->value.IsString())
				{
					strArray.insert(std::make_pair(
						childArray[i].MemberBegin()->name.GetString(),
						childArray[i].MemberBegin()->value.GetString()
					)
					);
				}
			}
		}
		
		ParseValues(newNode, ImVec2(100, 100), doubleArray, strArray);
	}
	else if (name == "execution")
	{
		if (inJsonObject.HasMember("const"))
		{
			auto childArray = inJsonObject.FindMember("const")->value.GetArray();

			for (unsigned int i = 0; i < childArray.Size(); i++)
			{
				if (childArray[i].MemberBegin()->value.IsNumber())
				{
					doubleArray.insert(std::make_pair(
						childArray[i].MemberBegin()->name.GetString(),
						childArray[i].MemberBegin()->value.GetDouble()
					)
					);
				}
				else if (childArray[i].MemberBegin()->value.IsString())
				{
					strArray.insert(std::make_pair(
						childArray[i].MemberBegin()->name.GetString(),
						childArray[i].MemberBegin()->value.GetString()
					)
					);
				}
			}
		}

		ParseValues(newNode, ImVec2(100, 100), doubleArray, strArray);
	}
	else if (name == "selector")
	{
		if (!inJsonObject.HasMember("child"))
		{
			return;
		}

		auto childArray = inJsonObject.FindMember("child")->value.GetArray();

		for (unsigned int i = 0; i < childArray.Size(); i++)
		{
			ParseAINodes(newNode, childArray[i], ImVec2((float)newNodeDistanceX, (i + 1) * (float)newNodeDistanceY));
		}
	}
	else
	{
		return;
	}
}

void AINodeToJsonObject(Document& inDocument, Value& inObject, AINode* inAINode, Document::AllocatorType& inAllocator)
{
	if (inAINode == nullptr)
	{
		return;
	}

	Value type;
	type.SetString(inAINode->title, strlen(inAINode->title));
	inObject.AddMember("type", type, inAllocator);

	std::string nodeType = inAINode->title;

	if (nodeType == "execution")
	{
		Value name;
		name.SetString(inAINode->name.c_str(), inAINode->name.length());
		inObject.AddMember("name", name, inAllocator);
		return;
	}

	if (nodeType == "sequence" || nodeType == "selector")
	{
		Value childs;
		childs.SetArray();

		for (auto connection : inAINode->connections)
		{
			if (connection.input_node == inAINode)
				continue;

			if (strcmp(connection.input_slot, "Parent") != 0)
			{
				continue;
			}

			Value child;
			child.SetObject();

			AINodeToJsonObject(inDocument, child, (AINode*)connection.input_node, inAllocator);

			childs.PushBack(child, inAllocator);
		}

		inObject.AddMember("child", childs, inAllocator);
	}

	if (nodeType == "decoratorIf" || nodeType == "decoratorWhile")
	{
		Value name;
		name.SetString(inAINode->name.c_str(), inAINode->name.length());
		inObject.AddMember("name", name, inAllocator);

		for (auto connection : inAINode->connections)
		{
			if (connection.input_node == inAINode)
				continue;

			if (strcmp(connection.input_slot, "Parent") != 0)
			{
				continue;
			}

			Value child;
			child.SetObject();

			AINodeToJsonObject(inDocument, child, (AINode*)connection.input_node, inAllocator);

			inObject.AddMember("child", child, inAllocator);
			break;
		}

		Value constArray;
		constArray.SetArray();

		for (auto connection : inAINode->connections)
		{
			if (connection.input_node == inAINode)
				continue;

			AINode* node = (AINode*)(connection.input_node);

			Value constObject;
			constObject.SetObject();

			Value data;
			Value dataName;
			dataName.SetString(node->name.c_str(), node->name.length());

			if (strcmp(connection.input_slot, "Double") == 0)
			{
				data.SetDouble(node->doubleData);
			}
			else if (strcmp(connection.input_slot, "String") == 0)
			{
				std::string stringData = node->stringData;
				data.SetString(stringData.c_str(), stringData.length());
			}
			else
			{
				continue;
			}

			constObject.AddMember(dataName, data, inAllocator);
			constArray.PushBack(constObject, inAllocator);
		}

		inObject.AddMember("const", constArray, inAllocator);
	}
}

void AIAgent::SaveDataToJsonFIle()
{
	Document aiDocument;

	Document::AllocatorType& allocator = aiDocument.GetAllocator();
	aiDocument.SetObject();

	std::string aiName = _fileName;
	int subStrPos = (int)aiName.rfind(".json");
	aiName = aiName.substr(0, subStrPos);

	Value objectName;
	objectName.SetString(aiName.c_str(), aiName.length());
	aiDocument.AddMember("ai", objectName, allocator);

	Value behaviourTreeObject;
	behaviourTreeObject.SetObject();

	AINode* firstNode = nullptr;

	for (auto iter = _rootNode->connections.begin(); iter != _rootNode->connections.end(); iter++)
	{
		if ((AINode*)iter->input_node == _rootNode)
			continue;

		firstNode = (AINode*)iter->input_node;
		break;
	}

	AINodeToJsonObject(aiDocument, behaviourTreeObject, firstNode, allocator);

	aiDocument.AddMember("behaviourTree", behaviourTreeObject, allocator);

	rapidjson::StringBuffer buffer;
	buffer.Clear();
	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	aiDocument.Accept(writer);

	std::string json = buffer.GetString();

	std::string fileAddress = ".\\scripts\\" +_fileName;
	std::ofstream jsonFile(fileAddress);

	if (!jsonFile.is_open())
	{
		return;
	}

	jsonFile.write(json.c_str(), json.length());

	jsonFile.close();
}