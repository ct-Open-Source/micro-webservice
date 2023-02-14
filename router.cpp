#include "router.hpp"

namespace warp {

    bool operator==(const path_method& lhs, const path_method& rhs)
    {
        return lhs.path == rhs.path && lhs.verb == rhs.verb;
    }

}
