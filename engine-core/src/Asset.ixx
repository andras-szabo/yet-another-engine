module;

export module Asset;

#if defined ( __INTELLISENSE__ )
#include <memory>

#include "GUID.ixx"
#else
import GUID;
import std;
#endif;

namespace Engine
{
	export enum class AssetType
	{
		Undefined = 0,

		Scene = 1,
		Texture = 2,
		StaticMesh = 3,
	};

	export template<typename T>
	class AssetRef
	{
	public:
		AssetRef(GUID guid) : _guid(guid)
		{}

		GUID Guid() const;
		T* Ptr();

		void Assign(std::shared_ptr<T> ptr) noexcept 
		{ 
			if (!ptr) { _ptr.reset();}
			else { _ptr = ptr; }
		}

	private:
		GUID _guid;
		std::shared_ptr<T> _ptr { nullptr };
	};

	template <typename T>
	GUID AssetRef<T>::Guid() const
	{
		return _guid;
	}

	template <typename T>
	T* AssetRef<T>::Ptr()
	{
		return _ptr.get();
	}

} // namespace Engine