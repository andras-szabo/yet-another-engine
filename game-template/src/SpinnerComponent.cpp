module;

#include "LoggerMacros.h"
#include "ReflectionMacros.h"

module SpinnerComponent;

#if defined ( __INTELLISENSE__ )
#include "SpinnerComponent.ixx"
#else
#endif

REGISTER_COMPONENT(SpinnerComponent)
REFLECTED_FIELDS(SpinnerComponent,
    { "speed",  Engine::FieldType::Float, offsetof(SpinnerComponent, speed) },
    { "axis",   Engine::FieldType::Vec3,  offsetof(SpinnerComponent, axis) },
    { "active", Engine::FieldType::Bool,  offsetof(SpinnerComponent, active) })


//Engine::FieldSpan SpinnerComponent::GetFieldDescriptors() const
//{
//    return {};
//}

//struct SpinnerComponent_Registrar                                        
//{                                                                   
//SpinnerComponent_Registrar()                                         
//{                                                            
//    auto foo = 123; //SpinnerComponent::StaticTypeID();
//    auto factory = []() -> std::unique_ptr<Engine::Component>
//    {
//            return std::make_unique<SpinnerComponent>();
//    };
//
//    Engine::RegisterComponent(foo, factory);
    //Engine::RegisterComponent(foo, nullptr);
//Engine::RegisterComponent(ClassName::StaticTypeID(), 
 //   []() -> std::unique_ptr<Engine::Component> {        
 //   return std::make_unique<ClassName>();           
 //   });                                                 
/*}                                                               
~SpinnerComponent_Registrar()                                        
{     */                                                          
//Engine::UnregisterComponent(ClassName::StaticTypeID());     
//}                                                               
//};                                                                  

//static SpinnerComponent_Registrar _SpinnerComponent_registrar{};

//std::span<const Engine::FieldDescriptor> SpinnerComponent::GetFieldDescriptors()
//{
//    return std::span<const Engine::FieldDescriptor>{};
//}


void SpinnerComponent::OnCreate()
{
    LOG_INFO("[SpinnerComponent] Created – speed={:.1f} deg/s", speed);
}

void SpinnerComponent::OnUpdate(float dt)
{
    if (!active)
    {
        return;
    }

    // Placeholder: a real implementation would retrieve the owning
    // GameObject's Transform and apply a quaternion rotation each frame.
    const float angleDeg = speed * dt;
    LOG_TRACE("[SpinnerComponent] Rotating {:.4f} deg around ({:.2f},{:.2f},{:.2f})",
        angleDeg, axis.x, axis.y, axis.z);
}

void SpinnerComponent::OnDestroy()
{
    LOG_INFO("[SpinnerComponent] Destroyed");
}
