#pragma once

class patches_component
{
public:
    patches_component();

    static void initialize_ProjectWorldToScreen_patch();
};

namespace patches_component_hooks
{
    DEFINE_FUNCTION_HOOK(void, SendInventoryChangedNotification, (UObject* Inventory, uintptr_t b, uintptr_t c, uintptr_t d));
    DEFINE_FUNCTION_HOOK(void, ProjectWorldToScreen, ());
}
