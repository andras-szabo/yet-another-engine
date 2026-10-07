module;

#include <cassert>
#include "engine_core_api.h"
#include "LoggerMacros.h"

module AssetDatabase;

#if defined ( __INTELLISENSE__ )
#include "AssetDatabase.ixx"
#include "DataFile.ixx"
#include "Logger.ixx"
#include "Utility.ixx"
#else
import DataFile;
import Logger;
import Utility;
import std;
#endif

namespace Engine
{
	AssetDatabase::AssetDatabase()
	{
		_impl = new AssetDatabase_Impl{};
	}

	AssetDatabase::~AssetDatabase()
	{
		delete _impl;
		_impl = nullptr;
	}

	AssetDatabase::AssetDatabase(AssetDatabase&& other)
	{
		if (other._impl != _impl)
		{
			_impl = other._impl;
			other._impl = nullptr;
		}
	}

	AssetDatabase& AssetDatabase::operator=(AssetDatabase&& other)
	{
		if (other._impl != _impl)
		{
			_impl = other._impl;
			other._impl = nullptr;
		}

		return *this;
	}

	int AssetDatabase::GetAssetCount() const
	{
		assert(_impl->pathsByGuid.size() == _impl->guidsByPath.size() && "AssetDatabase internal size mismatch!");
		return static_cast<int>(_impl->pathsByGuid.size());
	}

	void AssetDatabase::Update(std::span<const Engine::FileChangeEvent> fileChangeEvents)
	{
		namespace fs = std::filesystem;

		// TODO actually implement the rest of this
		for (const auto& event : fileChangeEvents)
		{
			const fs::directory_entry entry{ event.path };
			const auto assetType = IsAssetFile(entry);

			if (assetType == AssetType::Undefined)
			{
				continue;
			}

			const std::string pathAsString = Engine::WideToUtf8(event.path);

			switch (event.type)
			{

			case Engine::FileChangeType::Added:
			{
				LOG_INFO("File added: {}", pathAsString);
				TryAddNewAsset(entry, assetType);
				break;
			}

			case Engine::FileChangeType::Modified:
				LOG_INFO("File modified: {}", pathAsString);
				break;

			case Engine::FileChangeType::Removed:
			{
				LOG_INFO("File removed: {}", pathAsString);
				TryRemoveAsset(entry, assetType);
				break;
			}

			case Engine::FileChangeType::Renamed:
			{
				LOG_INFO("File renamed: from {} to {}", Engine::WideToUtf8(event.oldPath), pathAsString);
				const fs::directory_entry oldEntry { event.oldPath };
				TryRenameAsset(entry, oldEntry, assetType);
				break;
			}

			}
		}
	}

	const std::unordered_map<std::string, Engine::GUID>& AssetDatabase::GetAssetGUIDsByPath() const
	{
		return _impl->guidsByPath;
	}

	Engine::Expected<void> AssetDatabase::TryRenameAsset(const std::filesystem::directory_entry& entry,
		const std::filesystem::directory_entry& oldEntry,
		AssetType assetType)
	{
		namespace fs = std::filesystem;

		assert(assetType != AssetType::Undefined && "Invalid asset type.");

		const auto oldGuidByPath = _impl->guidsByPath.find(oldEntry.path().string());
		const auto oldGuid = (oldGuidByPath != _impl->guidsByPath.end()) 
			? oldGuidByPath->second 
			: Engine::GUID::Invalid();

		const auto newGuidByPath = _impl->guidsByPath.find(entry.path().string());
		const auto newGuid = (newGuidByPath != _impl->guidsByPath.end())
			? newGuidByPath->second
			: Engine::GUID::Invalid();

		fs::path oldMetaFilePath{};
		bool doesOldMetaFileExist = DoesMetaFileExist(oldEntry, oldMetaFilePath);

		fs::path newMetaFilePath{};
		bool doesNewMetaFileExist = DoesMetaFileExist(entry, newMetaFilePath);

		if (oldGuid.IsValid() && !newGuid.IsValid())
		{
			if (doesNewMetaFileExist)
			{
				LOG_ERROR("Asset rename error: new meta file already exists: {}", newMetaFilePath.string());
				return Engine::Unexpected({ Engine::ErrorType::File, "Asset rename error: new meta file already exists." });
			}

			// Asset was renamed, update the database
			LOG_INFO("Asset renamed: {} -> {}", oldEntry.path().string(), entry.path().string());

			_impl->guidsByPath.erase(oldGuidByPath);
			_impl->guidsByPath.emplace(entry.path().string(), oldGuid);
			_impl->pathsByGuid[oldGuid] = entry.path().string();

			CreateAssetMetaFile(assetType, newMetaFilePath, oldGuid);

			if (doesOldMetaFileExist)
			{
				LOG_INFO("Removing associated meta file");
				fs::remove(oldMetaFilePath);
			}
		}
		else if (!oldGuid.IsValid() && newGuid.IsValid())
		{
			// Asset was renamed to an existing asset, update the database
			LOG_INFO("Asset renamed to existing asset: {} -> {}", oldEntry.path().string(), entry.path().string());
			return Engine::Unexpected({ Engine::ErrorType::File, "Asset renamed to existing asset." });
		}
		else if (oldGuid.IsValid() && newGuid.IsValid() && oldGuid != newGuid)
		{
			LOG_ERROR("Asset rename conflict: {} -> {} (GUIDs: {} -> {})", oldEntry.path().string(), entry.path().string(), oldGuid, newGuid);
			return Engine::Unexpected({ Engine::ErrorType::File, "Asset rename conflict." });
		}
		else
		{
			LOG_ERROR("Asset rename error: {} -> {} (GUIDs: {} -> {})", oldEntry.path().string(), entry.path().string(), oldGuid, newGuid);
			return Engine::Unexpected({ Engine::ErrorType::File, "Asset rename error." });
		}

		return {};
	}

	Engine::Expected<void> AssetDatabase::TryRemoveAsset(const std::filesystem::directory_entry& entry, AssetType assetType)
	{
		namespace fs = std::filesystem;

		assert(assetType != AssetType::Undefined && "Invalid asset type.");

		const std::string assetPath = entry.path().string();
		const auto guidByPath = _impl->guidsByPath.find(assetPath);

		fs::path metaFilePath{};
		bool doesMetaFileExist = DoesMetaFileExist(entry, metaFilePath);
		if (doesMetaFileExist)
		{
			LOG_INFO("Removing associated meta file");
			fs::remove(metaFilePath);
		}

		if (guidByPath == _impl->guidsByPath.end())
		{
			LOG_INFO("Asset not in database: {} ({})", assetPath, AssetTypeToString(assetType));
			return {};
		}

		const Engine::GUID guid = guidByPath->second;
		const auto pathByGuid = _impl->pathsByGuid.find(guid);
		if (pathByGuid == _impl->pathsByGuid.end())
		{
			LOG_ERROR("Asset guid exists in guidsByPath but not in pathsByGuid: {} ({}) -> {}", assetPath, AssetTypeToString(assetType), guid);
			return Engine::Unexpected({ Engine::ErrorType::File, "Asset guid exists in guidsByPath but not in pathsByGuid." });
		}

		LOG_INFO("Asset removed from database: {} ({}) -> {}", assetPath, AssetTypeToString(assetType), guid);
		_impl->pathsByGuid.erase(pathByGuid);
		_impl->guidsByPath.erase(guidByPath);

		return {};
	}

	Engine::Expected<void> AssetDatabase::TryAddNewAsset(const std::filesystem::directory_entry& entry, AssetType assetType)
	{
		namespace fs = std::filesystem;

		assert(assetType != AssetType::Undefined && "Invalid asset type.");

		// 1. Check if the asset path is already associated with a guid.
		const std::string assetPath = entry.path().string();
		const auto guidByPath = _impl->guidsByPath.find(assetPath);
		fs::path metaFilePath{};
		bool doesMetaFileExist = DoesMetaFileExist(entry, metaFilePath);

		if (guidByPath != _impl->guidsByPath.end())
		{
			Engine::GUID guid = guidByPath->second;

			if (doesMetaFileExist)
			{
				Engine::GUID serializedGuid = Engine::GUID::Invalid();
				Engine::AssetType serializedAssetType{ Engine::AssetType::Undefined };

				if (!TryExtractGuidAndAssetTypeFromMetaFile(metaFilePath, serializedGuid, serializedAssetType))
				{
					// asset meta file looks broken, not sure how to handle this.
					return Engine::Unexpected({ Engine::ErrorType::File, "Asset meta file looks broken." });
				}
				else
				{
					if (serializedGuid != guid)
					{
						// asset meta file has a different guid than the one we have in the database, not sure how to handle this.
						// let's not destructively handle this for now, just log and error
						return Engine::Unexpected({ Engine::ErrorType::File, "Asset meta file has a different guid than the one we have in the database." });
					}
				}

				// Let's check if the path is also correct, maybe it was moved.
				const auto pathByGuid = _impl->pathsByGuid.find(guid);
				if (pathByGuid == _impl->pathsByGuid.end() || pathByGuid->second != assetPath)
				{
					// This should never happen, but let's handle it gracefully.
					return Engine::Unexpected({ Engine::ErrorType::File, "Asset guid exists in guidsByPath but not in pathsByGuid." });
				}

				LOG_INFO("Meta file exists and looks fine. Asset already in database: {} ({}) -> {}", assetPath, AssetTypeToString(assetType), guid);
				return {};
			}

			// No meta file exists. maybe it was removed?; let's recreate it
			LOG_INFO("Asset already in database, but no meta file exists. Recreating meta file: {} ({}) -> {}", assetPath, AssetTypeToString(assetType), guid);
			CreateAssetMetaFile(assetType, metaFilePath, guid);
			return {};
		}
		// Asset not in database yet, let's add it!

		const auto guid = CreateAssetMetaFile(assetType, metaFilePath);
		const auto existingPath = _impl->pathsByGuid.find(guid);
		if (existingPath != _impl->pathsByGuid.end())
		{
			const auto existingPathAsString = existingPath->first;
			LOG_ERROR("Asset GUID collision. {} and {} share the same GUID.",
				existingPathAsString,
				assetPath);

			return Engine::Unexpected({ Engine::ErrorType::File, "Asset GUID collision." });
		}

		LOG_INFO("Asset file added to database: {} ({}) -> {}", assetPath, AssetTypeToString(assetType), guid);

		_impl->pathsByGuid.emplace(guid, assetPath);
		_impl->guidsByPath.emplace(assetPath, guid);

		return {};
	}

	Engine::Expected<void> AssetDatabase::PopulateFromFolder(const std::filesystem::path& path)
	{
		namespace fs = std::filesystem;

		// 1. Create new std::unordered_maps
		// 2. Populate them
		//		2.1. Gather filters (e.g. partial importing!)
		//		2.2. Scan folders for all sources
		//		2.3. Create .meta files if they don't exist
		//		2.4. Check for GUID clashes
		//		2.5. If GUID clash resolved (or none), add to new maps
		// 3. If everything is OK, replace current _pathsByGuid and _guidsByPath with
		//	  the newly created ones

		// To return an error, use sg like:
		// return Engine::Unexpected{ Engine::Error { Engine::ErrorType::File, "Game DLL not found."} };

		if (!fs::exists(path))
		{
			return Engine::Unexpected{ Engine::Error { Engine::ErrorType::File, "Path not found." } };
		}

		std::unordered_map<GUID, std::string> pathsByGuid;
		std::unordered_map<std::string, GUID> guidsByPath;

		for (const auto& entry : fs::recursive_directory_iterator(path))
		{
			LOG_TRACE("Path: {}", entry.path().string());

			if (DoesMatchFilter(entry))
			{
				const auto assetType = IsAssetFile(entry);
				if (assetType != AssetType::Undefined)
				{
					const std::string assetPath = entry.path().string();
					if (guidsByPath.find(assetPath) != guidsByPath.end())
					{
						LOG_ERROR("Asset path has multiple associated GUIDs. {}", assetPath);
						continue;
					}

					fs::path metaFilePath{};
					Engine::GUID _guid = Engine::GUID::Invalid();

					if (!DoesMetaFileExist(entry, metaFilePath))
					{
						_guid = CreateAssetMetaFile(assetType, metaFilePath);
					}
					else
					{
						LOG_INFO("Meta file exists...");
						Engine::AssetType serializedAssetType{ Engine::AssetType::Undefined };
						if (!TryExtractGuidAndAssetTypeFromMetaFile(metaFilePath, _guid, serializedAssetType))
						{
							LOG_ERROR("Asset meta file looks broken. {}", metaFilePath.string());
							continue;
						}

						LOG_INFO("Meta file has type and GUID");

						if (serializedAssetType != assetType)
						{
							LOG_ERROR("Asset meta file type mismatch. {} actual: {}, expected: {}", assetPath,
								AssetTypeToString(assetType),
								AssetTypeToString(serializedAssetType));
							continue;
						}
					}

					const auto existingPath = pathsByGuid.find(_guid);
					if (existingPath != pathsByGuid.end())
					{
						const auto existingPathAsString = existingPath->first;
						LOG_ERROR("Asset GUID collision. {} and {} share the same GUID.",
							existingPathAsString,
							assetPath);

						continue;
					}

					LOG_INFO("Asset file added to database: {} ({}) -> {}", assetPath, AssetTypeToString(assetType), _guid);

					pathsByGuid.emplace(_guid, assetPath);
					guidsByPath.emplace(assetPath, _guid);
				}
			}
		}

		_impl->pathsByGuid = std::move(pathsByGuid);
		_impl->guidsByPath = std::move(guidsByPath);

		return {};
	}

	std::string AssetDatabase::AssetTypeToString(AssetType t) const
	{
		switch (t)
		{
		case AssetType::Scene:		return "Scene";
		case AssetType::StaticMesh:	return "Static Mesh";
		case AssetType::Texture:	return "Texture";
		}

		return "Undefined";
	}

	bool AssetDatabase::DoesMatchFilter(const std::filesystem::directory_entry& directoryEntry) const
	{
		// TODO
		return true;
	}

	Engine::GUID AssetDatabase::CreateAssetMetaFile(Engine::AssetType assetType,
		const std::filesystem::path& metaFilePath,
		std::optional<Engine::GUID> guid) const
	{
		const Engine::GUID _guid = guid.value_or(Engine::GUID{});

		Engine::DataFile out;

		out["Data"].SetULong(_guid.id, 0);
		out["Data"].SetInt(static_cast<int>(assetType), 1);
		out["Data"].SetString(AssetTypeToString(assetType), 2);

		// TODO other things

		Engine::DataFile::Serialize(out, metaFilePath.string());

		return _guid;
	}

	bool AssetDatabase::TryExtractGuidAndAssetTypeFromMetaFile(const std::filesystem::path& metaFilePath,
		Engine::GUID& _guid,
		Engine::AssetType& type) const
	{
		const auto meta = Engine::DataFile::Deserialize(metaFilePath.string());
		if (meta.has_value())
		{
			const auto& value = meta.value();

			_guid = value["Data"].GetULong();
			type = static_cast<Engine::AssetType>(value["Data"].GetInt(1));

			return true;

		}
		else
		{
			return false;
		}
	}


	AssetType AssetDatabase::IsAssetFile(const std::filesystem::directory_entry& directoryEntry) const
	{
		namespace fs = std::filesystem;

		if (!directoryEntry.is_directory())
		{
			const std::wstring extensionAsString{ directoryEntry.path().extension() };
			const auto typeByExtensionPair = _impl->assetTypesByExtension.find(extensionAsString);
			if (typeByExtensionPair != _impl->assetTypesByExtension.end())
			{
				return (*typeByExtensionPair).second;
			}
		}

		return AssetType::Undefined;
	}

	bool AssetDatabase::DoesMetaFileExist(const std::filesystem::directory_entry& directoryEntry,
		std::filesystem::path& metaFilePath) const
	{
		namespace fs = std::filesystem;

		const fs::path metaExtension{ ".meta" };

		metaFilePath = directoryEntry.path();
		metaFilePath.concat(metaExtension.string());

		LOG_INFO(metaFilePath.string());

		return fs::exists(metaFilePath);
	}

	Engine::Expected<void> AssetDatabase_Impl::RegisterAssetLoader(const std::wstring& extension,
		AssetLoaderFn loaderFn)
	{
		if (_loaderFunctionsByExtension.find(extension) != _loaderFunctionsByExtension.end())
		{
			return Engine::Unexpected({ Engine::ErrorType::Undefined, "An asset loader is already registered for this extension." });
		}

		_loaderFunctionsByExtension[extension] = loaderFn;
		return {};
	}

	Engine::Expected<std::shared_ptr<void>> AssetDatabase_Impl::LoadAsset(const Engine::GUID guid)
	{
		// If the asset is already loaded, jolly good, let's return it.
		const auto loadedAssetIt = _loadedAssetsByGUID.find(guid);

		if (loadedAssetIt != _loadedAssetsByGUID.end() && !loadedAssetIt->second.expired())
		{
			return loadedAssetIt->second.lock();
		}

		// Otherwise, let's find the path for the asset
		const auto pathIt = pathsByGuid.find(guid);
		if (pathIt == pathsByGuid.end())
		{
			return Engine::Unexpected({ Engine::ErrorType::File, "No path found for the given asset GUID." });
		}

		// ...and load it from there.
		auto loadedAssetPtr = DoLoadAssetFromPath(pathIt->second);
		if (!loadedAssetPtr.has_value())
		{
			return loadedAssetPtr;
		}

		_loadedAssetsByGUID.insert_or_assign(guid, loadedAssetPtr.value());
		return loadedAssetPtr;
	}

	Engine::Expected<std::shared_ptr<void>> AssetDatabase_Impl::LoadAsset(const std::string_view path)
	{
		// Find associated GUID
		const auto loadedGuid = guidsByPath.find(std::string(path));
		Engine::GUID guid{};

		if (loadedGuid == guidsByPath.end())
		{
			return Engine::Unexpected({ Engine::ErrorType::File, "No GUID found for the given asset path." });
		}

		guid = loadedGuid->second;

		// Find already-loaded asset by guid, if present
		auto loadedAssetIt = _loadedAssetsByGUID.find(guid);
		if (loadedAssetIt != _loadedAssetsByGUID.end() && !loadedAssetIt->second.expired())
		{
			return loadedAssetIt->second.lock();
		}

		// If not present, load it from disk...
		auto loadedAssetPtr = DoLoadAssetFromPath(path);
		if (!loadedAssetPtr.has_value())
		{
			return loadedAssetPtr;
		}

		// ... and cache it before returning
		_loadedAssetsByGUID.insert_or_assign(guid, loadedAssetPtr.value());
		return loadedAssetPtr;
	}

	Engine::Expected<std::shared_ptr<void>> AssetDatabase_Impl::DoLoadAssetFromPath(const std::string_view path)
	{
		const auto extension = std::filesystem::path(path).extension();
		const auto loaderIt = _loaderFunctionsByExtension.find(extension);

		if (loaderIt == _loaderFunctionsByExtension.end())
		{
			return Engine::Unexpected({ Engine::ErrorType::Undefined, "No loader function registered for this file extension." });
		}

		const auto& loaderFn = loaderIt->second;
		const std::shared_ptr<void> loadedAssetPtr = loaderFn(path);
		if (!loadedAssetPtr)
		{
			return Engine::Unexpected({ Engine::ErrorType::Undefined, "Failed to load asset." });
		}

		return loadedAssetPtr;
	}

	bool AssetDatabase_Impl::IsAssetLoaded(const Engine::GUID guid) const
	{
		const auto loadedAssetIt = _loadedAssetsByGUID.find(guid);
		return loadedAssetIt != _loadedAssetsByGUID.end() && !loadedAssetIt->second.expired();
	}

	bool AssetDatabase_Impl::ReleaseAsset(const Engine::GUID guid)
	{
		const auto loadedAssetIt = _loadedAssetsByGUID.find(guid);

		if (loadedAssetIt != _loadedAssetsByGUID.end() && loadedAssetIt->second.expired())
		{
			_loadedAssetsByGUID.erase(loadedAssetIt);
			return true;
		}

		return false;
	}

} // namespace Engine