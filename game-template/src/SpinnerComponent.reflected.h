// SpinnerComponent.reflected.h
//
// Hand-written seam file for SpinnerComponent.
// A future code-generation tool will produce this automatically from FIELD()
// annotations in SpinnerComponent.ixx.
//
// Include this file AFTER the class definition, still inside the module purview.

REFLECTED_FIELDS(SpinnerComponent,
    { "speed",  Engine::FieldType::Float, offsetof(SpinnerComponent, speed)  },
    { "axis",   Engine::FieldType::Vec3,  offsetof(SpinnerComponent, axis)   },
    { "active", Engine::FieldType::Bool,  offsetof(SpinnerComponent, active) }
)
