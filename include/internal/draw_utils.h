#ifndef DRAW_UTILS_H
#define DRAW_UTILS_H

#include <plano_types.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <utility>
#include <cstring>
#include <fstream>
#include <iostream>




//using namespace turnkey::types;
namespace ed = ax::NodeEditor;
namespace util = ax::NodeEditor::Utilities;
//using namespace ax;
//using ax::Widgets::IconType;

namespace plano {
namespace internal {

static bool CanCreateLink(plano::types::Pin* a, plano::types::Pin* b)
{
    if (!a || !b || a == b || a->Kind == b->Kind || a->Type != b->Type || a->Node == b->Node)
        return false;

    return true;
}

static inline ImRect ImGui_GetItemRect()
{
    return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
}

static inline ImRect ImRect_Expanded(const ImRect& rect, float x, float y)
{
    auto result = rect;
    result.Min.x -= x;
    result.Min.y -= y;
    result.Max.x += x;
    result.Max.y += y;
    return result;
}

static ImColor GetIconColor(plano::types::PinType type)
{
    switch (type)
    {
        default:
        case plano::types::PinType::Flow:     return ImColor(255, 255, 255);
        case plano::types::PinType::Bool:     return ImColor(220,  48,  48);
        case plano::types::PinType::Int:      return ImColor( 68, 201, 156);
        case plano::types::PinType::Float:    return ImColor(147, 226,  74);
        case plano::types::PinType::String:   return ImColor(124,  21, 153);
        case plano::types::PinType::Object:   return ImColor( 51, 150, 215);
        case plano::types::PinType::Function: return ImColor(218,   0, 183);
        case plano::types::PinType::Delegate: return ImColor(255,  48,  48);
    }
};

struct statepack {
    ed::NodeId contextNodeId      = 0;
    ed::LinkId contextLinkId      = 0;
    ed::PinId  contextPinId       = 0;
    bool createNewNode  = false;
    plano::types::Pin* newNodeLinkPin = nullptr;
    plano::types::Pin* newLinkPin     = nullptr;
};

}
}

#endif // DRAW_UTILS_H
