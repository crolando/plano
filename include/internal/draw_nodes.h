#ifndef NODE_TURNKEY_DRAW_NODES_H
#define NODE_TURNKEY_DRAW_NODES_H
#include <node_turnkey_api.h>
#include <imgui.h>
#include "widgets.h"
#include <imgui_node_editor.h>
#include <example_property_im_draw.h>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

// newLinkPin is used here to cause runtime highlighting of relavent candidate pins
// when you are dragging a link.

namespace turnkey {
namespace internal {
void draw_nodes(turnkey::types::Pin* newLinkPin);
}
}
#endif // NODE_TURNKEY_DRAW_NODES_H
