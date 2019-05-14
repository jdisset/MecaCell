#ifndef MECACELL_H
#define MECACELL_H

#ifndef MECACELL_VIEWER
#define MECACELL_VIEWER_ADDITIONS(...) void dummy()
#define ADD_BUTTON(...)
#endif

#include "basecell.hpp"
#include "connectablecell.hpp"
#include "integrators.hpp"
#include "utilities/config.hpp"
#include "utilities/exportable.hpp"
#include "world.hpp"

namespace MecaCell {
EXPORTABLE_NAMESPACE_DEFINITIONS
}
#endif
