module;

#include "engine_core_api.h"
#include "LoggerMacros.h"

export module AssetDatabase;

#if defined ( __INTELLISENSE__ )
#include <filesystem>
#include <future>
#include <span>
#include <unordered_map>

#include "Asset.ixx"
#include "EngineError.ixx"
#include "FileWatcher.ixx"
#include "Guid.ixx"
#include "Logger.ixx"
#else
import Asset;
import GUID;
import Error;
import FileWatcher;
import Logger;

import std;
#endif

namespace Engine
{
	/// <summary>
	/// A function that, given a path, reads the file, and turns it into some kind of
	/// deserialized blob of data, which you'll be able to cast to a specific type of
	/// asset with static_pointer_cast.
	/// </summary>
	export using AssetLoaderFn = std::shared_ptr<void>(*)(const std::string_view path);

	/// <summary>
	/// A function that, given a path and a type-erased pointer to an asset, serializes
	/// the asset and writes it to disk at the given path.
	/// </summary>
	export using AssetSaverFn = Engine::Expected<void>(*)(const std::string_view path, const std::shared_ptr<void> assetPtr);

	/// <summary>
	/// A function that creates a new asset of some type, returning a type-erased pointer
	/// to the newly created asset.
	/// </summary>
	export using AssetCreatorFn = Engine::Expected<std::shared_ptr<void>>(*)();

	struct AssetDatabase_Impl;

	export class ENGINE_CORE_API AssetDatabase
	{
	private:	
		AssetDatabase_Impl* _impl { nullptr };

	public:
		AssetDatabase();
		~AssetDatabase();

		AssetDatabase(const AssetDatabase& other) = delete;
		AssetDatabase& operator=(const AssetDatabase& other) = delete;

		AssetDatabase(AssetDatabase&& other);
		AssetDatabase& operator=(AssetDatabase&& other);

		int GetAssetCount() const;

		Engine::Expected<void> PopulateFromFolder(const std::filesystem::path& path);
		Engine::AssetType IsAssetFile(const std::filesystem::directory_entry& directoryEntry) const;

		void Update(std::span<const Engine::FileChangeEvent> fileChangeEvents);

		Engine::Expected<void> TryAddNewAsset(const std::filesystem::directory_entry& entry, AssetType assetType);
		Engine::Expected<void> TryRemoveAsset(const std::filesystem::directory_entry& entry, AssetType assetType);
		Engine::Expected<void> TryRenameAsset(const std::filesystem::directory_entry& entry, 
			const std::filesystem::directory_entry& oldEntry,
			AssetType assetType);

		Engine::Expected<Engine::GUID> CreateNewAsset(const std::filesystem::path& path, 
			AssetType assetType, 
			std::optional<Engine::GUID> guid = std::nullopt);

		const std::unordered_map<std::string, Engine::GUID>& GetAssetGUIDsByPath() const;

		template <typename T>
		bool IsAssetLoaded(const AssetRef<T>& assetRef) const;

		bool IsAssetLoaded(const Engine::GUID guid) const;

		template <typename T>
		Engine::Expected<T*> LoadAsset(AssetRef<T>& assetRef);

		template <typename T>
		std::future<std::shared_ptr<T>> LoadAssetByPath(const std::filesystem::path& path) noexcept;

		template <typename T>
		bool ReleaseAsset(AssetRef<T>& assetRef);

		bool ReleaseAsset(const Engine::GUID guid);

		Engine::Expected<void> RegisterAssetLoader(const std::wstring& extension, AssetLoaderFn loaderFn);
		Engine::Expected<void> RegisterAssetSaver(const std::wstring& extension, AssetSaverFn saverFn);
		Engine::Expected<void> RegisterAssetCreator(const std::wstring& extension, AssetCreatorFn creatorFn);

	private:
		bool DoesMatchFilter(const std::filesystem::directory_entry& directoryEntry) const;
		bool DoesMetaFileExist(const std::filesystem::directory_entry& directoryEntry, std::filesystem::path& metaFilePath) const;
		Engine::GUID CreateAssetMetaFile(Engine::AssetType assetType, 
			const std::filesystem::path& metaFilePath,
			std::optional<Engine::GUID> guid = std::nullopt) const;
		bool TryExtractGuidAndAssetTypeFromMetaFile(const std::filesystem::path& metaFilePath, 
			Engine::GUID& _guid, 
			Engine::AssetType& type) const;

		Expected<std::shared_ptr<void>> LoadAssetByGuid(const Engine::GUID guid);
		Expected<std::shared_ptr<void>> LoadAssetByPath(const std::filesystem::path& path);

		std::string AssetTypeToString(Engine::AssetType assetType) const;
	};

	template <typename T>
	bool AssetDatabase::IsAssetLoaded(const AssetRef<T>& assetRef) const
	{
		return IsAssetLoaded(assetRef.Guid());
	}

	template <typename T>
	Engine::Expected<T*> AssetDatabase::LoadAsset(AssetRef<T>& assetRef)
	{
		Expected<std::shared_ptr<void>> loadedAssetPtr = LoadAssetByGuid(assetRef.Guid());
		if (loadedAssetPtr.has_value())
		{
			assetRef.Assign(std::static_pointer_cast<T>(loadedAssetPtr.value()));
			return assetRef.Ptr();
		}

		return loadedAssetPtr.error();
	}


	template <typename T>
	std::future<std::shared_ptr<T>> AssetDatabase::LoadAssetByPath(const std::filesystem::path& path) noexcept
	{
		return std::async(std::launch::async, [this, path]()
			{
				Expected<std::shared_ptr<void>> loadedAssetPtr = LoadAssetByPath(path);
				if (loadedAssetPtr.has_value())
				{
					return std::static_pointer_cast<T>(loadedAssetPtr.value());
				}

				LOG_ERROR("Failed to load asset at path {}: {}", path.string(), loadedAssetPtr.error().message);
				return std::shared_ptr<T>(nullptr);
			});
	}

	/// <summary>
	/// Releases the asset held by the given AssetRef. This will decrement the ref count of the asset,
	/// and if the ref count reaches zero, the asset will be unloaded from memory. Either way, the 
	/// AssetRef's pointer will be released and set to nullptr.
	/// </summary>
	/// <typeparam name="T"></typeparam>
	/// <param name="assetRef"></param>
	/// <returns>Whether the asset was unloaded from memory as the result of this release.</returns>
	template <typename T>
	bool AssetDatabase::ReleaseAsset(AssetRef<T>& assetRef)
	{
		if (assetRef.Ptr())
		{
			assetRef.Assign(nullptr);
			return ReleaseAsset(assetRef.Guid());
		}

		return false;
	}

} // namespace Engine