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
		GUID Guid() const;
		T* Ptr();
		
		void Resolve();

	private:
		GUID guid;
		std::shared_ptr<T> ptr;
	};

	template <typename T>
	void AssetRef<T>::Resolve()
	{
		// Well, that's the rub.
		//
		// it would be something like:
		//
		// = dear asset database, load this asset (?)
		//		= so for this to work, we need to store data in a type erased way,
		//		  and then register a function that knows how to turn a void* into
		//		  a T*.
		//			Claude: "resolving a T* from a GUID requires the engine to know
		//					 how to construct a T from asset data - and engine-core can't have
		//					 compiled-in knowledge of user-defined asset types.
		//					 Recommendation: mirror ComponentRegistry.ixx: that file already
		//				     solves this for components: a type-erased unordered_map<unsinged int,
		//					 ComponentFactoryFn> populated by RegisterComponent(typeID, factory)
		//					 calls from the game DLL, exposed via GlobalCopmonentRegistry(). Do
		//					 the same for assets:
		// 
		//					 - Keep AssetRef<T> as the public-facing template: stores GUID,
		//					   a cached T*, and calls Resolve(). 
		//					 - internally, AssetRef<T>::Resolve() asks a type-erased
		//					   AssetLoaderRegistry (keyed by an asset type ID, same idea as
		//					   ComponentRegistry) for a void*; then static_casts it into a T*.
		//					 - users register a loader function for their custom asset type from
		//					   their DLL (RegisterAssetLoader<MyAsset>(typeID, loadFn), exactly
		//					   like RegisterComponent. T B C
		// = wait for loading
		// = assign to the shared ptr
		// = done.
		//
		// Eh. But, then, shouldn't the assetDatabase own an assetRef?
				// think about this first.
	}

	template <typename T>
	GUID AssetRef<T>::Guid() const
	{
		return guid;
	}

	template <typename T>
	T* AssetRef<T>::Ptr()
	{
		return ptr.get();
	}

} // namespace Engine