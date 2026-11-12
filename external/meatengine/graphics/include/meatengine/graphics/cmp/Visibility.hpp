#pragma once
    
namespace me::graphics::cmp {
    struct Visibility {
        bool value = true;
        bool inherit = true;
    };

    struct ComputedVisibility {
        bool visible = true;
    };
}