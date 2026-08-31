module;

#include <cassert>
#include "engine_core_api.h"

module AssetDatabase;

#if defined ( __INTELLISENSE__ )
#include "AssetDatabase.ixx"
#include "DataFile.ixx"
#else
import DataFile;
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
			if (DoesMatchFilter(entry))
			{
				const auto assetType = IsAssetFile(entry);
				if (assetType != AssetType::Undefined)
				{
					// TODO
					//		- check if there's a corresponding .meta file, and create one if not
					//		- then add it to the maps

					// TODO what if there are assets called the same,
					// but w/ different extensions :(

					fs::path metaFilePath {};
					Engine::GUID guid = Engine::GUID::Invalid();

					if (!DoesMetaFileExist(entry, metaFilePath))
					{
						guid = CreateMetaFile(assetType, metaFilePath);
					}
					else
					{
						Engine::AssetType serializedAssetType{ Engine::AssetType::Undefined };
						if (!TryExtractGuidAndAssetTypeFromMetaFile(metaFilePath, guid, serializedAssetType))
						{
							return Engine::Unexpected{ Engine::Error { Engine::ErrorType::File, "Guid collision!" } };
						}

						// TODO:
						// if the serialized asset type is different to the actual one,
						// maybe log a warning?
					}

					// So now we have a guid and an asset type;
					// now to check that the guid is indeed unique in the map...

					//TODO
				}
			}
		}

		_impl->pathsByGuid = std::move(pathsByGuid);
		_impl->guidsByPath = std::move(guidsByPath);

		return {};
	}

	bool AssetDatabase::DoesMatchFilter(const std::filesystem::directory_entry& directoryEntry) const
	{
		// TODO
		return true;
	}

	Engine::GUID AssetDatabase::CreateMetaFile(Engine::AssetType assetType,
		const std::filesystem::path& metaFilePath) const
	{
		const Engine::GUID guid{};

		Engine::DataFile out;

		out["GUID"].SetULong(guid.id);
		out["Type"].SetInt(static_cast<int>(assetType));
		// TODO other things

		Engine::DataFile::Serialize(out, metaFilePath.string());

		return guid;
	}

	bool AssetDatabase::TryExtractGuidAndAssetTypeFromMetaFile(const std::filesystem::path& metaFilePath, 
		Engine::GUID& guid, 
		Engine::AssetType& type) const
	{
		const auto meta = Engine::DataFile::Deserialize(metaFilePath.string());
		if (meta.has_value())
		{
			const auto& value = meta.value();

			guid = value["GUID"].GetULong();
			type = static_cast<Engine::AssetType>(value["Type"].GetInt());

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
		metaFilePath.replace_extension(metaExtension);

		return fs::exists(metaFilePath);
	}
} // namespace Engine