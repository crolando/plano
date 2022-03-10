#ifndef NODE_TURNKEY_HANDLE_INTERACTIONS_H
#define NODE_TURNKEY_HANDLE_INTERACTIONS_H

#include <node_turnkey_api.h>
#include <imgui.h>
#include "utilities/widgets.h"
#include <imgui_node_editor.h>
#include <example_node_spawner.h>
#include <example_property_im_draw.h>
#include <QDebug>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <draw_utils.h> // for statepack

//using namespace turnkey::types;
//using namespace turnkey::api;
//namespace ed = ax::NodeEditor;
//namespace util = ax::NodeEditor::Utilities;
//using namespace ax;
//using ax::Widgets::IconType;


void handle_link_dragging_interactions(turnkey::internal::s_Session.SessionData& ctx, statepack& s);
void handle_delete_interactions(turnkey::api::nodos_session_data& ctx);



#endif // NODE_TURNKEY_HANDLE_INTERACTIONS_H
