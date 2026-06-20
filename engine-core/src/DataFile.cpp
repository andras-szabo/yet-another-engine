module;

#include <cassert>
#include <cstdarg>

module DataFile;

#if defined ( __INTELLISENSE__ )
#include <format>
#include <fstream>
#include <functional>
#include <sstream>
#include <stack>
#include <string>

#include "DataFile.ixx"
#include "EngineError.ixx"
#else
import Error;
import std;
#endif

namespace Engine
{
	std::size_t StringViewHash::operator()(std::string_view sv) const noexcept
	{
		return std::hash<std::string_view>{}(sv);
	}

	std::size_t StringViewHash::operator()(const std::string& s) const noexcept
	{
		return std::hash<std::string_view>{}(s);
	}

	DataFile& DataFile::operator=(float f)
	{
		SetFloat(f);
		return *this;
	}

	DataFile& DataFile::operator=(int i)
	{
		SetInt(i);
		return *this;
	}

	DataFile& DataFile::operator=(const std::string& s)
	{
		SetString(s);
		return *this;
	}

	DataFile& DataFile::operator=(const ISerializable& serializable)
	{
		serializable.ToDataFile(*this);
		return *this;
	}

	std::string DataFile::EscapeString(const std::string& str)
	{
		std::string escaped;
		const char separator{ ',' };

		for (const auto c : str)
		{
			if (c == '\n') { escaped.append("\\n"); }
			else if (c == '\r') { escaped.append("\\r"); }
			else if (c == '"') { escaped.append("\\\""); }
			else if (c == separator) { escaped.append("\\" + std::string(1, separator)); }
			else escaped += c;
		}

		return escaped;
	}

	bool DataFile::Serialize(const DataFile& data,
		const std::string& fileName,
		const std::string& indent)
	{
		const std::string comma{ ", " };

		std::function<void(const DataFile&, std::ofstream&, int)> Write = [&]
		(const DataFile& d, std::ofstream& f, int currentIndent)
			{
				auto indentStr = [&](int count, const std::string_view indent_) -> std::string
					{
						std::string is;
						for (int i = 0; i < count; ++i)
						{
							is += indent_;
						}
						return is;
					};

				// Write contents
				const auto contentSize = d.GetValueCount();
				if (contentSize == 0 && d._impl.children.empty())
				{
					return;
				}

				if (contentSize > 0)
				{
					f << indentStr(currentIndent, indent);

					for (auto i = 0; i < contentSize; ++i)
					{
						if (i > 0)
						{
							f << comma;
						}

						f << EscapeString(d.GetString(i));
					}

					f << "\n";
				}

				int childIndex = 0;
				for (const auto& childNode : d._impl.children)
				{
					f << indentStr(currentIndent, indent) << "[" << d._impl.childrenNames.at(childIndex++) << "]\n";
					f << indentStr(currentIndent, indent) << "{\n";
					Write(childNode, f, currentIndent + 1);
					f << indentStr(currentIndent, indent) << "}\n";
				}
			};

		// TODO - validate path
		//		-> e.g. it doesn't contain invalid chars "<>:\"|?*";
		//				it isn't over windows' MAX_PATH (260 char GHCPSZ)
		//				it doesn't contain directory traversal ("..")
		//				it is not the root directory, and the path actually exists

		if (std::ofstream file(fileName); file.is_open())
		{
			Write(data, file, 0);
			return true;
		}

		return false;
	}

	Expected<DataFile> DataFile::FromStream(std::istream& stream)
	{
		DataFile df(8192);

		std::stack<std::reference_wrapper<DataFile>> stk;
		stk.push(df);

		auto trim = [](std::string_view& s)
			{
				const auto start = s.find_first_not_of(" \t\n\r\f\v");
				const auto end = s.find_last_not_of(" \t\n\r\f\v");
				s = (start == std::string_view::npos || end == std::string_view::npos) ?
					std::string_view{} : s.substr(start, end - start + 1);
			};

		std::string line;
		while (std::getline(stream, line))
		{
			if (stk.empty())
			{
				break;
			}

			std::string_view lineView = line;
			if (lineView.empty())
			{
				continue;
			}

			trim(lineView);
			const bool isNode = lineView.front() == '[' && lineView.back() == ']';

			if (isNode)
			{
				const std::string nodeName(lineView.substr(1, lineView.size() - 2));
				auto& p = stk.top().get()[std::string_view{ nodeName }];
				stk.push(p);
			}
			else
			{

				if (line == "{") { continue; }
				if (line == "}") { stk.pop(); continue; }

				auto& currentNode = stk.top().get();
				std::istringstream asStringStream{ line };

				for (std::string value; std::getline(asStringStream, value, ',');)
				{
					const auto index = static_cast<int>(currentNode.GetValueCount());
					trim(lineView = value);
					currentNode.SetString(std::string(lineView), index);
				}
			}
		}

		return df;
	}

	Expected<DataFile> DataFile::FromString(const std::string& asString)
	{
		std::stringstream stream{ asString };
		return FromStream(stream);
	}

	Expected<DataFile> DataFile::Deserialize(const std::string& fileName)
	{
		if (std::ifstream file(fileName); file.is_open())
		{
			auto df = FromStream(file);
			file.close();
			return df;
		}

		return Engine::Unexpected({ Engine::ErrorType::File, "Couldn't open file" });
	}

	DataFile::DataFile(std::size_t expectedChildrenCount)
	{
		_impl.children.reserve(expectedChildrenCount);
		_impl.childIndexByName.reserve(expectedChildrenCount);
	}

	DataFile& DataFile::operator[](std::string_view name)
	{
		auto item = _impl.childIndexByName.find(name);

		if (item == _impl.childIndexByName.end())
		{
			_impl.childrenNames.push_back(name.data());
			auto nameAsString = _impl.childrenNames.back();
			_impl.childIndexByName[nameAsString] = static_cast<int>(_impl.children.size());
			_impl.children.emplace_back();

			return _impl.children.back();
		}

		return _impl.children[(*item).second];
	}

	const DataFile& DataFile::operator[](std::string_view name) const
	{
		return at(name);
	}

	const DataFile& DataFile::at(std::string_view name) const				// Will throw if key not present.
	{
		const auto item = _impl.childIndexByName.find(name);
		return _impl.children[(*item).second];
	}

	bool DataFile::IsEmpty() const
	{
		return _impl.content.size() == (std::size_t)0 && _impl.children.size() == (std::size_t)0;
	}

	std::size_t DataFile::GetValueCount() const
	{
		return _impl.content.size();
	}

	void DataFile::SetInt(int i, int index)
	{
		SetString(std::to_string(i), index);
	}

	void DataFile::SetUInt(unsigned int i, int index)
	{
		SetString(std::to_string(i), index);
	}

	void DataFile::SetULong(unsigned long long i, int index)
	{
		SetString(std::to_string(i), index);
	}

	void DataFile::SetFloat(float f, int index)
	{
		SetString(std::to_string(f), index);
	}

	void DataFile::SetFloats(int count, ...)
	{
		std::va_list args;
		va_start(args, count);
		for (int i = 0; i < count; ++i)
		{
			SetFloat(va_arg(args, float), i);
		}
		va_end(args);
	}

	void DataFile::CreateLeaf() const
	{
		// When used with the overloaded [] operators, this can be used
		// to construct an empty leaf node:
		// e.g. udf[Foo][Bar][Baz].CreateLeaf() creates the Foo -> Bar -> Baz
		// relationship, with Baz being an empty leaf node.
	}

	bool DataFile::HasChild(std::string_view name) const
	{
		return _impl.childIndexByName.find(name) != _impl.childIndexByName.end();
	}

	std::vector<DataFile> DataFile::GetChildren() const
	{
		return _impl.children;
	}

	std::vector<std::string> DataFile::GetChildrenNames() const
	{
		return _impl.childrenNames;
	}

	void DataFile::Clear()
	{
		_impl.content.clear();
		_impl.children.clear();
		_impl.childrenNames.clear();
		_impl.childIndexByName.clear();
	}

	void DataFile::SetString(const std::string_view strView, int index)
	{
		SetString(std::string{ strView }, index);
	}

	void DataFile::SetString(const std::string& str, int index)
	{
		if (0 <= index)
		{
			if (index >= static_cast<int> (_impl.content.size()))
			{
				_impl.content.resize(index + 1);
			}

			_impl.content[index] = str;
		}
	}

	Engine::Expected<std::string> DataFile::TryGetString(int index) const
	{
		if (0 <= index && index < _impl.content.size())
		{
			return _impl.content[index];
		}

		return Engine::Unexpected({ Engine::ErrorType::OutOfBounds, std::format("Read error at index {}", index) });
	}

	const std::string& DataFile::GetString(int index) const
	{
		if (0 <= index && index < _impl.content.size())
		{
			return _impl.content[index];
		}

		return EMPTY_STRING;
	}

	Engine::Expected<int> DataFile::TryGetInt(int index) const
	{
		if (0 <= index && index < _impl.content.size())
		{
			return GetInt(index);
		}

		return Engine::Unexpected({ Engine::ErrorType::OutOfBounds, std::format("Read error at index {}", index) });
	}

	int DataFile::GetInt(int index) const
	{
		return std::stoi(GetString(index));
	}

	unsigned int DataFile::GetUInt(int index) const
	{
		const unsigned long ul = std::stoul(GetString(index));
		assert(ul <= std::numeric_limits<unsigned int>::max());
		return static_cast<unsigned int>(ul);
	}

	unsigned long long DataFile::GetULong(int index) const
	{
		return std::stoull(GetString(index));
	}

	float DataFile::GetFloat(int index) const
	{
		return std::stof(GetString(index));
	}

	std::string DataFile::ToString(const std::string& indent) const
	{
		const std::string comma{ ", " };

		std::function<void(const DataFile&, std::ostream&, int)> Write = [&]
		(const DataFile& d, std::ostream& f, int currentIndent)
			{
				auto indentStr = [&](int count, const std::string& indent) -> std::string
					{
						std::string is;
						for (int i = 0; i < count; ++i)
						{
							is += indent;
						}
						return is;
					};

				// Write contents
				if (d.GetValueCount() == 0 && d._impl.children.size() == 0)
				{
					return;
				}

				const auto contentSize = d.GetValueCount();
				if (contentSize > 0)
				{
					f << indentStr(currentIndent, indent);

					for (auto i = 0; i < contentSize; ++i)
					{
						if (i > 0)
						{
							f << comma;
						}

						f << EscapeString(d.GetString(i));
					}

					f << "\n";
				}

				int  childIndex = 0;
				for (const auto& childNode : d._impl.children)
				{
					f << indentStr(currentIndent, indent) << "[" << d._impl.childrenNames.at(childIndex++) << "]\n";
					f << indentStr(currentIndent, indent) << "{\n";
					Write(childNode, f, currentIndent + 1);
					f << indentStr(currentIndent, indent) << "}\n";
				}
			};

		std::stringstream stream;
		Write(*this, stream, 0);
		return stream.str();
	}
} // namespace Engine