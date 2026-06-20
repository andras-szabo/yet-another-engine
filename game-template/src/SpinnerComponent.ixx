module;

#include "ReflectionMacros.h"
#include "LoggerMacros.h"

export module SpinnerComponent;

#if defined( __INTELLISENSE__ )
#include "../../engine-core/src/EngineCore.ixx"
#include "../../engine-core/src/EngineInstance.ixx"
#else
import EngineCore;
import EngineInstance;
#endif

// SpinnerComponent — reference implementation for game developers.
//
// Demonstrates the full component authoring pattern:
//   - COMPONENT_BODY  : declares reflection and type-ID hooks
//   - REGISTER_COMPONENT : auto-registers / unregisters with the engine registry
//   - FIELD()         : passive annotations for the future code-gen tool
//   - OnCreate / OnUpdate / OnDestroy lifecycle hooks
//   - SpinnerComponent.reflected.h : seam file with field descriptors

export class SpinnerComponent : public Engine::Component
{
    COMPONENT_BODY(SpinnerComponent)
    REGISTER_COMPONENT(SpinnerComponent)

public:
    // Rotation speed in degrees per second.
    FIELD() float speed = 90.0f;

    // World-space axis to rotate around (should be a unit vector).
    FIELD() Engine::Vec3 axis = Engine::Vec3{ 0.f, 1.f, 0.f };

    // When false the spinner does nothing during OnUpdate.
    FIELD() bool active = true;

    SpinnerComponent() = default;
    ~SpinnerComponent() override = default;

    void OnCreate() override
    {
        LOG_INFO("[SpinnerComponent] Created – speed={:.1f} deg/s", speed);
    }

    void OnUpdate(float dt) override
    {
        if (!active)
            return;

        // Placeholder: a real implementation would retrieve the owning
        // GameObject's Transform and apply a quaternion rotation each frame.
        const float angleDeg = speed * dt;
        LOG_TRACE("[SpinnerComponent] Rotating {:.4f} deg around ({:.2f},{:.2f},{:.2f})",
            angleDeg, axis.x, axis.y, axis.z);
    }

    void OnDestroy() override
    {
        LOG_INFO("[SpinnerComponent] Destroyed");
    }
};

#include "SpinnerComponent.reflected.h"
