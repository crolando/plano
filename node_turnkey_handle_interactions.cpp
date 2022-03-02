#include <node_turnkey_handle_interactions.h>


// ====================================================================================================================================
// NODOS DEV - Handle link-dragging interactions in immediate mode.
// BeginCreate() - Handle dragging a link out of a pin
//     QueryNewLink() - Hovering over a destination pin
//         AcceptNewItem()
//         RejectNewItem()
//     QueryNewNode() - hovering over the graph
//         AcceptNewItem()
//         RejectNewItem()
// EndCreate()
// ====================================================================================================================================
void handle_link_dragging_interactions(turnkey::api::nodos_session_data& ctx, statepack& s)
{

    // ====================================================================================================================================
    // NODOS DEV - Handle dragging a link out of a pin
    // ====================================================================================================================================
    if (ed::BeginCreate(ImColor(255, 255, 255), 2.0f))
    {
        // function declaration to show labels ---------------------------------------------
        auto showLabel = [](const char* label, ImColor color)
        {
            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetTextLineHeight());
            auto size = ImGui::CalcTextSize(label);

            auto padding = ImGui::GetStyle().FramePadding;
            auto spacing = ImGui::GetStyle().ItemSpacing;

            ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(spacing.x, -spacing.y));

            auto rectMin = ImGui::GetCursorScreenPos() - padding;
            auto rectMax = ImGui::GetCursorScreenPos() + size + padding;

            auto drawList = ImGui::GetWindowDrawList();
            drawList->AddRectFilled(rectMin, rectMax, color, size.y * 0.15f);
            ImGui::TextUnformatted(label);
        };

        // ====================================================================================================================================
        // NODOS DEV - Handle "Query New Link" interaction:
        //   When you've started dragging off a pin, and you're now hovering over a candidate destination pin
        // ====================================================================================================================================
        // startPinId and endPinId are "return by reference" from QueryNewLink.
        ed::PinId startPinId = 0, endPinId = 0;
        if (ed::QueryNewLink(&startPinId, &endPinId))
        {
            // setup stack vars for tests
            auto startPin = ctx.FindPin(startPinId);
            auto endPin   = ctx.FindPin(endPinId);
            s.newLinkPin = startPin ? startPin : endPin;

            // in this system you can drag from inputs to outputs
            // but we have to mirror them for the tests here
            if (startPin->Kind == ed::PinKind::Input)
            {
                std::swap(startPin, endPin);
                std::swap(startPinId, endPinId);
            }

            // bring the owner nodes into scope, based on the pins.
            auto startNode = startPin->Node;
            auto endNode = endPin->Node;

            // Run tests & then handle interactions (hover, mouse release, etc)
            if (startPin && endPin)
            {
                if (endPin == startPin)
                {
                    ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                }
                else if (ctx.isNodeAncestor(endNode,startNode)){
                    showLabel("x Connection would create a loop", ImColor(45, 32, 32, 180));
                    ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                }
                else if (endPin->Kind == startPin->Kind)
                {
                    showLabel("x Incompatible Pin Kind", ImColor(45, 32, 32, 180));
                    ed::RejectNewItem(ImColor(255, 0, 0), 2.0f);
                }
                else if (endPin->Node == startPin->Node)
                {
                    showLabel("x Cannot connect to self", ImColor(45, 32, 32, 180));
                    ed::RejectNewItem(ImColor(255, 0, 0), 1.0f);
                }
                else if (endPin->Type != startPin->Type)
                {
                    showLabel("x Incompatible Pin Type", ImColor(45, 32, 32, 180));
                    ed::RejectNewItem(ImColor(255, 128, 128), 1.0f);
                }
                else
                {
                    showLabel("+ Create Link", ImColor(32, 45, 32, 180));
                    if (ed::AcceptNewItem(ImColor(128, 255, 128), 4.0f))
                    {
                        ctx.s_Links.emplace_back(Link(ctx.GetNextId(), startPinId, endPinId));
                        ctx.s_Links.back().Color = GetIconColor(startPin->Type);
                    }
                }
            } // Done with pin connection interaction handling
        } // Done with if (ed::QueryNewLink(&startPinId, &endPinId))


        // ====================================================================================================================================
        // NODOS DEV - Handle "Query New Node" interaction:
        //   When you've started dragging off a pin, and you're now hovering over the graph
        // ====================================================================================================================================
        // pinId is "return by reference" from QueryNewNode()
        ed::PinId pinId = 0;
        if (ed::QueryNewNode(&pinId))
        {
            s.newLinkPin = ctx.FindPin(pinId);
            if (s.newLinkPin)
                showLabel("+ Create Node", ImColor(32, 45, 32, 180));

            if (ed::AcceptNewItem())
            {
                s.createNewNode  = true;
                s.newNodeLinkPin = ctx.FindPin(pinId);
                s.newLinkPin = nullptr;
                ed::Suspend();
                ImGui::OpenPopup("Create New Node");
                ed::Resume();
            }
        }
    } // End of "if (ed::BeginCreate()) "
    else
        s.newLinkPin = nullptr;

    ed::EndCreate(); // Formal end of "Create" block

}




// ====================================================================================================================================
// NODOS DEV - Delete interactions in immediate mode.
// BeginDelete() - ??? Describe conditions
//     QueryDeletedLink() - ??? describe conditions
//         AcceptDeletedItem()
//     QueryDeletedNode() - ??? describe conditions
//         AcceptDeletedItem()
// EndDelete()
// ====================================================================================================================================
void handle_delete_interactions(turnkey::api::nodos_session_data& ctx)
{
    if (ed::BeginDelete())
    {
        ed::LinkId linkId = 0;
        while (ed::QueryDeletedLink(&linkId))
        {
            if (ed::AcceptDeletedItem())
            {
                auto id = std::find_if(ctx.s_Links.begin(), ctx.s_Links.end(), [linkId](auto& link) { return link.ID == linkId; });
                if (id != ctx.s_Links.end())
                    ctx.s_Links.erase(id);
            }
        }

        ed::NodeId nodeId = 0;
        while (ed::QueryDeletedNode(&nodeId))
        {
            if (ed::AcceptDeletedItem())
            {
                auto id = std::find_if(ctx.s_Nodes.begin(), ctx.s_Nodes.end(), [nodeId](auto& node) { return node.ID == nodeId; });
                if (id != ctx.s_Nodes.end())
                    ctx.s_Nodes.erase(id);
            }
        }
    }
    ed::EndDelete();
}
