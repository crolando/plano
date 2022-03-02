#include <node_turnkey_api.h>
#include <imgui.h>
#include "utilities/widgets.h"
#include <imgui_node_editor.h>
#include <example_node_spawner.h>
#include <example_property_im_draw.h>
#include <QDebug>

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <draw_utils.h>
#include <node_turnkey_draw_nodes.h>

using namespace turnkey::types;
using namespace turnkey::api;
namespace ed = ax::NodeEditor;
namespace util = ax::NodeEditor::Utilities;
using namespace ax;
using ax::Widgets::IconType;


void turnkey::api::nodos_session_data::DrawPinIcon(const Pin& pin, bool connected, int alpha)
{
    IconType iconType;
    ImColor  color = GetIconColor(pin.Type);
    color.Value.w = alpha / 255.0f;
    switch (pin.Type)
    {
        case PinType::Flow:     iconType = IconType::Flow;   break;
        case PinType::Bool:     iconType = IconType::Circle; break;
        case PinType::Int:      iconType = IconType::Circle; break;
        case PinType::Float:    iconType = IconType::Circle; break;
        case PinType::String:   iconType = IconType::Circle; break;
        case PinType::Object:   iconType = IconType::Circle; break;
        case PinType::Function: iconType = IconType::Circle; break;
        case PinType::Delegate: iconType = IconType::Square; break;
        default:
            return;
    }

    ax::Widgets::Icon(ImVec2(s_PinIconSize, s_PinIconSize), iconType, connected, color, ImColor(32, 32, 32, alpha));
};


void turnkey::api::nodos_session_data::Frame(void)
{
    UpdateTouch();
    auto& io = ImGui::GetIO();

    //ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);

    ed::SetCurrentEditor(m_Editor);

    //auto& style = ImGui::GetStyle();

# if 0
    {
        for (auto x = -io.DisplaySize.y; x < io.DisplaySize.x; x += 10.0f)
        {
            ImGui::GetWindowDrawList()->AddLine(ImVec2(x, 0), ImVec2(x + io.DisplaySize.y, io.DisplaySize.y),
                IM_COL32(255, 255, 0, 255));
        }
    }
# endif



    //static float leftPaneWidth  = 400.0f;
    //static float rightPaneWidth = 800.0f;
    //Splitter(true, 4.0f, &leftPaneWidth, &rightPaneWidth, 50.0f, 50.0f);

    //ShowLeftPane(leftPaneWidth - 4.0f);

    //ImGui::SameLine(0.0f, 12.0f);

    // This little beauty lets me split up this horrifyingly huge file.
    static statepack s;

    // ====================================================================================================================================
    // NODOS DEV - Immediate Mode node drawing.
    // ====================================================================================================================================
    ed::Begin("Node editor");

    // ====================================================================================================================================
    // NODOS DEV - draw nodes
    // newLinkPin is passed to allow highlighting of valid candiate type-safe pin destinations when link-drawing from another pin.
    // ====================================================================================================================================
    draw_nodes(*this,s.newLinkPin);

    // ====================================================================================================================================
    // NODOS DEV - draw links
    // ====================================================================================================================================
    for (auto& link : s_Links)
        ed::Link(link.ID, link.StartPinID, link.EndPinID, link.Color, 2.0f);

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
    if (!s.createNewNode)
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
                            s_Links.emplace_back(Link(GetNextId(), startPinId, endPinId));
                            s_Links.back().Color = GetIconColor(startPin->Type);
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


        // ====================================================================================================================================
        // NODOS DEV - Delete interactions in immediate mode.
        // BeginDelete() - ??? Describe conditions
        //     QueryDeletedLink() - ??? describe conditions
        //         AcceptDeletedItem()
        //     QueryDeletedNode() - ??? describe conditions
        //         AcceptDeletedItem()
        // EndDelete()
        // ====================================================================================================================================
        if (ed::BeginDelete())
        {
            ed::LinkId linkId = 0;
            while (ed::QueryDeletedLink(&linkId))
            {
                if (ed::AcceptDeletedItem())
                {
                    auto id = std::find_if(s_Links.begin(), s_Links.end(), [linkId](auto& link) { return link.ID == linkId; });
                    if (id != s_Links.end())
                        s_Links.erase(id);
                }
            }

            ed::NodeId nodeId = 0;
            while (ed::QueryDeletedNode(&nodeId))
            {
                if (ed::AcceptDeletedItem())
                {
                    auto id = std::find_if(s_Nodes.begin(), s_Nodes.end(), [nodeId](auto& node) { return node.ID == nodeId; });
                    if (id != s_Nodes.end())
                        s_Nodes.erase(id);
                }
            }
        }
        ed::EndDelete();
    } // Close bracket for "if (!s.createNewNode)"


    // ====================================================================================================================================
    // NODOS DEV - Handle right-click context menu spawning
    //   This section just "fires" handlers that occur later.
    //
    // ShowNodeContextMenu() - handle right-click on a node
    // ShowPinContextMenu() - handle right-click on a pin
    // ShowLinkContextMenu() - handle right-click on link
    // ShowBackgroundContextMenu() - handle right-click on graph
    //
    // Special Note about reference frame switching:
    // popup windows are not done in graph space, they're done in screen space.
    // Suspend() changes the posititiong reference frame from "graph" to "screen"
    // so, all following calls are in screen space. Then Resume() goes back to reference frame.
    // ====================================================================================================================================
    ImGui::SetCursorScreenPos(ImGui::GetCursorScreenPos());
# if 1
    auto openPopupPosition = ImGui::GetMousePos();
    ed::Suspend();
    if (ed::ShowNodeContextMenu(&s.contextNodeId))
        ImGui::OpenPopup("Node Context Menu");
    else if (ed::ShowPinContextMenu(&s.contextPinId))
        ImGui::OpenPopup("Pin Context Menu");
    else if (ed::ShowLinkContextMenu(&s.contextLinkId))
        ImGui::OpenPopup("Link Context Menu");
    else if (ed::ShowBackgroundContextMenu())
    {
        ImGui::OpenPopup("Create New Node");
        s.newNodeLinkPin = nullptr;
    }
    // Resume:  Calls hereafter are now in the graph reference frame.
    ed::Resume();



    // ====================================================================================================================================
    // NODOS DEV - Draw context menu bodies
    //   This section "implements" the OpenPopup() calls from the previous section.
    //
    //   "NodeContextMenu"
    //   "Pin Context Menu"
    //   "Link Context Menu"
    //   "Create New Node"
    //
    // Please read notes above about screen-space and graph-space conversion
    // implemented in Suspend() and Resume()
    // ====================================================================================================================================
    // Suspend: Calls hereafter are in screnspace.
    ed::Suspend();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
    if (ImGui::BeginPopup("Node Context Menu")) // ----------------------------------------------------------------------------------------
    {
        auto node = FindNode(s.contextNodeId);

        ImGui::TextUnformatted("Node Context Menu");
        ImGui::Separator();
        if (node)
        {
            ImGui::Text("ID: %p", node->ID.AsPointer());
            ImGui::Text("Type: %s", node->Type == NodeType::Blueprint ? "Blueprint" : (node->Type == NodeType::Tree ? "Tree" : "Comment"));
            ImGui::Text("Inputs: %d", (int)node->Inputs.size());
            ImGui::Text("Outputs: %d", (int)node->Outputs.size());
        }
        else
            ImGui::Text("Unknown node: %p", s.contextNodeId.AsPointer());
        ImGui::Separator();
        if (ImGui::MenuItem("Delete"))
            ed::DeleteNode(s.contextNodeId);
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Pin Context Menu")) // ----------------------------------------------------------------------------------------
    {
        auto pin = FindPin(s.contextPinId);

        ImGui::TextUnformatted("Pin Context Menu");
        ImGui::Separator();
        if (pin)
        {
            ImGui::Text("ID: %p", pin->ID.AsPointer());
            if (pin->Node)
                ImGui::Text("Node: %p", pin->Node->ID.AsPointer());
            else
                ImGui::Text("Node: %s", "<none>");
        }
        else
            ImGui::Text("Unknown pin: %p", s.contextPinId.AsPointer());

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Link Context Menu")) // ----------------------------------------------------------------------------------------
    {
        auto link = FindLink(s.contextLinkId);

        ImGui::TextUnformatted("Link Context Menu");
        ImGui::Separator();
        if (link)
        {
            ImGui::Text("ID: %p", link->ID.AsPointer());
            ImGui::Text("From: %p", link->StartPinID.AsPointer());
            ImGui::Text("To: %p", link->EndPinID.AsPointer());
        }
        else
            ImGui::Text("Unknown link: %p", s.contextLinkId.AsPointer());
        ImGui::Separator();
        if (ImGui::MenuItem("Delete"))
            ed::DeleteLink(s.contextLinkId);
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Create New Node")) // ----------------------------------------------------------------------------------------
    {
        auto newNodePostion = openPopupPosition;
        //ImGui::SetCursorScreenPos(ImGui::GetMousePosOnOpeningCurrentPopup());

        //auto drawList = ImGui::GetWindowDrawList();
        //drawList->AddCircleFilled(ImGui::GetMousePosOnOpeningCurrentPopup(), 10.0f, 0xFFFF00FF);

        Node* node = nullptr;

        // Populate the context right click menu with all the nodes in the registry.
        for(auto nodos: NodeRegistry){
            if (ImGui::MenuItem(nodos.first.c_str())){
                node = NewRegistryNode(nodos.first);
            }
        }

        // Populate the right click menu with the example nodes
        if (ImGui::MenuItem("Input Action"))
            node = SpawnInputActionNode(this);
        if (ImGui::MenuItem("Output Action"))
            node = SpawnOutputActionNode(this);
        if (ImGui::MenuItem("Branch"))
            node = SpawnBranchNode(this);
        if (ImGui::MenuItem("Do N"))
            node = SpawnDoNNode(this);
        if (ImGui::MenuItem("Set Timer"))
            node = SpawnSetTimerNode(this);
        if (ImGui::MenuItem("Less"))
            node = SpawnLessNode(this);
        if (ImGui::MenuItem("Weird"))
            node = SpawnWeirdNode(this);
        if (ImGui::MenuItem("Trace by Channel"))
            node = SpawnTraceByChannelNode(this);
        if (ImGui::MenuItem("Print String"))
            node = SpawnPrintStringNode(this);
        ImGui::Separator();
        if (ImGui::MenuItem("Comment"))
            node = SpawnComment(this);
        ImGui::Separator();
        if (ImGui::MenuItem("Sequence"))
            node = SpawnTreeSequenceNode(this);
        if (ImGui::MenuItem("Move To"))
            node = SpawnTreeTaskNode(this);
        if (ImGui::MenuItem("Random Wait"))
            node = SpawnTreeTask2Node(this);
        ImGui::Separator();
        if (ImGui::MenuItem("Message"))
            node = SpawnMessageNode(this);
        ImGui::Separator();
        if (ImGui::MenuItem("Transform"))
            node = SpawnHoudiniTransformNode(this);
        if (ImGui::MenuItem("Group"))
            node = SpawnHoudiniGroupNode(this);

        // Do post-node-spawn actions here.
        if (node)
        {            
            BuildNodes();

            s.createNewNode = false;

            // Move node to near the mouse location
            ed::SetNodePosition(node->ID, newNodePostion);

            // This section auto-connects a pin in your new node to a link you've dragged out
            if (auto startPin = s.newNodeLinkPin)
            {
                auto& pins = startPin->Kind == ed::PinKind::Input ? node->Outputs : node->Inputs;

                for (auto& pin : pins)
                {
                    if (CanCreateLink(startPin, &pin))
                    {
                        auto endPin = &pin;
                        if (startPin->Kind == ed::PinKind::Input)
                            std::swap(startPin, endPin);

                        s_Links.emplace_back(Link(GetNextId(), startPin->ID, endPin->ID));
                        s_Links.back().Color = GetIconColor(startPin->Type);

                        break;
                    }
                }
            }
        } // Done with post-node-spawn actions

        ImGui::EndPopup();
    }  // Done with ImGui::BeginPopup("Create New Node")
    else
        s.createNewNode = false;
    ImGui::PopStyleVar();
    ed::Resume();
# endif

/*
    cubic_bezier_t c;
    c.p0 = pointf(100, 600);
    c.p1 = pointf(300, 1200);
    c.p2 = pointf(500, 100);
    c.p3 = pointf(900, 600);

    auto drawList = ImGui::GetWindowDrawList();
    auto offset_radius = 15.0f;
    auto acceptPoint = [drawList, offset_radius](const bezier_subdivide_result_t& r)
    {
        drawList->AddCircle(to_imvec(r.point), 4.0f, IM_COL32(255, 0, 255, 255));

        auto nt = r.tangent.normalized();
        nt = pointf(-nt.y, nt.x);

        drawList->AddLine(to_imvec(r.point), to_imvec(r.point + nt * offset_radius), IM_COL32(255, 0, 0, 255), 1.0f);
    };

    drawList->AddBezierCurve(to_imvec(c.p0), to_imvec(c.p1), to_imvec(c.p2), to_imvec(c.p3), IM_COL32(255, 255, 255, 255), 1.0f);
    cubic_bezier_subdivide(acceptPoint, c);
*/
    ed::End(); // END CALL.  We are no longer drawing nodes.

    // ====================================================================================================================================
    // NODOS DEV - outside-of-begin-end field.
    // TODO: Expand greatly and reorganize.  This is hard to find, currently.
    // ====================================================================================================================================
    // retrive selections using api
    if (ax::NodeEditor::HasSelectionChanged())
    {
        int count =  ax::NodeEditor::GetSelectedObjectCount();

        // Get node selections
        ax::NodeEditor::NodeId* node_buffer = nullptr;
        int sel_nodes = ax::NodeEditor::GetSelectedNodes(nullptr,0);
        if (sel_nodes > 0) {
            node_buffer = new ax::NodeEditor::NodeId[sel_nodes];
            ax::NodeEditor::GetSelectedNodes(node_buffer,sel_nodes);
            for (int i = 0; i < sel_nodes; i++) {
                Node* Node = FindNode(node_buffer[i]);
                qDebug() << "Selected Node ID: " << Node->ID.Get() ;
            }
        }


        // Get link selections
        ax::NodeEditor::LinkId* link_buffer = nullptr;
        int sel_links = ax::NodeEditor::GetSelectedLinks(nullptr,0);
        if (sel_links > 0) {
            link_buffer = new ax::NodeEditor::LinkId[sel_links];
            ax::NodeEditor::GetSelectedLinks(link_buffer,sel_links);
            for (int i = 0; i < sel_links; i++) {
                Link* Link = FindLink(link_buffer[i]);
                qDebug() << "Selected Link ID: " << Link->ID.Get() ;
            }
        }
        qDebug() << "--------------- end of list -------------";

        // int sel_links = ax::NodeEditor::GetSelectedLinks(LinkId* links, int size);
        //bool IsNodeSelected(NodeId nodeId);
        //bool IsLinkSelected(LinkId linkId);


        delete[] node_buffer;
        delete[] link_buffer;

    }


    //ImGui::ShowTestWindow();
    //ImGui::ShowMetricsWindow();
}
