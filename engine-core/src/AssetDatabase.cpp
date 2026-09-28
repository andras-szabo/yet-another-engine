module;

#include <cassert>
#include "engine_core_api.h"
#include "LoggerMacros.h"

module AssetDatabase;

#if defined ( __INTELLISENSE__ )
#include "AssetDatabase.ixx"
#include "DataFile.ixx"
#include "Logger.ixx"
#else
import DataFile;
import Logger;
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

					fs::path metaFilePath {};
					Engine::GUID _guid = Engine::GUID::Invalid();

					if (!DoesMetaFileExist(entry, metaFilePath))
					{
						_guid = CreateMetaFile(assetType, metaFilePath);
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

	Engine::GUID AssetDatabase::CreateMetaFile(Engine::AssetType assetType,
		const std::filesystem::path& metaFilePath) const
	{
		const Engine::GUID _guid{};

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

		if (directoryEntry.is_regular_file())
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