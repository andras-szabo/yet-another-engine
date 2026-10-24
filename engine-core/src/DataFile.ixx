module;

#include "engine_core_api.h"

export module DataFile;

#if defined ( __INTELLISENSE__ )
#include <string>
#include <memory>
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

	struct DataFile_Impl;

	export class ENGINE_CORE_API DataFile
	{
	public:
		static std::string EscapeString(const std::string& str);
		static bool Serialize(const DataFile& data, const std::string& fileName, const std::string& indent = "  ");
		static Expected<DataFile> Deserialize(const std::string& fileName);
		static Expected<DataFile> FromStream(std::istream& stream);
		static Expected<DataFile> FromString(const std::string& asString);

		DataFile();
		~DataFile();
		DataFile(const DataFile& other) = delete;
		DataFile& operator=(const DataFile& other) = delete;

		DataFile& operator=(DataFile&& other) noexcept;
		DataFile(DataFile&& other) noexcept;

		explicit DataFile(std::size_t expectedChildrenCount);

		DataFile& operator=(float f);
		DataFile& operator=(int i);
		DataFile& operator=(const std::string& s);
		DataFile& operator=(const ISerializable& serializable);

		DataFile& operator[](std::string_view name);
		const DataFile& operator[](std::string_view name) const;
		const DataFile& at(std::string_view name) const;

		DataFile DeepCopy() const;

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
		std::vector<const DataFile*> GetChildren() const;
		std::vector<std::string> GetChildrenNames() const;

		Engine::Expected<std::string> TryGetString(int index = 0) const;
		Engine::Expected<int> TryGetInt(int index = 0) const;

	private:

// The implementation is hidden to avoid exposing the internal structure of DataFile.
// Why the warning suppression? Because std::unique_ptr<DataFile_Impl> is not exported 
// from the DLL, and MSVC warns about that, thinking this is an ABI risk: a client
// built with a different STL or compiler version could see a different layout
// for unique_ptr. (Client sees sizeof(DataFile), and the member layout, and
// compiles inline code against them. If the client's STL differs from the DLL's,
// the two can disagree. That's a real problem for std::vector or std::string
// members, because their layout changes with _ITERATOR_DEBUG_LEVEL and the STL version.
//
// But:
// - A unique_ptr with the default deleter is one pointer on MSVC STL, libstdc++ and libc++.
// There's no debug-mode variation like vector has, so sizeof(DataFile) is the same
// everywhere.
// - No inline access: if the dtor, copy and move are all defined in DataFile.cpp,
// then client code never instantiates unique_ptr or default_delete. The allocation
// and deallocation both happen inside the DLL, so there's no cross-heap or cross-STL
// deletion.
#pragma warning(push)
#pragma warning(disable: 4251) 
		std::unique_ptr<DataFile_Impl> _impl;
#pragma warning(pop)
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
