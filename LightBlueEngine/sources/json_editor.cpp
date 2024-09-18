// <>インクルード
#include <fstream>
#include <imgui.h>
#include <ImGuiFileDialog.h>

// ""インクルード
// LightBlueEngine
#include "graphics/graphics.h"
#include "json_editor.h"
#include "engine_utility.h"

// using
using json = nlohmann::json;

//--------------------------------------
//	JSONEditor メンバ関数
//-------------------------------------
// JSONを保存したいパスを設定する初期化関数
void JSONEditor::Initialize(std::string output_path_str)
{
	output_path.assign(output_path_str);

	if (output_path_str.compare("*.json"))
	{
		ImportJSON(std::filesystem::path(output_path_str));
	}
}

// ImGuiを表示する関数。このJSONエディターの核となる。
void JSONEditor::DebugGUI()
{
	Graphics* graphics = Graphics::GetInstance();

	float width	= SCast(float, graphics->GetScreenWidth());
	float height	= SCast(float, graphics->GetScreenHeight());

	ImGuiFileDialog* imgui_fd = ImGuiFileDialog::Instance();

	char output_path_chr[64];
	strcpy_s(output_path_chr, sizeof(output_path_chr), output_path.string().c_str());

	if (ImGui::Begin("JSON Editter", (bool*)0, ImGuiWindowFlags_NoResize))
	{
		if (ImGui::InputText("Output Path", output_path_chr, sizeof(output_path_chr)))
		{

		}

		ImGui::NextColumn();

		if (ImGui::Button("SelectOutputPath"))
		{
			imgui_fd->OpenDialog("JSONSelecter", "Specify the output destination for JSON files", ".json", "output_json/");
		}

		if (imgui_fd->Display("JSONSelecter", 0, ImVec2(width, height * 0.5f), ImVec2(width, height * 0.5f)))
		{
			if (imgui_fd->IsOk())
			{
				std::filesystem::path currentpath(imgui_fd->GetCurrentPath());
				std::filesystem::path filepath(imgui_fd->GetFilePathName());

				filepath = filepath.lexically_relative(currentpath.parent_path());
				output_path.assign(filepath);

				std::ifstream input_file(filepath);
				if (input_file)
				{
					parameters.clear();
					ImportJSON(filepath);
				}
			}
			imgui_fd->Close();
		}

		// パラメータの設定
		if (ImGui::BeginTable("Parameters", 5, ImGuiTableFlags_Resizable))
		{
			for (int id = 0; id < parameters.size(); id++)
			{
				// パラメータから各データを取得
				ParamData&		param		= parameters.at(id);
				std::string&	param_name	= param.PARAM_NAME;

				EnumJSONType&	param_type	= param.PARAM_TYPE;
				std::any&		param_value = param.PARAM_VALUE;

				ImGui::TableNextRow();
				
				// ID表示
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Id %d", id);

				ImGui::TableSetColumnIndex(1);

				// パラメータ名
				char param_name_chr[64];
				strcpy_s(param_name_chr, sizeof(param_name_chr), param_name.c_str());

				std::string name_id = "Key##" + std::to_string(id);
				if (ImGui::InputText(name_id.c_str(), param_name_chr, sizeof(param_name_chr)))
				{
					param_name.assign(param_name_chr);
				}

				ImGui::TableSetColumnIndex(2);

				// パラメータの型名・設定
				std::string type_id = "Type##" + std::to_string(id);

				const char* types[] = {
					"String",
					"Integer",
					"Float",
					"Boolean",
					"Null",
					"Array",
					"Object",
				};

				int selecting_id = SCast(int, param_type);
				const char* selecting_type = types[selecting_id];

				// 型変更のプルダウン
				if (ImGui::BeginCombo(type_id.c_str(), selecting_type))
				{
					for (int i = 0; i < SCast(int, EnumJSONType::JSON_TYPE_NUM); i++)
					{
						const bool is_selected = (selecting_id == i);
						if (ImGui::Selectable(types[i], is_selected))
						{
							selecting_id = i;

							switch (SCast(EnumJSONType, selecting_id))
							{
							case EnumJSONType::STRING:
								param_value = std::string("");
								break;
							case EnumJSONType::INTEGER:
								param_value = int(0);
								break;
							case EnumJSONType::FLOAT:
								param_value = float(0.0f);
								break;
							case EnumJSONType::BOOLEAN:
								param_value = bool(false);
								break;
							case EnumJSONType::JSON_NULL:
								param_value = nullptr;
								break;
							case EnumJSONType::ARRAY:
							case EnumJSONType::OBJECT:
								param_value = Parameters();
								break;
							}
						}

						if (is_selected)
							ImGui::SetItemDefaultFocus();

						param_type = SCast(EnumJSONType, selecting_id);
					}
					ImGui::EndCombo();
				}

				ImGui::TableSetColumnIndex(3);

				// パラメータの設定
				// パラメータの設定
				// string型
				if (param_type == EnumJSONType::STRING)
				{
					std::string& param_value_str = ACast(std::string&, param_value);
					char param_value_chr[64];
					strcpy_s(param_value_chr, sizeof(param_value_chr), ACast(std::string, param_value).c_str());
					std::string value_id = "Value##" + std::to_string(id);
					if (ImGui::InputText(value_id.c_str(), param_value_chr, sizeof(param_value_chr)))
					{
						param_value_str.assign(param_value_chr);
					}
				}

				// int型
				else if (param_type == EnumJSONType::INTEGER)
				{
					int& param_value_int = ACast(int&, param_value);
					std::string value_id = "Value##" + std::to_string(id);
					if (ImGui::InputInt(value_id.c_str(), &param_value_int))
					{

					}
				}

				// float型
				else if (param_type == EnumJSONType::FLOAT)
				{
					float& param_value_float = ACast(float&, param_value);
					std::string value_id = "Value##" + std::to_string(id);
					if (ImGui::InputFloat(value_id.c_str(), &param_value_float))
					{

					}
				}

				// bool型
				else if (param_type == EnumJSONType::BOOLEAN)
				{
					bool& param_value_bool = ACast(bool&, param_value);
					std::string value_id = "Value##" + std::to_string(id);
					if (ImGui::Checkbox(value_id.c_str(), &param_value_bool))
					{

					}
				}

				// null
				else if (param_type == EnumJSONType::JSON_NULL)
				{
					std::string value_id = "Null##" + std::to_string(id);
					ImGui::Text(value_id.c_str());
				}

				// 配列
				else if (param_type == EnumJSONType::ARRAY)
				{
					std::string value_id = "EditArray##" + std::to_string(id);
					if (ImGui::Button(value_id.c_str()))
					{
						editting_array = true;
						editting_object = false;
						ParamRef array = ACast(ParamRef, param_value);
						editting_parameter_ptr = &array;
						editting_parameter_name = std::to_string(id);
					}
				}

				// オブジェクト
				else if (param_type == EnumJSONType::OBJECT)
				{
					std::string value_id = "EditObject##" + std::to_string(id);
					if (ImGui::Button(value_id.c_str()))
					{
						editting_object = true;
						editting_array = false;
						ParamRef object = ACast(ParamRef, param_value);
						editting_parameter_ptr = &object;
						editting_parameter_name = std::to_string(id);
					}
				}

				ImGui::TableSetColumnIndex(4);

				// パラメータの消去
				std::string delete_id = "Delete##" + std::to_string(id);
				if (ImGui::Button(delete_id.c_str()))
				{
					DeleteParameter(parameters, id);
				}
			}
			ImGui::EndTable();
		}

		// パラメータ追加ボタン
		if (ImGui::Button("Add Parameter"))
		{
			auto& param			= parameters.emplace_back();
			param.PARAM_NAME	= "Default";
			param.PARAM_TYPE	= EnumJSONType::JSON_NULL;
			param.PARAM_VALUE	= nullptr;
		}

		// JSON出力ボタン
		if (ImGui::Button("OutputJSON"))
		{
			OutputJSON();
		}

		ImGui::Checkbox("Auto Update", &auto_update);
		
		if(auto_update)
			BakeToJSON();
		else
		{
			if (ImGui::Button("Bake"))
			{
				BakeToJSON();
			}
		}

		if (editting_object)
		{
			EditObject();
		}
		if (editting_array)
		{
			EditArray();
		}
	}
	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(0, height * 0.5f));
	ImGui::SetNextWindowSize(ImVec2(width, height * 0.5f));
	if (ImGui::Begin("BakedJSONText", (bool*)0, ImGuiWindowFlags_NoResize))
	{
		ImGui::Text(json_str.c_str());
	}
	ImGui::End();
}

// 指定したオブジェクトを操作
void JSONEditor::EditObject()
{
	if (ImGui::Begin("Editting Object", (bool*)0, ImGuiWindowFlags_NoResize))
	{
		// パラメータの設定
		if (ImGui::BeginTable("Parameters", 5))
		{
			for (int id = 0; id < editting_parameter_ptr->size(); id++)
			{
				// パラメータから各データを取得
				ParamData&		param		= editting_parameter_ptr->at(id);
				std::string&	param_name	= param.PARAM_NAME;
				EnumJSONType&	param_type	= param.PARAM_TYPE;
				std::any&		param_value = param.PARAM_VALUE;

				ImGui::TableNextRow();

				// ID表示
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Id %d", id);

				ImGui::TableSetColumnIndex(1);

				// パラメータ名
				char param_name_chr[64];
				strcpy_s(param_name_chr, sizeof(param_name_chr), param_name.c_str());

				std::string name_id = "Key##" + std::to_string(id);
				if (ImGui::InputText(name_id.c_str(), param_name_chr, sizeof(param_name_chr)))
				{
					param_name.assign(param_name_chr);
				}

				ImGui::TableSetColumnIndex(2);

				// パラメータの型名・設定
				std::string type_id = "Type##" + std::to_string(id);

				const char* types[] = {
					"String",
					"Integer",
					"Float",
					"Boolean",
					"Null",
					"Array",
					"Object",
				};

				int selecting_id = SCast(int, param_type);
				const char* selecting_type = types[selecting_id];

				if (ImGui::BeginCombo(type_id.c_str(), selecting_type))
				{
					for (int i = 0; i < SCast(int, EnumJSONType::JSON_TYPE_NUM); i++)
					{
						const bool is_selected = (selecting_id == i);
						if (ImGui::Selectable(types[i], is_selected))
						{
							selecting_id = i;

							switch (SCast(EnumJSONType, selecting_id))
							{
							case EnumJSONType::STRING:
								param_value = std::string("");
								break;
							case EnumJSONType::INTEGER:
								param_value = int(0);
								break;
							case EnumJSONType::FLOAT:
								param_value = float(0.0f);
								break;
							case EnumJSONType::BOOLEAN:
								param_value = bool(false);
								break;
							case EnumJSONType::JSON_NULL:
								param_value = nullptr;
								break;
							case EnumJSONType::ARRAY:
							case EnumJSONType::OBJECT:
								param_value = Parameters();
								break;
							default:
								break;
							}
						}

						if (is_selected)
							ImGui::SetItemDefaultFocus();

						param_type = SCast(EnumJSONType, selecting_id);
					}
					ImGui::EndCombo();
				}

				ImGui::TableSetColumnIndex(3);

				// パラメータの設定
				// string型
				if (param_type == EnumJSONType::STRING)
				{
					std::string& param_value_str = ACast(std::string&, param_value);
					char param_value_chr[64];
					strcpy_s(param_value_chr, sizeof(param_value_chr), ACast(std::string, param_value).c_str());
					std::string value_id = "Value##" + std::to_string(id);
					if (ImGui::InputText(value_id.c_str(), param_value_chr, sizeof(param_value_chr)))
					{
						param_value_str.assign(param_value_chr);
					}
				}

				// int型
				else if (param_type == EnumJSONType::INTEGER)
				{
					int& param_value_int = ACast(int&, param_value);
					std::string value_id = "Value##" + std::to_string(id);
					if (ImGui::InputInt(value_id.c_str(), &param_value_int))
					{

					}
				}

				// float型
				else if (param_type == EnumJSONType::FLOAT)
				{
					float& param_value_float = ACast(float&, param_value);
					std::string value_id = "Value##" + std::to_string(id);
					if (ImGui::InputFloat(value_id.c_str(), &param_value_float))
					{

					}
				}

				// bool型
				else if (param_type == EnumJSONType::BOOLEAN)
				{
					bool& param_value_bool = ACast(bool&, param_value);
					std::string value_id = "Value##" + std::to_string(id);
					if (ImGui::Checkbox(value_id.c_str(), &param_value_bool))
					{

					}
				}

				// null
				else if (param_type == EnumJSONType::JSON_NULL)
				{
					std::string value_id = "Null##" + std::to_string(id);
					ImGui::Text(value_id.c_str());
				}

				// 配列
				else if (param_type == EnumJSONType::ARRAY)
				{
					std::string value_id = "EditArray##" + std::to_string(id);
					if (ImGui::Button(value_id.c_str()))
					{
						editting_array = true;
						editting_object = false;
						ParamRef array = ACast(ParamRef, param_value);
						editting_parameter_ptr = &array;
						editting_parameter_name = std::to_string(id);
					}
				}

				// オブジェクト
				else if (param_type == EnumJSONType::OBJECT)
				{
					std::string value_id = "EditObject##" + std::to_string(id);
					if (ImGui::Button(value_id.c_str()))
					{
						editting_object = true;
						editting_array = false;
						ParamRef object = ACast(ParamRef, param_value);
						editting_parameter_ptr = &object;
						editting_parameter_name = std::to_string(id);
					}
				}

				ImGui::TableSetColumnIndex(4);

				// パラメータの消去
				std::string delete_id = "Delete##" + std::to_string(id);
				if (ImGui::Button(delete_id.c_str()))
				{
					DeleteParameter(*editting_parameter_ptr, id);
				}
			}
			ImGui::EndTable();
		}

		// Object中の要素追加ボタン
		if (ImGui::Button("Add Object Element"))
		{
			auto& param = editting_parameter_ptr->emplace_back();
			param.PARAM_NAME = "";
			param.PARAM_TYPE = EnumJSONType::JSON_NULL;
			param.PARAM_VALUE = nullptr;
		}

		if (ImGui::Button("Close"))
		{
			editting_object = false;
		}
	}
	ImGui::End();
}

// 指定した配列を操作
void JSONEditor::EditArray()
{
	if (ImGui::Begin("Editting Array", (bool*)0, ImGuiWindowFlags_NoResize))
	{
		// パラメータの設定
		if (ImGui::BeginTable("Parameters", 4, ImGuiTableFlags_Resizable))
		{
			for (int id = 0; id < editting_parameter_ptr->size(); id++)
			{
				// パラメータから各データを取得
				ParamData& param = editting_parameter_ptr->at(id);

				param.PARAM_NAME = std::to_string(id);

				EnumJSONType& param_type = param.PARAM_TYPE;
				std::any& param_value = param.PARAM_VALUE;

				ImGui::TableNextRow();

				// ID表示
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Id %d", id);

				ImGui::TableSetColumnIndex(1);

				// パラメータの型名・設定
				std::string type_id	= "Type##" + std::to_string(id);

				const char* types[] = {
					"String",
					"Integer",
					"Float",
					"Boolean",
					"Null",
					"Array",
					"Object",
				};

				int selecting_id = SCast(int, param_type);
				const char* selecting_type = types[selecting_id];

				if (ImGui::BeginCombo(type_id.c_str(), selecting_type))
				{
					for (int i = 0; i < SCast(int, EnumJSONType::JSON_TYPE_NUM); i++)
					{
						const bool is_selected = (selecting_id == i);
						if (ImGui::Selectable(types[i], is_selected))
						{
							selecting_id = i;

							switch (SCast(EnumJSONType, selecting_id))
							{
							case EnumJSONType::STRING:
								param_value = std::string("");
								break;
							case EnumJSONType::INTEGER:
								param_value = int(0);
								break;
							case EnumJSONType::FLOAT:
								param_value = float(0.0f);
								break;
							case EnumJSONType::BOOLEAN:
								param_value = bool(false);
								break;
							case EnumJSONType::JSON_NULL:
								param_value = nullptr;
								break;
							case EnumJSONType::ARRAY:
							case EnumJSONType::OBJECT:
								param_value = Parameters();
								break;
							default:
								break;
							}
						}

						if (is_selected)
							ImGui::SetItemDefaultFocus();

						param_type = SCast(EnumJSONType, selecting_id);
					}
					ImGui::EndCombo();
				}

				ImGui::TableSetColumnIndex(2);

				// パラメータの設定
				// string型
				if (param_type == EnumJSONType::STRING)
				{
					std::string& param_value_str = ACast(std::string&, param_value);
					char param_value_chr[64];
					strcpy_s(param_value_chr, sizeof(param_value_chr), ACast(std::string, param_value).c_str());
					std::string value_id = "Value##" + std::to_string(id);
					if (ImGui::InputText(value_id.c_str(), param_value_chr, sizeof(param_value_chr)))
					{
						param_value_str.assign(param_value_chr);
					}
				}

				// int型
				else if (param_type == EnumJSONType::INTEGER)
				{
					int& param_value_int = ACast(int&, param_value);
					std::string value_id = "Value##" + std::to_string(id);
					if (ImGui::InputInt(value_id.c_str(), &param_value_int))
					{

					}
				}

				// float型
				else if (param_type == EnumJSONType::FLOAT)
				{
					float& param_value_float = ACast(float&, param_value);
					std::string value_id = "Value##" + std::to_string(id);
					if (ImGui::InputFloat(value_id.c_str(), &param_value_float))
					{

					}
				}

				// bool型
				else if (param_type == EnumJSONType::BOOLEAN)
				{
					bool& param_value_bool = ACast(bool&, param_value);
					std::string value_id = "Value##" + std::to_string(id);
					if (ImGui::Checkbox(value_id.c_str(), &param_value_bool))
					{

					}
				}

				// null
				else if (param_type == EnumJSONType::JSON_NULL)
				{
					std::string value_id = "Null##" + std::to_string(id);
					ImGui::Text(value_id.c_str());
				}

				// 配列
				else if (param_type == EnumJSONType::ARRAY)
				{
					std::string value_id = "EditArray##" + std::to_string(id);
					if (ImGui::Button(value_id.c_str()))
					{
						editting_array = true;
						editting_object = false;
						ParamRef array = ACast(ParamRef, param_value);
						editting_parameter_ptr = &array;
						editting_parameter_name = std::to_string(id);
					}
				}

				// オブジェクト
				else if (param_type == EnumJSONType::OBJECT)
				{
					std::string value_id = "EditObject##" + std::to_string(id);
					if (ImGui::Button(value_id.c_str()))
					{
						editting_object = true;
						editting_array = false;
						ParamRef object = ACast(ParamRef, param_value);
						editting_parameter_ptr = &object;
						editting_parameter_name = std::to_string(id);
					}
				}

				ImGui::TableSetColumnIndex(3);

				// パラメータの消去
				std::string delete_id = "Delete##" + std::to_string(id);
				if (ImGui::Button(delete_id.c_str()))
				{
					DeleteParameter(*editting_parameter_ptr, id);
				}
			}
			ImGui::EndTable();
		}

		// Object中の要素追加ボタン
		if (ImGui::Button("Add Array Element"))
		{
			auto& param = editting_parameter_ptr->emplace_back();
			param.PARAM_TYPE	= EnumJSONType::JSON_NULL;
			param.PARAM_VALUE	= nullptr;
		}

		if (ImGui::Button("Close"))
		{
			editting_array = false;
		}
	}
	ImGui::End();
}

// JSONを文字列として書き出す
void JSONEditor::BakeToJSON()
{
	json json_data = {};

	RecursionAddToJSON(json_data, parameters, EnumJSONType::UNDEFINE);

	json_str = json_data.dump(4);
}

// JSONの文字列をJSONファイルとして出力する
void JSONEditor::OutputJSON()
{
	std::ofstream output;
	output.open(output_path, std::ios::out);
	output << json_str << std::endl;
	output.close();
}

// JSONファイルを読み込む再帰関数
void JSONEditor::RecursionReadJSON(json& json_root, ParamPtr param_root, std::string before_key)
{
	JSONItr begin = json_root.begin();
	JSONItr end = json_root.end();

	// JSONオブジェクト内を読み込み
	int	count = 0;
	for (JSONItr json_itr = begin; json_itr != end; json_itr++)
	{
		json		data		= *json_itr;
		auto		root_type	= json_root.type();
		auto		data_type	= data.type();
		auto&		param		= param_root->emplace_back();

		std::string key;

		if(root_type == json::value_t::object)
		{
			key.assign(json_itr.key());
		}
		else
		{
			key.assign(std::to_string(count));
			count++;
		}

		// 再帰関数を使って配列読み込み
		if (data_type == nlohmann::json::value_t::array)
		{
			param.PARAM_NAME	= key;
			param.PARAM_TYPE	= EnumJSONType::ARRAY;

			Parameters array;
			RecursionReadJSON(data, &array, key);

			param.PARAM_VALUE = Parameters(array);
		}

		// 再帰関数を使ってオブジェクト読み込み
		else if (data_type == nlohmann::json::value_t::object)
		{
			param.PARAM_NAME	= key;
			param.PARAM_TYPE	= EnumJSONType::OBJECT;

			Parameters object;
			RecursionReadJSON(data, &object, key);
			
			param.PARAM_VALUE = Parameters(object);
		}

		// JSONデータの中身を取得
		else
		{
			param.PARAM_NAME = key;

			if (data.is_string())
			{
				param.PARAM_TYPE = EnumJSONType::STRING;
				param.PARAM_VALUE = 
					root_type == json::value_t::array ?
					std::string(json_root[std::stoi(key)]) :
					std::string(json_root[key]);
			}
			else if (data.is_number_integer())
			{
				param.PARAM_TYPE = EnumJSONType::INTEGER;
				param.PARAM_VALUE =
					root_type == json::value_t::array ?
					int(json_root[std::stoi(key)]) :
					int(json_root[key]);
			}
			else if (data.is_number_float())
			{
				param.PARAM_TYPE = EnumJSONType::FLOAT;
				param.PARAM_VALUE =
					root_type == json::value_t::array ?
					float(json_root[std::stoi(key)]) :
					float(json_root[key]);
			}
			else if (data.is_boolean())
			{
				param.PARAM_TYPE = EnumJSONType::BOOLEAN;
				param.PARAM_VALUE =
					root_type == json::value_t::array ?
					bool(json_root[std::stoi(key)]) :
					bool(json_root[key]);
			}
			else if (data.is_null())
			{
				param.PARAM_TYPE = EnumJSONType::JSON_NULL;
				param.PARAM_VALUE = NULL;
			}
		}
	}
}

// JSONオブジェクトに追加していく再帰関数
void JSONEditor::RecursionAddToJSON(json& parent, ParamRef values, EnumJSONType before_type)
{
	for(auto& value : values)
	{
		// 配列の場合
		if(before_type == EnumJSONType::ARRAY)
		{
			if (value.PARAM_NAME.empty())	continue;

			int index = std::stoi(value.PARAM_NAME);
			std::string type_name = value.PARAM_VALUE.type().name();

			if (value.PARAM_NAME.empty())	continue;

			if (type_name == STRING_NAME)
				parent[index] = ACast(std::string, value.PARAM_VALUE);

			else if (type_name == INT_NAME)
				parent[index] = ACast(int, value.PARAM_VALUE);

			else if (type_name == FLOAT_NAME)
				parent[index] = ACast(float, value.PARAM_VALUE);

			else if (type_name == BOOL_NAME)
				parent[index] = ACast(bool, value.PARAM_VALUE);

			else if (type_name == NULLPTR_NAME)
				parent[index] = nullptr;

			else if (type_name == PARAMREF_NAME)
				RecursionAddToJSON(parent[index], ACast(ParamRef, value.PARAM_VALUE), value.PARAM_TYPE);
		}

		// パラメータ名が数字では無かった時
		else
		{
			if (value.PARAM_NAME.empty())	continue;

			std::string name = value.PARAM_NAME;
			if (name.empty())	continue;

			std::string type_name = value.PARAM_VALUE.type().name();

			if(type_name == STRING_NAME)
				parent[name] = ACast(std::string, value.PARAM_VALUE);

			else if (type_name == INT_NAME)
				parent[name] = ACast(int, value.PARAM_VALUE);
			
			else if (type_name == FLOAT_NAME)
				parent[name] = ACast(float, value.PARAM_VALUE);
			
			else if (type_name == BOOL_NAME)
				parent[name] = ACast(bool, value.PARAM_VALUE);
			
			else if (type_name == NULLPTR_NAME)
				parent[name] = nullptr;
			
			else if (type_name == PARAMREF_NAME)
				RecursionAddToJSON(parent[name], ACast(ParamRef, value.PARAM_VALUE), value.PARAM_TYPE);
		}
	}
}

// JSONファイルを読み込む
void JSONEditor::ImportJSON(std::filesystem::path json_path, ParamPtr parameter_ptr)
{
	ParamRef param_ref = parameter_ptr ? *parameter_ptr : parameters;

	for (auto& param : param_ref)
	{
		param.PARAM_VALUE.reset();
	}
	param_ref.clear();
	param_ref.resize(0);

	std::ifstream input_path(json_path);

	if (input_path.fail())
	{
		std::ofstream new_file(json_path);
		return;
	}
	else if (std::filesystem::file_size(json_path) == 0)
	{
		return;
	}

	auto json_data = json::parse(input_path);
	RecursionReadJSON(json_data, &param_ref);
	json_data.clear();
}

// パラメータを削除する
void JSONEditor::DeleteParameter(ParamRef delete_parameters, int index)
{
	delete_parameters.erase(delete_parameters.begin() + index);
}

// パラメータを追加する
void JSONEditor::AddParameter(std::string param_name, std::any* initial_data, EnumJSONType initial_type, ParamPtr parameter_ptr)
{
	ParamRef param_ref = parameter_ptr ? *parameter_ptr : parameters;

	auto& param = param_ref.emplace_back();
	param.PARAM_NAME = param_name;

	if (initial_type == EnumJSONType::UNDEFINE)
	{
		const std::string type_name = initial_data->type().name();

		if		(type_name == STRING_NAME)	param.PARAM_TYPE = EnumJSONType::STRING;
		else if (type_name == INT_NAME)		param.PARAM_TYPE = EnumJSONType::INTEGER;
		else if (type_name == FLOAT_NAME)	param.PARAM_TYPE = EnumJSONType::FLOAT;
		else if (type_name == BOOL_NAME)	param.PARAM_TYPE = EnumJSONType::BOOLEAN;
		else if (type_name == NULLPTR_NAME)	param.PARAM_TYPE = EnumJSONType::JSON_NULL;
		else
		{
			if(IsArray(param.PARAM_NAME))
			{
				param.PARAM_TYPE = EnumJSONType::ARRAY;
			}
			else
			{
				param.PARAM_TYPE = EnumJSONType::OBJECT;
			}
		}
	}
	else
		param.PARAM_TYPE = initial_type;

	if (initial_data)
		param.PARAM_VALUE = *initial_data;
}

// パラメータを変更する
void JSONEditor::SetParameter(std::string param_name, std::any& new_param, bool change_type, ParamPtr parameter_ptr)
{
	ParamRef param_ref = parameter_ptr ? *parameter_ptr : parameters;

	const std::string type_name = new_param.type().name();
	for (auto& param : param_ref)
	{
		if (param_name == param.PARAM_NAME)
		{
			if (change_type)
			{
				if		(type_name == STRING_NAME)	param.PARAM_TYPE = EnumJSONType::STRING;
				else if (type_name == INT_NAME)		param.PARAM_TYPE = EnumJSONType::INTEGER;
				else if (type_name == FLOAT_NAME)	param.PARAM_TYPE = EnumJSONType::FLOAT;
				else if (type_name == BOOL_NAME)	param.PARAM_TYPE = EnumJSONType::BOOLEAN;
				else if (type_name == NULLPTR_NAME)	param.PARAM_TYPE = EnumJSONType::JSON_NULL;
				else
				{
					if (IsArray(param.PARAM_NAME))
					{
						param.PARAM_TYPE = EnumJSONType::ARRAY;
					}
					else
					{
						param.PARAM_TYPE = EnumJSONType::OBJECT;
					}
				}

				param.PARAM_VALUE = new_param;
			}
			else
			{
				if (param.PARAM_VALUE.type().name() == type_name)
				{
					param.PARAM_VALUE = new_param;
					return;
				}
			}
		}
	}
}