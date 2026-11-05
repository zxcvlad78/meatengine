#pragma once
    
namespace me::cmp {
    struct Visibility {
        bool value = true;
        bool inherit = true;
    };

    struct ComputedVisibility {
        bool visible = true;
    };
}