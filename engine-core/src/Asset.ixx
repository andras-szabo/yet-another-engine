module;

export module Asset;

namespace Engine
{
	enum class AssetType
	{
		Undefined = 0,

		Scene = 1,
		Texture = 2,
		StaticMesh = 3,
	};

} // namespace Engine