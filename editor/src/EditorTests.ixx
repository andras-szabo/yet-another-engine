module;

#include <cassert>
#include <string>
#include <vector>

#include "LoggerMacros.h"

export module EditorTests;

#if defined ( __INTELLISENSE__ )
#include "../../engine-core/src/ComponentStorage.ixx"
#include "../../engine-core/src/IComponentStorage.ixx"
#include "../../engine-core/src/EngineError.ixx"
#include "../../engine-core/src/EngineInstance.ixx"
#include "../../engine-core/src/Logger.ixx"
#include "../../engine-core/src/Math.ixx"
#include "../../engine-core/src/Reflection.ixx"
#include "../../engine-core/src/Scene.ixx"
#include "../../engine-core/src/Serialization.ixx"
#else
import EngineCore;
import EngineInstance;

import std;
#endif

namespace EditorTests
{
	export struct TestSummary
	{
		int testsRun{ 0 };
		int testsFailed{ 0 };
		int checksFailed{ 0 };
	};

	// Runs every registered test whose name contains 'filter' (all tests if empty).
	// Failed checks are logged and counted; they do not abort the run.
	export TestSummary RunAllTests(std::string_view filter = {});

	// Names of all registered tests, in registration order.
	export std::vector<std::string> ListTests();
}

module :private;

namespace EditorTests
{
	// ---------------------------------------------------------------------------------
	// Minimal test framework
	// ---------------------------------------------------------------------------------

	struct TestContext
	{
		std::string testName;
		int checksFailed{ 0 };
	};

	bool Check(TestContext& ctx, bool condition, std::string_view what)
	{
		if (!condition)
		{
			++ctx.checksFailed;
			LOG_ERROR("[FAIL] {}: {}", ctx.testName, what);
		}
		return condition;
	}

	struct TestCase
	{
		std::string_view name;
		void (*fn)(TestContext&);
	};

	// ---------------------------------------------------------------------------------
	// Helpers
	// ---------------------------------------------------------------------------------

	// A scene plus a few helpers to build and inspect it by node *name*
	// (node indices aren't stable across removals).
	struct SceneFixture
	{
		SceneFixture()
			: cs{ &Engine::EngineInstance::GetComponentStorage() },
			scene{ cs, "TestScene" }
		{
		}

		const Engine::TransformStorage& Storage() const { return *scene.GetTransformStorage(); }

		int Add(std::string_view name, int parentIndex)
		{
			return scene.CreateGameObject(cs, name, parentIndex)->GetTransform()->GetSceneNodeIndex();
		}

		int AddUnder(std::string_view name, std::string_view parentName)
		{
			return Add(name, Idx(parentName));
		}

		int Idx(std::string_view name) const
		{
			const auto& names = Storage().names;
			for (int i = 0; i < (int)names.size(); ++i)
			{
				if (names[i] == name)
				{
					return i;
				}
			}
			return -1;
		}

		bool Remove(std::string_view name)
		{
			const int idx = Idx(name);
			return idx != -1 && scene.TryRemoveGameObject(idx);
		}

		std::vector<std::string> Children(std::string_view name) const
		{
			std::vector<std::string> result;
			const int idx = Idx(name);
			if (idx == -1)
			{
				return result;
			}

			const auto& hs = Storage().hierarchies;
			for (int c = hs[idx].firstChild; c != -1; c = hs[c].nextSibling)
			{
				result.push_back(Storage().names[c]);
			}
			return result;
		}

		int Prev(std::string_view name) const { return Storage().hierarchies[Idx(name)].prevSibling; }
		int Depth(std::string_view name) const { return Storage().hierarchies[Idx(name)].depth; }
		int Count() const { return scene.GetNodeCount(); }

		// Builds:
		//   _Root
		//   +- A
		//   |  +- A1
		//   |  +- A2
		//   +- B
		//   |  +- B1
		//   |     +- B1a
		//   +- C
		void BuildStandardTree()
		{
			AddUnder("A", "_Root");
			AddUnder("B", "_Root");
			AddUnder("C", "_Root");
			AddUnder("A1", "A");
			AddUnder("A2", "A");
			AddUnder("B1", "B");
			AddUnder("B1a", "B1");
		}

		using ComponentStoragePtr = decltype(&Engine::EngineInstance::GetComponentStorage());

		ComponentStoragePtr cs;
		Engine::Scene::Scene scene;
	};

	using Names = std::vector<std::string>;

	// Validates the transform storage, and that the scene's parallel vectors agree with it.
	void CheckConsistent(TestContext& ctx, const SceneFixture& f, std::string_view label)
	{
		std::string error;
		if (!Check(ctx, f.Storage().ValidateHierarchy(&error),
			std::format("{}: ValidateHierarchy failed: {}", label, error)))
		{
			return;
		}

		const auto& gos = f.scene.GetAllGameObjects();
		if (!Check(ctx, gos.size() == f.Storage().hierarchies.size(),
			std::format("{}: gameObjects ({}) and storage ({}) out of sync", label, gos.size(), f.Storage().hierarchies.size())))
		{
			return;
		}

		for (int i = 0; i < (int)gos.size(); ++i)
		{
			Check(ctx, gos[i]->GetName() == f.Storage().names[i],
				std::format("{}: node {} name mismatch ('{}' vs '{}')", label, i, gos[i]->GetName(), f.Storage().names[i]));
			Check(ctx, gos[i]->GetTransform() != nullptr && gos[i]->GetTransform()->GetSceneNodeIndex() == i,
				std::format("{}: node {} has a Transform with a stale node index", label, i));
		}
	}

	// "depth:name:guid" for each node, depth-first from the root.
	Names DescribeDepthFirst(Engine::Scene::Scene& scene)
	{
		Names result;
		scene.WalkDepthFirst(0, [&](Engine::Scene::Scene& s, std::size_t index, int depth)
			{
				result.push_back(std::format("{}:{}:{}", depth, s.GetNodeName(index), s.GetNodeGuid(index).id));
			});
		return result;
	}

	// ---------------------------------------------------------------------------------
	// Tests
	// ---------------------------------------------------------------------------------

	void Test_Scene_SerializationRoundTrip(TestContext& ctx)
	{
		SceneFixture f;
		const int first = f.Add("First_Child", 0);
		const int second = f.Add("Second_Child", first);
		f.Add("Third_Child", second);

		Engine::DataFile serialized;
		Engine::SerializeScene(f.scene, serialized);

		auto sceneMaybe = Engine::DeserializeScene(serialized, *f.cs);
		if (!Check(ctx, sceneMaybe.has_value(), "Couldn't deserialize scene."))
		{
			return;
		}

		auto sceneB = std::move(sceneMaybe.value());
		const auto& goA = f.scene.GetAllGameObjects();
		const auto& goB = sceneB.GetAllGameObjects();

		if (!Check(ctx, goA.size() == goB.size(), "Size mismatch."))
		{
			return;
		}

		for (std::size_t i = 0; i < goA.size(); ++i)
		{
			Check(ctx, goA[i]->GetName() == goB[i]->GetName(), "GO name mismatch");
			Check(ctx, goA[i]->GetGUID() == goB[i]->GetGUID(), "GUID mismatch");
		}
	}

	void Test_Hierarchy_Links(TestContext& ctx)
	{
		SceneFixture f;
		f.BuildStandardTree();

		Check(ctx, f.Count() == 8, "Expected 8 nodes");
		Check(ctx, f.Children("_Root") == Names{ "A", "B", "C" }, "Root's children");
		Check(ctx, f.Children("A") == Names{ "A1", "A2" }, "A's children");
		Check(ctx, f.Children("B1") == Names{ "B1a" }, "B1's children");

		Check(ctx, f.Prev("A") == -1, "First child has no prevSibling");
		Check(ctx, f.Prev("B") == f.Idx("A"), "B's left sibling is A");
		Check(ctx, f.Prev("C") == f.Idx("B"), "C's left sibling is B");
		Check(ctx, f.Prev("A2") == f.Idx("A1"), "A2's left sibling is A1");

		Check(ctx, f.Depth("B1a") == 3, "B1a depth");
		CheckConsistent(ctx, f, "after build");
	}

	void Test_Hierarchy_RemoveLeaf(TestContext& ctx)
	{
		SceneFixture f;
		f.BuildStandardTree();

		Check(ctx, f.Remove("A2"), "Remove A2");
		Check(ctx, f.Count() == 7, "Count after removing a leaf");
		Check(ctx, f.Children("A") == Names{ "A1" }, "A's children");
		Check(ctx, f.Idx("A2") == -1, "A2 is gone");
		CheckConsistent(ctx, f, "after removing A2");
	}

	void Test_Hierarchy_RemoveMiddleSubtree(TestContext& ctx)
	{
		SceneFixture f;
		f.BuildStandardTree();

		Check(ctx, f.Remove("B"), "Remove B");
		Check(ctx, f.Count() == 5, "B, B1, B1a should be gone");
		Check(ctx, f.Children("_Root") == Names{ "A", "C" }, "Root's children");
		Check(ctx, f.Idx("B1") == -1 && f.Idx("B1a") == -1, "Descendants of B are gone");
		Check(ctx, f.Prev("C") == f.Idx("A"), "C's left sibling is now A");
		Check(ctx, f.Children("A") == Names{ "A1", "A2" }, "Siblings' subtrees untouched");
		CheckConsistent(ctx, f, "after removing B");
	}

	void Test_Hierarchy_RemoveFirstChild(TestContext& ctx)
	{
		SceneFixture f;
		f.BuildStandardTree();

		Check(ctx, f.Remove("A"), "Remove A");
		Check(ctx, f.Count() == 5, "A, A1, A2 should be gone");
		Check(ctx, f.Children("_Root") == Names{ "B", "C" }, "Root's children");
		Check(ctx, f.Prev("B") == -1, "B is now the first child");
		CheckConsistent(ctx, f, "after removing A");
	}

	void Test_Hierarchy_RemoveLastChild(TestContext& ctx)
	{
		SceneFixture f;
		f.BuildStandardTree();

		Check(ctx, f.Remove("C"), "Remove C");
		Check(ctx, f.Count() == 7, "Count");
		Check(ctx, f.Children("_Root") == Names{ "A", "B" }, "Root's children");
		CheckConsistent(ctx, f, "after removing C");
	}

	void Test_Hierarchy_RemoveInvalid(TestContext& ctx)
	{
		SceneFixture f;
		f.BuildStandardTree();

		Check(ctx, !f.scene.TryRemoveGameObject(0), "Root cannot be removed");
		Check(ctx, !f.scene.TryRemoveGameObject(999), "Out-of-range index cannot be removed");
		Check(ctx, f.Count() == 8, "Nothing should have been removed");
		CheckConsistent(ctx, f, "after invalid removals");
	}

	void Test_Hierarchy_RemoveAllThenAdd(TestContext& ctx)
	{
		SceneFixture f;
		f.BuildStandardTree();

		Check(ctx, f.Remove("A") && f.Remove("B") && f.Remove("C"), "Remove all top-level nodes");
		Check(ctx, f.Count() == 1, "Only the root should remain");
		Check(ctx, f.Children("_Root").empty(), "Root has no children");
		CheckConsistent(ctx, f, "after removing everything");

		f.AddUnder("X", "_Root");
		f.AddUnder("Y", "_Root");
		f.AddUnder("X1", "X");
		Check(ctx, f.Children("_Root") == Names{ "X", "Y" }, "New children");
		Check(ctx, f.Prev("Y") == f.Idx("X"), "Y's left sibling is X");
		CheckConsistent(ctx, f, "after re-adding");
	}

	void Test_Hierarchy_SetParent(TestContext& ctx)
	{
		SceneFixture f;
		f.BuildStandardTree();

		f.scene.SetParent(f.Idx("A1"), f.Idx("C"));
		Check(ctx, f.Children("A") == Names{ "A2" }, "A lost A1");
		Check(ctx, f.Children("C") == Names{ "A1" }, "C gained A1");
		Check(ctx, f.Depth("A1") == 2, "A1 depth");
		Check(ctx, f.Prev("A2") == -1, "A2 is now A's first child");
		CheckConsistent(ctx, f, "after moving A1");

		f.scene.SetParent(f.Idx("B1"), 0);		// brings B1a along
		Check(ctx, f.Children("_Root") == Names{ "A", "B", "C", "B1" }, "B1 appended to root");
		Check(ctx, f.Prev("B1") == f.Idx("C"), "B1's left sibling is C");
		Check(ctx, f.Depth("B1a") == 2, "B1a depth follows its parent");
		Check(ctx, f.Children("B").empty(), "B lost B1");
		CheckConsistent(ctx, f, "after moving B1");

		// A move followed by a removal of the moved subtree
		Check(ctx, f.Remove("B1"), "Remove B1");
		Check(ctx, f.Idx("B1a") == -1, "B1a went with it");
		CheckConsistent(ctx, f, "after removing moved subtree");
	}

	// Builds a larger pseudo-random tree, then removes random subtrees until only the root is left,
	// checking consistency after every step. Deterministic (fixed seed).
	void Test_Hierarchy_RemoveRandomStress(TestContext& ctx)
	{
		SceneFixture f;

		std::uint32_t state = 12345u;
		auto next = [&state]() { state = state * 1664525u + 1013904223u; return state >> 8; };

		constexpr int NodesToAdd = 60;
		for (int i = 0; i < NodesToAdd; ++i)
		{
			const int parent = static_cast<int>(next() % (std::uint32_t)f.Count());
			f.Add(std::format("n{}", i), parent);
		}
		CheckConsistent(ctx, f, "after building random tree");

		int step = 0;
		while (f.Count() > 1 && ctx.checksFailed == 0)
		{
			const int victim = 1 + static_cast<int>(next() % (std::uint32_t)(f.Count() - 1));
			const int before = f.Count();

			// Expected number of removed nodes: size of the victim's subtree.
			int subtreeSize = 0;
			{
				const auto& hs = f.Storage().hierarchies;
				std::vector<int> pending{ victim };
				while (!pending.empty())
				{
					const int n = pending.back();
					pending.pop_back();
					++subtreeSize;
					for (int c = hs[n].firstChild; c != -1; c = hs[c].nextSibling)
					{
						pending.push_back(c);
					}
				}
			}

			Check(ctx, f.scene.TryRemoveGameObject(victim), "TryRemoveGameObject");
			Check(ctx, f.Count() == before - subtreeSize,
				std::format("step {}: expected {} nodes removed, got {}", step, subtreeSize, before - f.Count()));
			CheckConsistent(ctx, f, std::format("stress step {}", step));
			++step;
		}

		Check(ctx, f.Count() == 1, "Only the root should be left");
	}

	void Test_Serialization_RoundTripAfterRemoval(TestContext& ctx)
	{
		SceneFixture f;
		f.BuildStandardTree();

		// Removing from the front forces swap-and-pop to reorder the in-memory nodes, so that
		// some parents end up with a higher index than their children.
		Check(ctx, f.Remove("A"), "Remove A");
		f.AddUnder("D", "B1a");
		CheckConsistent(ctx, f, "before serializing");

		Engine::DataFile serialized;
		Engine::SerializeScene(f.scene, serialized);

		auto sceneMaybe = Engine::DeserializeScene(serialized, *f.cs);
		if (!Check(ctx, sceneMaybe.has_value(), "Couldn't deserialize scene."))
		{
			return;
		}

		auto loaded = std::move(sceneMaybe.value());
		Check(ctx, loaded.GetNodeCount() == f.Count(), "Node count after round trip");

		std::string error;
		Check(ctx, loaded.GetTransformStorage()->ValidateHierarchy(&error),
			std::format("Loaded scene hierarchy invalid: {}", error));

		const Names before = DescribeDepthFirst(f.scene);
		const Names after = DescribeDepthFirst(loaded);
		Check(ctx, before == after, "Depth-first structure (depth, name, GUID) survives the round trip");
	}

	constexpr TestCase AllTests[] =
	{
		{ "scene.serialization_round_trip",		Test_Scene_SerializationRoundTrip },
		{ "hierarchy.links",					Test_Hierarchy_Links },
		{ "hierarchy.remove_leaf",				Test_Hierarchy_RemoveLeaf },
		{ "hierarchy.remove_middle_subtree",	Test_Hierarchy_RemoveMiddleSubtree },
		{ "hierarchy.remove_first_child",		Test_Hierarchy_RemoveFirstChild },
		{ "hierarchy.remove_last_child",		Test_Hierarchy_RemoveLastChild },
		{ "hierarchy.remove_invalid",			Test_Hierarchy_RemoveInvalid },
		{ "hierarchy.remove_all_then_add",		Test_Hierarchy_RemoveAllThenAdd },
		{ "hierarchy.set_parent",				Test_Hierarchy_SetParent },
		{ "hierarchy.remove_random_stress",		Test_Hierarchy_RemoveRandomStress },
		{ "serialization.round_trip_after_removal", Test_Serialization_RoundTripAfterRemoval },
	};

	// ---------------------------------------------------------------------------------
	// Public API
	// ---------------------------------------------------------------------------------

	std::vector<std::string> ListTests()
	{
		std::vector<std::string> names;
		for (const TestCase& test : AllTests)
		{
			names.emplace_back(test.name);
		}
		return names;
	}

	TestSummary RunAllTests(std::string_view filter)
	{
		TestSummary summary;

		for (const TestCase& test : AllTests)
		{
			if (!filter.empty() && test.name.find(filter) == std::string_view::npos)
			{
				continue;
			}

			TestContext ctx;
			ctx.testName = std::string{ test.name };

			test.fn(ctx);

			++summary.testsRun;
			summary.checksFailed += ctx.checksFailed;
			if (ctx.checksFailed > 0)
			{
				++summary.testsFailed;
				LOG_ERROR("[FAILED] {} ({} failed check(s))", test.name, ctx.checksFailed);
			}
			else
			{
				LOG_INFO("[PASSED] {}", test.name);
			}
		}

		LOG_INFO("Tests: {} run, {} failed, {} failed check(s) in total.",
			summary.testsRun, summary.testsFailed, summary.checksFailed);

		return summary;
	}
}
