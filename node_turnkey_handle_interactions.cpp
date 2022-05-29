#include <node_turnkey_internal.h>
#include <node_turnkey_handle_interactions.h>

using namespace turnkey::internal;
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
void handle_link_dragging_interactions(statepack& s)
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
            auto startPin = FindPin(startPinId);
            auto endPin   = FindPin(endPinId);
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
                else if (isNodeAncestor(endNode,startNode)){
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

                        s_Session.s_Links.emplace_back(turnkey::types::Link(GetNextId(), startPinId, endPinId));
                        s_Session.s_Links.back().Color = GetIconColor(startPin->Type);
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
            s.newLinkPin = FindPin(pinId);
            if (s.newLinkPin)
                showLabel("+ Create Node", ImColor(32, 45, 32, 180));

            if (ed::AcceptNewItem())
            {
                s.createNewNode  = true;
                s.newNodeLinkPin = FindPin(pinId);
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
void handle_delete_interactions()
{
    if (ed::BeginDelete())
    {
        // This deletes links.  This is very simple because nothing refers to links.
        ed::LinkId linkId = 0;
        while (ed::QueryDeletedLink(&linkId))
        {
            if (ed::AcceptDeletedItem())
            {
                auto id = std::find_if(s_Session.s_Links.begin(), s_Session.s_Links.end(), [linkId](auto& link) { return link.ID == linkId; });
                if (id != s_Session.s_Links.end())
                    s_Session.s_Links.erase(id);
            }
        }

        // This deletes nodes.  This is horrible because links refer to nodes's pins.
        // so we have to clean up a case where links refer to pins that were destroyed during node destruction.
        // This incurs an expensive search.
        ed::NodeId nodeId = 0;
        while (ed::QueryDeletedNode(&nodeId))
        {
            if (ed::AcceptDeletedItem())
            {
                auto id = std::find_if(s_Session.s_Nodes.begin(), s_Session.s_Nodes.end(), [nodeId](auto& node) { return node.ID == nodeId; });

                if (id != s_Session.s_Nodes.end())
                {
                    // Node deletion routine.
                    // First, we have to record a node's pin IDs before destroying the node and its pins.
                    auto node = *id;
                    std::vector<unsigned int> pin_ids;
                    for(auto inp: node.Inputs)
                        pin_ids.push_back(inp.ID.Get());
                    for(auto outp: node.Outputs)
                        pin_ids.push_back(outp.ID.Get());

                    // Now that we know what pin IDs the node had, we can actually destroy it now.
                    s_Session.s_Nodes.erase(id);


                    // We have to destroy link objects that were connected to this dead node.
                    // you do this by asking all the links if they're connected to the dead pin ids.
                    auto it = s_Session.s_Links.begin();
                    while(it != s_Session.s_Links.end())
                    {
                        bool del = false;
                        for(auto pid: pin_ids)
                        {
                            auto endpin = it->EndPinID.Get();
                            auto startpin = it->StartPinID.Get();
                            // it is on the shit list
                            if(pid == endpin || pid == startpin)
                            {
                                del = true;

                            }
                        }
                        // are you connected to at least one dieing pin?
                        if(del)
                        {
                            it = s_Session.s_Links.erase(it);
                        } else {
                        it++;
                        }
                    }
                }  // End test if the node search was sucessful
            } // End of Accept Delete Item block
        } // End of QueryDeletedNode loop
    } // End BeginDelete test
    ed::EndDelete();
}
