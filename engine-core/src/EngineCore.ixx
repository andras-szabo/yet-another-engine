// EngineCore.ixx — root module interface unit for engine-core
//
// A C++ module interface unit declares what this module exports to importers.
// The 'export module' declaration names the module; everything marked 'export'
// below is visible to any translation unit that writes 'import EngineCore;'.
//
// This file is a stub. Subsequent tasks will add real exports here,
// or introduce module partitions (e.g. EngineCore:Math, EngineCore:Scene).

export module EngineCore;
export import Reflection;
export import Component;
export import EngineError;
export import Logger;
export import Math;
export import Transform;
export import GUID;

// Nothing exported yet — the module exists so the build graph is wired up
// correctly from the start. Editor and game-template already import it.
