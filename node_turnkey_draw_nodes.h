#ifndef NODE_TURNKEY_DRAW_NODES_H
#define NODE_TURNKEY_DRAW_NODES_H
#include <node_turnkey_api.h>
#include <imgui.h>
#include "utilities/widgets.h"
#include <imgui_node_editor.h>
#include <example_node_spawner.h>
#include <example_property_im_draw.h>
#include <QDebug>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

using namespace turnkey::types;
using namespace turnkey::api;
namespace ed = ax::NodeEditor;
namespace util = ax::NodeEditor::Utilities;
using namespace ax;
using ax::Widgets::IconType;

// newLinkPin is used here to cause runtime highlighting of relavent candidate pins
// when you are dragging a link.
void draw_nodes(turnkey::api::nodos_session_data& ctx,Pin* newLinkPin);


#endif // NODE_TURNKEY_DRAW_NODES_H
