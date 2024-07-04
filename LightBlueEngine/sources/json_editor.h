#ifndef __JSON_EDITOR_H__
#define __JSON_EDITOR_H__

// <>インクルード
#include <any>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <json.hpp>

// ""インクルード
#include "singleton.h"

// マクロ
#define PARAM_NAME	_Myfirst._Val
#define PARAM_TYPE	_Get_rest()._Myfirst._Val
#define PARAM_VALUE	_Get_rest()._Get_rest()._Myfirst._Val

#define GET_VALUE_FROM_ANY_PTR(object, type, index) (std::any_cast<type*>(object))[index]

#define GET_PARAMETER(name, type) \
	JSONEditor::GetInstance()->GetParameter(name) != nullptr ? \
	ACast(type, JSONEditor::GetInstance()->GetParameter(name)) : \
	nullptr

#define GET_PARAMETER_IN_PARAMPTR(name, type, param_ptr) \
	JSONEditor::GetInstance()->GetParameter(name, param_ptr) != nullptr ? \
	ACast(type, JSONEditor::GetInstance()->GetParameter(name, param_ptr)) : \
	nullptr

// define定数
#define STRING_NAME		"class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >"
#define INT_NAME		"int"
#define FLOAT_NAME		"float"
#define BOOL_NAME		"bool"
#define NULLPTR_NAME	"std::nullptr_t"
#define PARAMETERS_NAME	"class std::tuple<class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >,enum EnumJSONType,class std::any>"
#define PARAMREF_NAME	"class std::vector<class std::tuple<class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >,enum EnumJSONType,class std::any>,class std::allocator<class std::tuple<class std::basic_string<char,struct std::char_traits<char>,class std::allocator<char> >,enum EnumJSONType,class std::any> > >"

// enum class
enum class EnumJSONType
{
	STRING,
	INTEGER,
	FLOAT,
	BOOLEAN,
	JSON_NULL,
	ARRAY,
	OBJECT,

	JSON_TYPE_NUM,
	UNDEFINE
};

// using
using ParamData		= std::tuple<std::string, EnumJSONType, std::any>;
using JSONItr		= nlohmann::json::iterator;
using JSONType		= nlohmann::json::value_t;
using Parameters	= std::vector<ParamData>;
using ParamRef		= std::vector<ParamData>&;
using ParamPtr		= std::vector<ParamData>*;

template<typename T>
using UPtrVector = std::vector<std::unique_ptr<T>>;

// 関数
static bool IsArray(std::string key)
{
	if (std::all_of(key.begin(), key.end(), isdigit))
		return true;

	return false;
}

// class >> [JSONData]
class JSONEditor : public Singleton<JSONEditor>
{
public:
	// public:コンストラクタ・デストラクタ
	JSONEditor() {};
	~JSONEditor() {};

	// public:通常関数
	void Initialize(std::string = "*.json");
	void DebugGUI();
	void EditObject();
	void EditArray();
	void DeleteParameter(ParamRef, int);
	void BakeToJSON();
	void OutputJSON();
	void RecursionReadJSON(nlohmann::json&, ParamPtr, std::string = "");
	void RecursionAddToJSON(nlohmann::json&, ParamRef, EnumJSONType);

	// public:ゲッター関数
	std::any* GetParameter(std::string param_name, ParamPtr parameter_ptr = nullptr)
	{
		if(parameter_ptr)
		{
			for (auto& param : *parameter_ptr)
			{
				if (param_name == param.PARAM_NAME)
				{
					return &param.PARAM_VALUE;
				}
			}
		}
		else
		{
			for (auto& param : parameters)
			{
				if (param_name == param.PARAM_NAME)
				{
					return &param.PARAM_VALUE;
				}
			}
		}
		return nullptr;
	}

	// public:セッター関数
	void ImportJSON(std::filesystem::path, ParamPtr = nullptr);	// インポート後のパラメータ配列のポインタも取得できるためゲッター関数も兼ねる
	void AddParameter(std::string, std::any* = nullptr, EnumJSONType = EnumJSONType::UNDEFINE, ParamPtr = nullptr);
	void SetParameter(std::string, std::any&, bool = false, ParamPtr = nullptr);

private:
	// public:変数
	bool						editting_object = false;
	bool						editting_array	= false;
	bool						auto_update		= true;
	std::string					json_str;
	std::string					editting_parameter_name;
	std::filesystem::path		output_path;

	Parameters					parameters;
	ParamPtr					editting_parameter_ptr;
};

#endif // __JSON_EDITOR_H__