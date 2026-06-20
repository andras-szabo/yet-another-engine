module;

#include "engine_core_api.h"

export module DataFile;

#if defined ( __INTELLISENSE__ )
#include <string>
#include <unordered_map>
#include <vector>
#include "EngineError.ixx"
#else
import Error;
import std;
#endif

namespace Engine
{
	const std::string EMPTY_STRING{ "" };

	/* StringViewHash and StringViewEq make it possible to look up things in the unordered map
	   that contains std::strings using a string_view.
	   "using is_transparent = void" is a marker type; it signals to the compiler that the associative
	   container supports heterogeneous lookup.
	   When present, overloads accept types other than the container's key type, and will call
	   the comparator / hash / equality function with those mixed types. 
	   
	   And the return function in StringViewHash? It constructs a temporary hash<string_view> object, and
	   immediately invokes it with the passed-in string view. So it's brace initialization followed
	   by immediate invocation. Gotta love C++. */

	export struct ENGINE_CORE_API StringViewHash
	{
		using is_transparent = void;

		std::size_t operator()(std::string_view sv) const noexcept;
		std::size_t operator()(const std::string& s) const noexcept;
	};

	export struct ENGINE_CORE_API StringViewEq
	{
		using is_transparent = void;

		template <typename A, typename B>
		bool operator()(const A& a, const B& b) const noexcept
		{
			return std::string_view(a) == std::string_view(b);
		}
	};

	class ISerializable;
	class DataFile;

	struct DataFile_Impl
	{
		std::vector<std::string> content;
		std::vector<DataFile> children;
		std::vector<std::string> childrenNames;
		std::unordered_map<std::string, int, StringViewHash, StringViewEq> childIndexByName;
	};

	export class ENGINE_CORE_API DataFile
	{
	public:
		static std::string EscapeString(const std::string& str);
		static bool Serialize(const DataFile& data, const std::string& fileName, const std::string& indent = "  ");
		static Expected<DataFile> Deserialize(const std::string& fileName);
		static Expected<DataFile> FromStream(std::istream& stream);
		static Expected<DataFile> FromString(const std::string& asString);

		DataFile() = default;
		explicit DataFile(std::size_t expectedChildrenCount);
		DataFile& operator=(float f);
		DataFile& operator=(int i);
		DataFile& operator=(const std::string& s);
		DataFile& operator=(const ISerializable& serializable);

		DataFile& operator[](std::string_view name);
		const DataFile& operator[](std::string_view name) const;
		const DataFile& at(std::string_view name) const;

		bool IsEmpty() const;
		bool HasChild(std::string_view name) const;

		void SetString(const std::string& str, int index = 0);
		void SetString(const std::string_view str, int index = 0);
		void SetInt(int i, int index = 0);
		void SetUInt(unsigned int i, int index = 0);
		void SetULong(unsigned long long ulong, int index = 0);
		void SetFloat(float f, int index = 0);
		void SetFloats(int count, ...);

		void CreateLeaf() const;
		void Clear();

		std::size_t GetValueCount() const;
		const std::string& GetString(int index = 0) const;
		int GetInt(int index = 0) const;
		unsigned int GetUInt(int index = 0) const;
		unsigned long long GetULong(int index = 0) const;
		float GetFloat(int index = 0) const;
		std::string ToString(const std::string& indent = "  ") const;
		std::vector<DataFile> GetChildren() const;
		std::vector<std::string> GetChildrenNames() const;

		Engine::Expected<std::string> TryGetString(int index = 0) const;
		Engine::Expected<int> TryGetInt(int index = 0) const;

	private:
		DataFile_Impl _impl;
	};

	export class ENGINE_CORE_API ISerializable
	{
	public:
		virtual ~ISerializable() = default;
		virtual ISerializable& operator=(const DataFile& df) = 0;

		virtual void ToDataFile(DataFile& df) const = 0;
		virtual void FromDataFile(const DataFile& df) = 0;
	};
} // namespace Engine
