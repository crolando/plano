// This source file must have credited to thedmd on github.  This is a heavily modified version of this file, retrieved 12/27/2020:
// https://github.com/thedmd/imgui-node-editor/blob/master/examples/blueprints-example/blueprints-example.cpp

#include <NodosWidget.h>
#include "utilities/widgets.h"

#include <imgui_node_editor.h>
#include <example_node_spawner.h>
#include <node_utils.h> // BuildNodes

#include <example_property_im_draw.h>

#include <node_turnkey_types.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <utility>
#include <QDebug>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>



// Include your "user created node definitions"
#include <node_defs\import_animal.h>

using namespace turnkey::types;
using namespace turnkey::api;

// One day, this will be the mononlithic container for all these static vars.
nodos_session_data NodosSession;


static const int            s_PinIconSize = 24;

std::vector<Node>& s_Nodes = NodosSession.s_Nodes;

static std::vector<Link>    s_Links;   // Session Links - there is an ID that is important

// Backend populates this with node position data.  save it directly to your "project file"
// It's sort of private to the backend, so just let it serailze/deseralize and it should work.
// The this data hooks up to the s_Nodes and s_Links vectors based on the node.ID and link.ID.
static std::string          s_BlueprintData;

static ImTextureID          s_HeaderBackground = nullptr;
static ImTextureID          s_SampleImage = nullptr;
static ImTextureID          s_SaveIcon = nullptr;
static ImTextureID          s_RestoreIcon = nullptr;
static const float          s_TouchTime = 1.0f;

// Not sure what this is, but I think it is a way to determine if the node is dirty based on the last 'time'
// it was messed with.  Verify this at some point, won't you?
static std::map<ax::NodeEditor::NodeId, float, NodeIdLess> s_NodeTouchTime;

// Nodes and links are reflective - they know about each other.  This enforces this convention after construction
// and attaching those objects.  Having reflective data lets you query a link and get what node owns it.
void BuildNodes()
{
    for (auto& node : s_Nodes)
        NodosSession.BuildNode(&node);
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

namespace ed = ax::NodeEditor;
namespace util = ax::NodeEditor::Utilities;

using namespace ax;

using ax::Widgets::IconType;

static ed::EditorContext* m_Editor = nullptr;

//extern "C" __declspec(dllimport) short __stdcall GetAsyncKeyState(int vkey);
//extern "C" bool Debug_KeyPress(int vkey)
//{
//    static std::map<int, bool> state;
//    auto lastState = state[vkey];
//    state[vkey] = (GetAsyncKeyState(vkey) & 0x8000) != 0;
//    if (state[vkey] && !lastState)
//        return true;
//    else
//        return false;
//}

//static ed::NodeId GetNextNodeId()
//{
//    return ed::NodeId(GetNextId());
//}

static ed::LinkId GetNextLinkId()
{
    return ed::LinkId(NodosSession.GetNextId());
}

// TouchNode is called during config saves
static void TouchNode(ed::NodeId id)
{
    s_NodeTouchTime[id] = s_TouchTime;
}

// This is used for left pane stuff
static float GetTouchProgress(ed::NodeId id)
{
    auto it = s_NodeTouchTime.find(id);
    if (it != s_NodeTouchTime.end() && it->second > 0.0f)
        return (s_TouchTime - it->second) / s_TouchTime;
    else
        return 0.0f;
}

static void UpdateTouch()
{
    const auto deltaTime = ImGui::GetIO().DeltaTime;
    for (auto& entry : s_NodeTouchTime)
    {
        if (entry.second > 0.0f)
            entry.second -= deltaTime;
    }
}

static Node* FindNode(ed::NodeId id)
{
    for (auto& node : s_Nodes)
        if (node.ID == id)
            return &node;

    return nullptr;
}

static Link* FindLink(ed::LinkId id)
{
    for (auto& link : s_Links)
        if (link.ID == id)
            return &link;

    return nullptr;
}

static Pin* FindPin(ed::PinId id)
{
    if (!id)
        return nullptr;

    for (auto& node : s_Nodes)
    {
        for (auto& pin : node.Inputs)
            if (pin.ID == id)
                return &pin;

        for (auto& pin : node.Outputs)
            if (pin.ID == id)
                return &pin;
    }

    return nullptr;
}

static bool IsPinLinked(ed::PinId id)
{
    if (!id)
        return false;

    for (auto& link : s_Links)
        if (link.StartPinID == id || link.EndPinID == id)
            return true;

    return false;
}

static bool CanCreateLink(Pin* a, Pin* b)
{
    if (!a || !b || a == b || a->Kind == b->Kind || a->Type != b->Type || a->Node == b->Node)
        return false;

    return true;
}

//static void DrawItemRect(ImColor color, float expand = 0.0f)
//{
//    ImGui::GetWindowDrawList()->AddRect(
//        ImGui::GetItemRectMin() - ImVec2(expand, expand),
//        ImGui::GetItemRectMax() + ImVec2(expand, expand),
//        color);
//};

//static void FillItemRect(ImColor color, float expand = 0.0f, float rounding = 0.0f)
//{
//    ImGui::GetWindowDrawList()->AddRectFilled(
//        ImGui::GetItemRectMin() - ImVec2(expand, expand),
//        ImGui::GetItemRectMax() + ImVec2(expand, expand),
//        color, rounding);
//};

// NODOS DEV ==================================================================
// Test to enforce acyclicism
// This is probably the first recursive function I've wrote.  That shouldn't be
// a true statement.
// ============================================================================
bool isNodeAncestor(Node* Ancestor, Node* Decendent) {
    auto decendent_inputs = Decendent->Inputs;

    qDebug() << "-------------------------------------------------------";
    qDebug() << "Testing the ancestors of: " << Decendent->Name.c_str();
    qDebug() << "-------------------------------------------------------";

    // Early return for nodes that don't have inputs
    if(decendent_inputs.size() == 0) {
        qDebug() << Decendent->Name.c_str() <<  " has no input on pins ... skipping";
        return false;
    }

    // Handle nodes that have inputs.
    for(Pin p : decendent_inputs) {
        // Early return for unlinked pins
        if(!IsPinLinked(p.ID)) {
            qDebug() << p.Node->Name.c_str() <<  " has no input on pin: " << p.Name.c_str() << " ... skipping";
            continue;
        }
        // Handle a linked pin.  Pins do NOT know who they are attached to.
        // We have to search the links for the connected node.
        std::vector<Node*> AncestorNodes;
        for (auto& link : s_Links) {
            if(link.EndPinID == p.ID) {
                auto n = FindPin(link.StartPinID)->Node;
                qDebug() << n->Name.c_str() << "Was found as an ancestor node";

                if (n->ID == Ancestor->ID) {
                    qDebug() << "Returning True because: " << n->Name.c_str() << " == " << Ancestor->Name.c_str();
                    return true;
                } else {
                    AncestorNodes.push_back(n);
                }
            }
        }

        for(auto AncestorNode : AncestorNodes) {
            qDebug() << "Testing " << AncestorNode->Name.c_str() << "...";
            if(!AncestorNode) {
                qDebug() << "Something very bad happened here";
                return false;
            } else {
                if(AncestorNode->ID == Ancestor->ID) {
                    qDebug() << "Returning True because: " << AncestorNode->Name.c_str() << " == " << Ancestor->Name.c_str();
                    return true;
                } else {
                   if(isNodeAncestor(Ancestor,AncestorNode))
                       return true;
                } // here we let pass through!
            qDebug() << "Got no matches under: " << AncestorNode->Name.c_str();
            }
        } // Done searching ancestor nodes
    } // Done searching pins
}

void NodosWidget::NodeWidget_Initialize()
{

    // NODOS DEV ===================================================
    // Register nodes from the user's node description forms.    
    NodosSession.RegisterNewNode(node_defs::import_animal::ConstructDefinition());

    // NODOS DEV ===================================================
    // Config is what this system calls a mechanism to move node-related data to and from
    // users (nodos) and the backend (imgui_node_editor).
    //
    // The "save" callbacks are called whenever anything changes that config data.
    // so it will happen constantly throughout use.  Happens for new nodes
    //
    // The "load" callbacks are called if the backend encounters a new node ID for the first time
    // while handling most calls, including BeginNode and CreateNode.  When you're making
    // a new node during runtime, your load callback must say there is no data.
    //
    // So this is how config shit is handled:
    // If there's a LoadSettings and SaveSettings callback, then it doesn't use json files.
    // Else, it uses the SettingsFile.
    //
    // When implementing the LoadSettings and SaveSettings, you have to pass the sizeT back.
    // It will get called twice - once to probe the size, and then once to load the
    // pointer.  So you will need to check if data is null before putting shit into it.
    //
    // imgui_node_editor.cpp  ->  std::string ed::Config::Load()
    // if LoadSettings callback exists...
    // Call it first with a nullpointer to get the size.
    // if it's non-zero, then call it a second time passing a correctly sized buffer.

    // Config structure holds callsbacks for backend-frontend serialization transactions.
    ed::Config config;

    config.SaveSettings = [](const char* data, size_t size, ax::NodeEditor::SaveReasonFlags reason, void* userPointer) -> bool
    {
        s_BlueprintData.reserve(size); //maybe not needed
        s_BlueprintData.assign(data);
        std::ofstream out("project.txt");
        out << s_BlueprintData;
        return true;
    };

    config.LoadSettings = [](char* data, void* userPointer)->size_t {
        std::ifstream in("project.txt");
        std::stringstream b;
        b << in.rdbuf();
        s_BlueprintData = b.str();

        size_t size = s_BlueprintData.size();
        if(data) {
            memcpy(data,s_BlueprintData.c_str(),size);
        }
        return size;
    };

    config.LoadNodeSettings = [](ed::NodeId nodeId, char* data, void* userPointer) -> size_t
    {
        auto node = FindNode(nodeId);
        if (!node)
            return 0;

        if (data != nullptr)
            memcpy(data, node->State.data(), node->State.size());
        return node->State.size();
    };

    config.SaveNodeSettings = [](ed::NodeId nodeId, const char* data, size_t size, ed::SaveReasonFlags reason, void* userPointer) -> bool
    {
        auto node = FindNode(nodeId);
        if (!node)
            return false;

        node->State.assign(data, size);

        TouchNode(nodeId);

        return true;
    };

    m_Editor = ed::CreateEditor(&config);
    ed::SetCurrentEditor(m_Editor);

    // ====================================================================================================================================
    // NODOS DEV - populate graph with nodes.  This should happen on project loads.
    // The SPAWN *** calls don't fill out the reflective pin's members.  After these calls, you have to call
    // BuildNodes();
    //
    // The ed::SetNodePosition() Call shouldn't be used like this because it keeps the node
    // position from deserializing correctly
    // ====================================================================================================================================

    //Node* node;
    //node = SpawnInputActionNode(s_Nodes);
    //node = SpawnBranchNode(s_Nodes);
    //node = SpawnDoNNode(s_Nodes);
    //node = SpawnOutputActionNode(s_Nodes);
    //node = SpawnSetTimerNode(s_Nodes);
    //
    //node = SpawnTreeSequenceNode(s_Nodes);
    //node = SpawnTreeTaskNode(s_Nodes);
    //node = SpawnTreeTask2Node(s_Nodes);
    //
    //node = SpawnComment(s_Nodes);
    //node = SpawnComment(s_Nodes);
    //node = SpawnLessNode(s_Nodes);
    //node = SpawnWeirdNode(s_Nodes);
    //node = SpawnMessageNode(s_Nodes);
    //node = SpawnPrintStringNode(s_Nodes);
    //
    //node = SpawnHoudiniTransformNode(s_Nodes);
    //node = SpawnHoudiniGroupNode(s_Nodes);
    //
    //ed::NavigateToContent();
    //
    //BuildNodes();
    //
    //s_Links.push_back(Link(GetNextLinkId(), s_Nodes[5].Outputs[0].ID, s_Nodes[6].Inputs[0].ID));
    //s_Links.push_back(Link(GetNextLinkId(), s_Nodes[5].Outputs[0].ID, s_Nodes[7].Inputs[0].ID));
    //
    //s_Links.push_back(Link(GetNextLinkId(), s_Nodes[14].Outputs[0].ID, s_Nodes[15].Inputs[0].ID));

    s_HeaderBackground = textures.LoadTexture("Data/BlueprintBackground.png");
    s_SaveIcon         = textures.LoadTexture("Data/ic_save_white_24dp.png");
    s_RestoreIcon      = textures.LoadTexture("Data/ic_restore_white_24dp.png");


    // Extremely bad deserialization system
    // PHASE ONE - READ FILE TO MEMORY --------------------------------------------
    std::string line; // tracks current line in file read loop
    std::ifstream inf("nodos_project.txt");
    int id = 0; // actual node id.  Note that the node id and s_Nodes[x] index are NOT THE SAME.
    int highest_id = 0; // Track highest ID encountered, so we can seed the s_NextId variable which is used a lot in GetNextId()
    std::string NodeName;  // Actually node type
    std::stringstream PropBuffer; // Accumulator for properties lines in a loop.
    std::string Properties; // Whole, intact, Properties table after the loop.
    int PropertiesCount; // Count of properties lines under a node section.

    // Processes each node in turn.  Note there are loads
    // of more getline statements inside this loop, so this outer loop ends up iterating on whole node boundaries.
    while(std::getline(inf,line))
    {
        // Shuttle data into ID, NodeName, and Properties variables.
        // first line is ID, which links us back to the file that has the node positions and zoom
        id = std::stol(line);

        // The entire example relies heavily on s_NextId to generate fresh IDs.
        // After serialization, s_NextId is usually 1, which overlaps our
        // re-serialized data.
        highest_id = std::max(highest_id,id);

        // advance to next line
        std::getline(inf,line);
        // node type is listed here
        NodeName = line;
        // advance to next line
        std::getline(inf,line);
        // count of properties lines are here
        PropertiesCount = std::stol(line);
        // read in properties line by line
        for(int i = 0; i < PropertiesCount; i++) {
            std::getline(inf,line);
            // note that we have to re-add the endline because getline consumes it.
            PropBuffer << line <<std::endl;
        }
        // now we have all the propreties!
        Properties = PropBuffer.str();

        // PHASE TWO - INSTANTIATE NODES ------------------------------------------
        // Use data in Nodename and Properties to instantiate nodes from the registry
        // (new node definition system)
        if (NodosSession.NodeRegistry.count(NodeName) > 0)
        {
            NodosSession.RestoreRegistryNode(NodeName,&Properties,id);
        } else {
            // This mess is only here to support the old examples. We can remove this
            // when the old examples are ported to the new node_defs system.
            //
            // Call the appropriate example node spawner.  You must do this
            // because the spawners have the pin layout information, and pin instantiation
            // must be done to keep the ID alignment.
                     if (NodeName == "InputAction Fire") {SpawnInputActionNode(NodosSession);}
                else if (NodeName == "Branch")           {SpawnBranchNode(NodosSession);}
                else if (NodeName == "Do N")             {SpawnDoNNode(NodosSession);}
                else if (NodeName == "OutputAction")     {SpawnOutputActionNode(NodosSession);}
                else if (NodeName == "Print String")     {SpawnPrintStringNode(NodosSession);}
                else if (NodeName == "")                 {SpawnMessageNode(NodosSession);}
                else if (NodeName == "Set Timer")        {SpawnSetTimerNode(NodosSession);}
                else if (NodeName == "<")                {SpawnLessNode(NodosSession);}
                else if (NodeName == "o.O")              {SpawnWeirdNode(NodosSession);}
                else if (NodeName == "Single Line Trace by Channel") {SpawnTraceByChannelNode(NodosSession);}
                else if (NodeName == "Sequence")         {SpawnTreeSequenceNode(NodosSession);}
                else if (NodeName == "Move To")          {SpawnTreeTaskNode(NodosSession);}
                else if (NodeName == "Random Wait")      {SpawnTreeTask2Node(NodosSession);}
                else if (NodeName == "Test Comment")     {SpawnComment(NodosSession);}
                else if (NodeName == "Transform")        {SpawnHoudiniTransformNode(NodosSession);}
                else if (NodeName == "Group")            {SpawnHoudiniGroupNode(NodosSession);}
                else {  throw std::invalid_argument("Deserializer encountered a unrecognized legacy node name: " + NodeName);}
                s_Nodes.back().Properties.deseralize(Properties);
        } // Done with node instantiation.
    } // Done with a node processing section.  Loop back if there's another node (more lines in getline)
    // Make pins and node reference reflective.
    BuildNodes();
    //auto& io = ImGui::GetIO();
}

void NodosWidget::NodeWidget_Finalize()
{
    textures.DestroyTexture(s_RestoreIcon);
    textures.DestroyTexture(s_SaveIcon);
    textures.DestroyTexture(s_HeaderBackground);
    s_RestoreIcon = nullptr;
    s_SaveIcon = nullptr;
    s_HeaderBackground = nullptr;

    // Extremely bad serilzation system    
    std::ofstream out("nodos_project.txt");
    // For every node in s_Nodes...
    for (unsigned long long i = 0; i < s_Nodes.size(); i++)
    {
        // First line is ID
        out << s_Nodes[i].ID.Get() << std::endl;
        // Next line is Name (node type)
        out << s_Nodes[i].Name << std::endl;
        int count = 0;
        // The next line is a number describing the count of properties lines
        std::string props = s_Nodes[i].Properties.serialize(count);
        out << count << std::endl;
        // Then the next lines are the actual property lines.
        out << props;
    }

    if (m_Editor)
    {
        ed::DestroyEditor(m_Editor);
        m_Editor = nullptr;
    }
}

static bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f)
{
    using namespace ImGui;
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiID id = window->GetID("##Splitter");
    ImRect bb;
    bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
    bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
    return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
}

ImColor GetIconColor(PinType type)
{
    switch (type)
    {
        default:
        case PinType::Flow:     return ImColor(255, 255, 255);
        case PinType::Bool:     return ImColor(220,  48,  48);
        case PinType::Int:      return ImColor( 68, 201, 156);
        case PinType::Float:    return ImColor(147, 226,  74);
        case PinType::String:   return ImColor(124,  21, 153);
        case PinType::Object:   return ImColor( 51, 150, 215);
        case PinType::Function: return ImColor(218,   0, 183);
        case PinType::Delegate: return ImColor(255,  48,  48);
    }
};

void DrawPinIcon(const Pin& pin, bool connected, int alpha)
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

void ShowStyleEditor(bool* show = nullptr)
{
    if (!ImGui::Begin("Style", show))
    {
        ImGui::End();
        return;
    }

    auto paneWidth = ImGui::GetContentRegionAvailWidth();

    auto& editorStyle = ed::GetStyle();
    ImGui::BeginHorizontal("Style buttons", ImVec2(paneWidth, 0), 1.0f);
    ImGui::TextUnformatted("Values");
    ImGui::Spring();
    if (ImGui::Button("Reset to defaults"))
        editorStyle = ed::Style();
    ImGui::EndHorizontal();
    ImGui::Spacing();
    ImGui::DragFloat4("Node Padding", &editorStyle.NodePadding.x, 0.1f, 0.0f, 40.0f);
    ImGui::DragFloat("Node Rounding", &editorStyle.NodeRounding, 0.1f, 0.0f, 40.0f);
    ImGui::DragFloat("Node Border Width", &editorStyle.NodeBorderWidth, 0.1f, 0.0f, 15.0f);
    ImGui::DragFloat("Hovered Node Border Width", &editorStyle.HoveredNodeBorderWidth, 0.1f, 0.0f, 15.0f);
    ImGui::DragFloat("Selected Node Border Width", &editorStyle.SelectedNodeBorderWidth, 0.1f, 0.0f, 15.0f);
    ImGui::DragFloat("Pin Rounding", &editorStyle.PinRounding, 0.1f, 0.0f, 40.0f);
    ImGui::DragFloat("Pin Border Width", &editorStyle.PinBorderWidth, 0.1f, 0.0f, 15.0f);
    ImGui::DragFloat("Link Strength", &editorStyle.LinkStrength, 1.0f, 0.0f, 500.0f);
    //ImVec2  SourceDirection;
    //ImVec2  TargetDirection;
    ImGui::DragFloat("Scroll Duration", &editorStyle.ScrollDuration, 0.001f, 0.0f, 2.0f);
    ImGui::DragFloat("Flow Marker Distance", &editorStyle.FlowMarkerDistance, 1.0f, 1.0f, 200.0f);
    ImGui::DragFloat("Flow Speed", &editorStyle.FlowSpeed, 1.0f, 1.0f, 2000.0f);
    ImGui::DragFloat("Flow Duration", &editorStyle.FlowDuration, 0.001f, 0.0f, 5.0f);
    //ImVec2  PivotAlignment;
    //ImVec2  PivotSize;
    //ImVec2  PivotScale;
    //float   PinCorners;
    //float   PinRadius;
    //float   PinArrowSize;
    //float   PinArrowWidth;
    ImGui::DragFloat("Group Rounding", &editorStyle.GroupRounding, 0.1f, 0.0f, 40.0f);
    ImGui::DragFloat("Group Border Width", &editorStyle.GroupBorderWidth, 0.1f, 0.0f, 15.0f);

    ImGui::Separator();

    static ImGuiColorEditFlags edit_mode = ImGuiColorEditFlags_RGB;
    ImGui::BeginHorizontal("Color Mode", ImVec2(paneWidth, 0), 1.0f);
    ImGui::TextUnformatted("Filter Colors");
    ImGui::Spring();
    ImGui::RadioButton("RGB", &edit_mode, ImGuiColorEditFlags_RGB);
    ImGui::Spring(0);
    ImGui::RadioButton("HSV", &edit_mode, ImGuiColorEditFlags_HSV);
    ImGui::Spring(0);
    ImGui::RadioButton("HEX", &edit_mode, ImGuiColorEditFlags_HEX);
    ImGui::EndHorizontal();

    static ImGuiTextFilter filter;
    filter.Draw("", paneWidth);

    ImGui::Spacing();

    ImGui::PushItemWidth(-160);
    for (int i = 0; i < ed::StyleColor_Count; ++i)
    {
        auto name = ed::GetStyleColorName((ed::StyleColor)i);
        if (!filter.PassFilter(name))
            continue;

        ImGui::ColorEdit4(name, &editorStyle.Colors[i].x, edit_mode);
    }
    ImGui::PopItemWidth();

    ImGui::End();
}

void NodosWidget::NodeWidget_Frame()
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

    static ed::NodeId contextNodeId      = 0;
    static ed::LinkId contextLinkId      = 0;
    static ed::PinId  contextPinId       = 0;
    static bool createNewNode  = false;
    static Pin* newNodeLinkPin = nullptr;
    static Pin* newLinkPin     = nullptr;

    //static float leftPaneWidth  = 400.0f;
    //static float rightPaneWidth = 800.0f;
    //Splitter(true, 4.0f, &leftPaneWidth, &rightPaneWidth, 50.0f, 50.0f);

    //ShowLeftPane(leftPaneWidth - 4.0f);

    //ImGui::SameLine(0.0f, 12.0f);



    // ====================================================================================================================================
    // NODOS DEV - Immediate Mode node drawing.
    // ====================================================================================================================================
    ed::Begin("Node editor");
    {
        auto cursorTopLeft = ImGui::GetCursorScreenPos();

        util::BlueprintNodeBuilder builder(s_HeaderBackground, textures.GetTextureWidth(s_HeaderBackground), textures.GetTextureHeight(s_HeaderBackground));

        // ====================================================================================================================================
        // NODOS DEV - draw nodes of type Blueprint and Simple
        // ====================================================================================================================================
        for (auto& node : s_Nodes)
        {
            // Guard for non-blueprints and non-simple nodes --------------------------------------------------------
            if (node.Type != NodeType::Blueprint && node.Type != NodeType::Simple)
                continue;
            // Load isSimple
            const auto isSimple = node.Type == NodeType::Simple;

            bool hasOutputDelegates = false;
            for (auto& output : node.Outputs)
                if (output.Type == PinType::Delegate)
                    hasOutputDelegates = true;
            // Build header, which can include delegates (header output pin) --------------------------------------------------------
            builder.Begin(node.ID);
                if (!isSimple)
                {
                    builder.Header(node.Color);
                        ImGui::Spring(0);
                        ImGui::TextUnformatted(node.Name.c_str());
                        ImGui::Spring(1);
                        ImGui::Dummy(ImVec2(0, 28));
                        if (hasOutputDelegates)
                        {
                            ImGui::BeginVertical("delegates", ImVec2(0, 28));
                            ImGui::Spring(1, 0);
                            for (auto& output : node.Outputs)
                            {
                                if (output.Type != PinType::Delegate)
                                    continue;

                                auto alpha = ImGui::GetStyle().Alpha;
                                if (newLinkPin && !CanCreateLink(newLinkPin, &output) && &output != newLinkPin)
                                    alpha = alpha * (48.0f / 255.0f);

                                ed::BeginPin(output.ID, ed::PinKind::Output);
                                ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));
                                ed::PinPivotSize(ImVec2(0, 0));
                                ImGui::BeginHorizontal(output.ID.AsPointer());
                                ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
                                if (!output.Name.empty())
                                {
                                    ImGui::TextUnformatted(output.Name.c_str());
                                    ImGui::Spring(0);
                                }
                                DrawPinIcon(output, IsPinLinked(output.ID), (int)(alpha * 255));
                                ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.x / 2);
                                ImGui::EndHorizontal();
                                ImGui::PopStyleVar();
                                ed::EndPin();

                                //DrawItemRect(ImColor(255, 0, 0));
                            }
                            ImGui::Spring(1, 0);
                            ImGui::EndVertical();
                            ImGui::Spring(0, ImGui::GetStyle().ItemSpacing.x / 2);
                        }
                        else
                            ImGui::Spring(0);
                    builder.EndHeader();
                }

                // Build node inputs, including pin names.  note "bool" type has button hard-coded.--------------------------------------------------------
                for (auto& input : node.Inputs)
                {
                    auto alpha = ImGui::GetStyle().Alpha;
                    if (newLinkPin && !CanCreateLink(newLinkPin, &input) && &input != newLinkPin)
                        alpha = alpha * (48.0f / 255.0f);

                    builder.Input(input.ID);
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
                    DrawPinIcon(input, IsPinLinked(input.ID), (int)(alpha * 255));
                    ImGui::Spring(0);
                    if (!input.Name.empty())
                    {
                        ImGui::TextUnformatted(input.Name.c_str());
                        ImGui::Spring(0);
                    }
                    if (input.Type == PinType::Bool)
                    {
                         ImGui::Button("Hello");
                         ImGui::Spring(0);
                    }
                    ImGui::PopStyleVar();
                    builder.EndInput();
                }

                // Optional "middle" part.  Used in simple blueprints only.--------------------------------------------------------
                // "For simple blueprints, has the node name in the center."
                // But for others, it draws the properties... stuff
                if (isSimple)
                {
                    builder.Middle();

                    ImGui::Spring(1, 0);
                    ImGui::TextUnformatted(node.Name.c_str());
                    ImGui::Spring(1, 0);
                } else {
                    builder.Middle();                    
                    if(NodosSession.NodeRegistry.count(node.Name) > 0){
                        NodosSession.NodeRegistry[node.Name].DrawAndEditProperties(node.Properties);
                    }else{
                        im_draw_basic_widgets(node.Properties);
                    }
                }

                // output column.
                // DEV - experiment to have buffers be owned by individual nodes --------------------------------------------------------
                for (auto& output : node.Outputs)
                {
                    //
                    if (!isSimple && output.Type == PinType::Delegate)
                        continue;

                    auto alpha = ImGui::GetStyle().Alpha;
                    if (newLinkPin && !CanCreateLink(newLinkPin, &output) && &output != newLinkPin)
                        alpha = alpha * (48.0f / 255.0f);

                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
                    builder.Output(output.ID);
                    if (output.Type == PinType::String)
                    {
                        static char buffer[128] = "Edit Me\nMultiline!";
                        static bool wasActive = false;

                        ImGui::PushItemWidth(100.0f);
                        ImGui::InputText("##edit", buffer, 127);
                        ImGui::PopItemWidth();
                        if (ImGui::IsItemActive() && !wasActive)
                        {
                            ed::EnableShortcuts(false);
                            wasActive = true;
                        }
                        else if (!ImGui::IsItemActive() && wasActive)
                        {
                            ed::EnableShortcuts(true);
                            wasActive = false;
                        }
                        ImGui::Spring(0);
                    }
                    if (!output.Name.empty())
                    {
                        ImGui::Spring(0);
                        ImGui::TextUnformatted(output.Name.c_str());
                    }
                    ImGui::Spring(0);
                    DrawPinIcon(output, IsPinLinked(output.ID), (int)(alpha * 255));
                    ImGui::PopStyleVar();
                    builder.EndOutput();
                }
            builder.End();
        }
        // ====================================================================================================================================
        // NODOS DEV - draw nodes of type Tree
        // ====================================================================================================================================
        for (auto& node : s_Nodes)
        {
            if (node.Type != NodeType::Tree)
                continue;

            const float rounding = 5.0f;
            const float padding  = 12.0f;

            const auto pinBackground = ed::GetStyle().Colors[ed::StyleColor_NodeBg];

            ed::PushStyleColor(ed::StyleColor_NodeBg,        ImColor(128, 128, 128, 200));
            ed::PushStyleColor(ed::StyleColor_NodeBorder,    ImColor( 32,  32,  32, 200));
            ed::PushStyleColor(ed::StyleColor_PinRect,       ImColor( 60, 180, 255, 150));
            ed::PushStyleColor(ed::StyleColor_PinRectBorder, ImColor( 60, 180, 255, 150));

            ed::PushStyleVar(ed::StyleVar_NodePadding,  ImVec4(0, 0, 0, 0));
            ed::PushStyleVar(ed::StyleVar_NodeRounding, rounding);
            ed::PushStyleVar(ed::StyleVar_SourceDirection, ImVec2(0.0f,  1.0f));
            ed::PushStyleVar(ed::StyleVar_TargetDirection, ImVec2(0.0f, -1.0f));
            ed::PushStyleVar(ed::StyleVar_LinkStrength, 0.0f);
            ed::PushStyleVar(ed::StyleVar_PinBorderWidth, 1.0f);
            ed::PushStyleVar(ed::StyleVar_PinRadius, 5.0f);
            ed::BeginNode(node.ID);

            ImGui::BeginVertical(node.ID.AsPointer());
            ImGui::BeginHorizontal("inputs");
            ImGui::Spring(0, padding * 2);

            ImRect inputsRect;
            int inputAlpha = 200;
            if (!node.Inputs.empty())
            {
                    auto& pin = node.Inputs[0];
                    ImGui::Dummy(ImVec2(0, padding));
                    ImGui::Spring(1, 0);
                    inputsRect = ImGui_GetItemRect();

                    ed::PushStyleVar(ed::StyleVar_PinArrowSize, 10.0f);
                    ed::PushStyleVar(ed::StyleVar_PinArrowWidth, 10.0f);
                    ed::PushStyleVar(ed::StyleVar_PinCorners, 12);
                    ed::BeginPin(pin.ID, ed::PinKind::Input);
                    ed::PinPivotRect(inputsRect.GetTL(), inputsRect.GetBR());
                    ed::PinRect(inputsRect.GetTL(), inputsRect.GetBR());
                    ed::EndPin();
                    ed::PopStyleVar(3);

                    if (newLinkPin && !CanCreateLink(newLinkPin, &pin) && &pin != newLinkPin)
                        inputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));
            }
            else
                ImGui::Dummy(ImVec2(0, padding));

            ImGui::Spring(0, padding * 2);
            ImGui::EndHorizontal();

            ImGui::BeginHorizontal("content_frame");
            ImGui::Spring(1, padding);

            ImGui::BeginVertical("content", ImVec2(0.0f, 0.0f));
            ImGui::Dummy(ImVec2(160, 0));
            ImGui::Spring(1);
            ImGui::TextUnformatted(node.Name.c_str());
            ImGui::Spring(1);
            ImGui::EndVertical();
            auto contentRect = ImGui_GetItemRect();

            ImGui::Spring(1, padding);
            ImGui::EndHorizontal();

            ImGui::BeginHorizontal("outputs");
            ImGui::Spring(0, padding * 2);

            ImRect outputsRect;
            int outputAlpha = 200;
            if (!node.Outputs.empty())
            {
                auto& pin = node.Outputs[0];
                ImGui::Dummy(ImVec2(0, padding));
                ImGui::Spring(1, 0);
                outputsRect = ImGui_GetItemRect();

                ed::PushStyleVar(ed::StyleVar_PinCorners, 3);
                ed::BeginPin(pin.ID, ed::PinKind::Output);
                ed::PinPivotRect(outputsRect.GetTL(), outputsRect.GetBR());
                ed::PinRect(outputsRect.GetTL(), outputsRect.GetBR());
                ed::EndPin();
                ed::PopStyleVar();

                if (newLinkPin && !CanCreateLink(newLinkPin, &pin) && &pin != newLinkPin)
                    outputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));
            }
            else
                ImGui::Dummy(ImVec2(0, padding));

            ImGui::Spring(0, padding * 2);
            ImGui::EndHorizontal();

            ImGui::EndVertical();

            ed::EndNode();
            ed::PopStyleVar(7);
            ed::PopStyleColor(4);

            auto drawList = ed::GetNodeBackgroundDrawList(node.ID);

            //const auto fringeScale = ImGui::GetStyle().AntiAliasFringeScale;
            //const auto unitSize    = 1.0f / fringeScale;

            //const auto ImDrawList_AddRect = [](ImDrawList* drawList, const ImVec2& a, const ImVec2& b, ImU32 col, float rounding, int rounding_corners, float thickness)
            //{
            //    if ((col >> 24) == 0)
            //        return;
            //    drawList->PathRect(a, b, rounding, rounding_corners);
            //    drawList->PathStroke(col, true, thickness);
            //};

            drawList->AddRectFilled(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(),
                IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, 12);
            //ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
            drawList->AddRect(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(),
                IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, 12);
            //ImGui::PopStyleVar();
            drawList->AddRectFilled(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1),
                IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, 3);
            //ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
            drawList->AddRect(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1),
                IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, 3);
            //ImGui::PopStyleVar();
            drawList->AddRectFilled(contentRect.GetTL(), contentRect.GetBR(), IM_COL32(24, 64, 128, 200), 0.0f);
            //ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
            drawList->AddRect(
                contentRect.GetTL(),
                contentRect.GetBR(),
                IM_COL32(48, 128, 255, 100), 0.0f);
            //ImGui::PopStyleVar();
        }

        // ====================================================================================================================================
        // NODOS DEV - draw nodes of type Houdini
        // ====================================================================================================================================
        for (auto& node : s_Nodes)
        {
            if (node.Type != NodeType::Houdini)
                continue;

            const float rounding = 10.0f;
            const float padding  = 12.0f;


            ed::PushStyleColor(ed::StyleColor_NodeBg,        ImColor(229, 229, 229, 200));
            ed::PushStyleColor(ed::StyleColor_NodeBorder,    ImColor(125, 125, 125, 200));
            ed::PushStyleColor(ed::StyleColor_PinRect,       ImColor(229, 229, 229, 60));
            ed::PushStyleColor(ed::StyleColor_PinRectBorder, ImColor(125, 125, 125, 60));

            const auto pinBackground = ed::GetStyle().Colors[ed::StyleColor_NodeBg];

            ed::PushStyleVar(ed::StyleVar_NodePadding,  ImVec4(0, 0, 0, 0));
            ed::PushStyleVar(ed::StyleVar_NodeRounding, rounding);
            ed::PushStyleVar(ed::StyleVar_SourceDirection, ImVec2(0.0f,  1.0f));
            ed::PushStyleVar(ed::StyleVar_TargetDirection, ImVec2(0.0f, -1.0f));
            ed::PushStyleVar(ed::StyleVar_LinkStrength, 0.0f);
            ed::PushStyleVar(ed::StyleVar_PinBorderWidth, 1.0f);
            ed::PushStyleVar(ed::StyleVar_PinRadius, 6.0f);
            ed::BeginNode(node.ID);

            ImGui::BeginVertical(node.ID.AsPointer());
            if (!node.Inputs.empty())
            {
                ImGui::BeginHorizontal("inputs");
                ImGui::Spring(1, 0);

                ImRect inputsRect;
                int inputAlpha = 200;
                for (auto& pin : node.Inputs)
                {
                    ImGui::Dummy(ImVec2(padding, padding));
                    inputsRect = ImGui_GetItemRect();
                    ImGui::Spring(1, 0);
                    inputsRect.Min.y -= padding;
                    inputsRect.Max.y -= padding;

                    //ed::PushStyleVar(ed::StyleVar_PinArrowSize, 10.0f);
                    //ed::PushStyleVar(ed::StyleVar_PinArrowWidth, 10.0f);
                    ed::PushStyleVar(ed::StyleVar_PinCorners, 15);
                    ed::BeginPin(pin.ID, ed::PinKind::Input);
                    ed::PinPivotRect(inputsRect.GetCenter(), inputsRect.GetCenter());
                    ed::PinRect(inputsRect.GetTL(), inputsRect.GetBR());
                    ed::EndPin();
                    //ed::PopStyleVar(3);
                    ed::PopStyleVar(1);

                    auto drawList = ImGui::GetWindowDrawList();
                    drawList->AddRectFilled(inputsRect.GetTL(), inputsRect.GetBR(),
                        IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, 15);
                    drawList->AddRect(inputsRect.GetTL(), inputsRect.GetBR(),
                        IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, 15);

                    if (newLinkPin && !CanCreateLink(newLinkPin, &pin) && &pin != newLinkPin)
                        inputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));
                }

                //ImGui::Spring(1, 0);
                ImGui::EndHorizontal();
            }

            ImGui::BeginHorizontal("content_frame");
            ImGui::Spring(1, padding);

            ImGui::BeginVertical("content", ImVec2(0.0f, 0.0f));
            ImGui::Dummy(ImVec2(160, 0));
            ImGui::Spring(1);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
            ImGui::TextUnformatted(node.Name.c_str());
            ImGui::PopStyleColor();
            ImGui::Spring(1);
            ImGui::EndVertical();
            auto contentRect = ImGui_GetItemRect();

            ImGui::Spring(1, padding);
            ImGui::EndHorizontal();

            if (!node.Outputs.empty())
            {
                ImGui::BeginHorizontal("outputs");
                ImGui::Spring(1, 0);

                ImRect outputsRect;
                int outputAlpha = 200;
                for (auto& pin : node.Outputs)
                {
                    ImGui::Dummy(ImVec2(padding, padding));
                    outputsRect = ImGui_GetItemRect();
                    ImGui::Spring(1, 0);
                    outputsRect.Min.y += padding;
                    outputsRect.Max.y += padding;

                    ed::PushStyleVar(ed::StyleVar_PinCorners, 3);
                    ed::BeginPin(pin.ID, ed::PinKind::Output);
                    ed::PinPivotRect(outputsRect.GetCenter(), outputsRect.GetCenter());
                    ed::PinRect(outputsRect.GetTL(), outputsRect.GetBR());
                    ed::EndPin();
                    ed::PopStyleVar();

                    auto drawList = ImGui::GetWindowDrawList();
                    drawList->AddRectFilled(outputsRect.GetTL(), outputsRect.GetBR(),
                        IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, 15);
                    drawList->AddRect(outputsRect.GetTL(), outputsRect.GetBR(),
                        IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, 15);


                    if (newLinkPin && !CanCreateLink(newLinkPin, &pin) && &pin != newLinkPin)
                        outputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));
                }

                ImGui::EndHorizontal();
            }

            ImGui::EndVertical();

            ed::EndNode();
            ed::PopStyleVar(7);
            ed::PopStyleColor(4);

            auto drawList = ed::GetNodeBackgroundDrawList(node.ID);

            //const auto fringeScale = ImGui::GetStyle().AntiAliasFringeScale;
            //const auto unitSize    = 1.0f / fringeScale;

            //const auto ImDrawList_AddRect = [](ImDrawList* drawList, const ImVec2& a, const ImVec2& b, ImU32 col, float rounding, int rounding_corners, float thickness)
            //{
            //    if ((col >> 24) == 0)
            //        return;
            //    drawList->PathRect(a, b, rounding, rounding_corners);
            //    drawList->PathStroke(col, true, thickness);
            //};

            //drawList->AddRectFilled(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(),
            //    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, 12);
            //ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
            //drawList->AddRect(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(),
            //    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, 12);
            //ImGui::PopStyleVar();
            //drawList->AddRectFilled(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1),
            //    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, 3);
            ////ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
            //drawList->AddRect(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1),
            //    IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, 3);
            ////ImGui::PopStyleVar();
            //drawList->AddRectFilled(contentRect.GetTL(), contentRect.GetBR(), IM_COL32(24, 64, 128, 200), 0.0f);
            //ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
            //drawList->AddRect(
            //    contentRect.GetTL(),
            //    contentRect.GetBR(),
            //    IM_COL32(48, 128, 255, 100), 0.0f);
            //ImGui::PopStyleVar();
        }
        // ====================================================================================================================================
        // NODOS DEV - draw nodes of type Comment
        // ====================================================================================================================================
        for (auto& node : s_Nodes)
        {
            if (node.Type != NodeType::Comment)
                continue;

            const float commentAlpha = 0.75f;

            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, commentAlpha);
            ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(255, 255, 255, 64));
            ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(255, 255, 255, 64));
            ed::BeginNode(node.ID);
            ImGui::PushID(node.ID.AsPointer());
            ImGui::BeginVertical("content");
            ImGui::BeginHorizontal("horizontal");
            ImGui::Spring(1);
            ImGui::TextUnformatted(node.Name.c_str());
            ImGui::Spring(1);
            ImGui::EndHorizontal();
            ed::Group(node.Size);
            ImGui::EndVertical();
            ImGui::PopID();
            ed::EndNode();
            ed::PopStyleColor(2);
            ImGui::PopStyleVar();

            if (ed::BeginGroupHint(node.ID))
            {
                //auto alpha   = static_cast<int>(commentAlpha * ImGui::GetStyle().Alpha * 255);
                auto bgAlpha = static_cast<int>(ImGui::GetStyle().Alpha * 255);

                //ImGui::PushStyleVar(ImGuiStyleVar_Alpha, commentAlpha * ImGui::GetStyle().Alpha);

                auto min = ed::GetGroupMin();
                //auto max = ed::GetGroupMax();

                ImGui::SetCursorScreenPos(min - ImVec2(-8, ImGui::GetTextLineHeightWithSpacing() + 4));
                ImGui::BeginGroup();
                ImGui::TextUnformatted(node.Name.c_str());
                ImGui::EndGroup();

                auto drawList = ed::GetHintBackgroundDrawList();

                auto hintBounds      = ImGui_GetItemRect();
                auto hintFrameBounds = ImRect_Expanded(hintBounds, 8, 4);

                drawList->AddRectFilled(
                    hintFrameBounds.GetTL(),
                    hintFrameBounds.GetBR(),
                    IM_COL32(255, 255, 255, 64 * bgAlpha / 255), 4.0f);

                drawList->AddRect(
                    hintFrameBounds.GetTL(),
                    hintFrameBounds.GetBR(),
                    IM_COL32(255, 255, 255, 128 * bgAlpha / 255), 4.0f);

                //ImGui::PopStyleVar();
            }
            ed::EndGroupHint();
        }
        // ====================================================================================================================================
        // NODOS DEV - draw links
        // ====================================================================================================================================
        for (auto& link : s_Links)
            ed::Link(link.ID, link.StartPinID, link.EndPinID, link.Color, 2.0f);

        // ====================================================================================================================================
        // NODOS DEV - Graph interactions in immediate mode.
        // BeginCreate() - Handle dragging a link out of a pin
        //     QueryNewLink()
        //         AcceptNewItem()
        //         RejectNewItem()
        //     QueryNewNode(PinId* pinId, const ImVec4& color, float thickness)
        //         AcceptNewItem()
        //         RejectNewItem()
        // EndCreate()
        // ====================================================================================================================================
        if (!createNewNode)
        {
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

                // QueryNewLink is true when you've started dragging off a pin, and you're now hovering over a pin
                // This does fire right off the bat typically, as you're over the pin you just dragged off of.
                // QueryNewLink populates its arguments with the pin ids, then you can figure out if you
                // will allow the connection.  It
                ed::PinId startPinId = 0, endPinId = 0;
                if (ed::QueryNewLink(&startPinId, &endPinId))
                {
                    auto startPin = FindPin(startPinId);
                    auto endPin   = FindPin(endPinId);
                    //qDebug() << startPin->ID.Get() << " " << endPin->ID.Get() ;
                    newLinkPin = startPin ? startPin : endPin;

                    if (startPin->Kind == ed::PinKind::Input)
                    {
                        std::swap(startPin, endPin);
                        std::swap(startPinId, endPinId);
                    }

                    // Startpin node should never connect to an ancestor end-pin
                    auto startNode = startPin->Node;
                    auto endNode = endPin->Node;




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
                                s_Links.emplace_back(Link(NodosSession.GetNextId(), startPinId, endPinId));
                                s_Links.back().Color = GetIconColor(startPin->Type);
                            }
                        }
                    }
                }

                ed::PinId pinId = 0;
                if (ed::QueryNewNode(&pinId))
                {
                    newLinkPin = FindPin(pinId);
                    if (newLinkPin)
                        showLabel("+ Create Node", ImColor(32, 45, 32, 180));

                    if (ed::AcceptNewItem())
                    {
                        createNewNode  = true;
                        newNodeLinkPin = FindPin(pinId);
                        newLinkPin = nullptr;
                        ed::Suspend();
                        ImGui::OpenPopup("Create New Node");
                        ed::Resume();
                    }
                }
            }
            else
                newLinkPin = nullptr;

            ed::EndCreate();

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
        }

        ImGui::SetCursorScreenPos(cursorTopLeft);
    }

# if 1
    auto openPopupPosition = ImGui::GetMousePos();
    ed::Suspend();
    if (ed::ShowNodeContextMenu(&contextNodeId))
        ImGui::OpenPopup("Node Context Menu");
    else if (ed::ShowPinContextMenu(&contextPinId))
        ImGui::OpenPopup("Pin Context Menu");
    else if (ed::ShowLinkContextMenu(&contextLinkId))
        ImGui::OpenPopup("Link Context Menu");
    else if (ed::ShowBackgroundContextMenu())
    {
        ImGui::OpenPopup("Create New Node");
        newNodeLinkPin = nullptr;
    }
    ed::Resume();

    ed::Suspend();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8, 8));
    if (ImGui::BeginPopup("Node Context Menu"))
    {
        auto node = FindNode(contextNodeId);

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
            ImGui::Text("Unknown node: %p", contextNodeId.AsPointer());
        ImGui::Separator();
        if (ImGui::MenuItem("Delete"))
            ed::DeleteNode(contextNodeId);
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Pin Context Menu"))
    {
        auto pin = FindPin(contextPinId);

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
            ImGui::Text("Unknown pin: %p", contextPinId.AsPointer());

        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Link Context Menu"))
    {
        auto link = FindLink(contextLinkId);

        ImGui::TextUnformatted("Link Context Menu");
        ImGui::Separator();
        if (link)
        {
            ImGui::Text("ID: %p", link->ID.AsPointer());
            ImGui::Text("From: %p", link->StartPinID.AsPointer());
            ImGui::Text("To: %p", link->EndPinID.AsPointer());
        }
        else
            ImGui::Text("Unknown link: %p", contextLinkId.AsPointer());
        ImGui::Separator();
        if (ImGui::MenuItem("Delete"))
            ed::DeleteLink(contextLinkId);
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopup("Create New Node"))
    {
        auto newNodePostion = openPopupPosition;
        //ImGui::SetCursorScreenPos(ImGui::GetMousePosOnOpeningCurrentPopup());

        //auto drawList = ImGui::GetWindowDrawList();
        //drawList->AddCircleFilled(ImGui::GetMousePosOnOpeningCurrentPopup(), 10.0f, 0xFFFF00FF);

        Node* node = nullptr;

        // NODOS DEV ====================================================================================
        // YOU ARE IN THE RIGHT CLICK MENU HANDLER NOW.
        // Populate the right click menu with all the nodes in the registry.
        for(auto nodos: NodosSession.NodeRegistry){
            if (ImGui::MenuItem(nodos.first.c_str())){
                node = NodosSession.NewRegistryNode(nodos.first);
            }
        }

        if (ImGui::MenuItem("Input Action"))
            node = SpawnInputActionNode(NodosSession);
        if (ImGui::MenuItem("Output Action"))
            node = SpawnOutputActionNode(NodosSession);
        if (ImGui::MenuItem("Branch"))
            node = SpawnBranchNode(NodosSession);
        if (ImGui::MenuItem("Do N"))
            node = SpawnDoNNode(NodosSession);
        if (ImGui::MenuItem("Set Timer"))
            node = SpawnSetTimerNode(NodosSession);
        if (ImGui::MenuItem("Less"))
            node = SpawnLessNode(NodosSession);
        if (ImGui::MenuItem("Weird"))
            node = SpawnWeirdNode(NodosSession);
        if (ImGui::MenuItem("Trace by Channel"))
            node = SpawnTraceByChannelNode(NodosSession);
        if (ImGui::MenuItem("Print String"))
            node = SpawnPrintStringNode(NodosSession);
        ImGui::Separator();
        if (ImGui::MenuItem("Comment"))
            node = SpawnComment(NodosSession);
        ImGui::Separator();
        if (ImGui::MenuItem("Sequence"))
            node = SpawnTreeSequenceNode(NodosSession);
        if (ImGui::MenuItem("Move To"))
            node = SpawnTreeTaskNode(NodosSession);
        if (ImGui::MenuItem("Random Wait"))
            node = SpawnTreeTask2Node(NodosSession);
        ImGui::Separator();
        if (ImGui::MenuItem("Message"))
            node = SpawnMessageNode(NodosSession);
        ImGui::Separator();
        if (ImGui::MenuItem("Transform"))
            node = SpawnHoudiniTransformNode(NodosSession);
        if (ImGui::MenuItem("Group"))
            node = SpawnHoudiniGroupNode(NodosSession);

        if (node)
        {
            BuildNodes();

            createNewNode = false;

            ed::SetNodePosition(node->ID, newNodePostion);

            if (auto startPin = newNodeLinkPin)
            {
                auto& pins = startPin->Kind == ed::PinKind::Input ? node->Outputs : node->Inputs;

                for (auto& pin : pins)
                {
                    if (CanCreateLink(startPin, &pin))
                    {
                        auto endPin = &pin;
                        if (startPin->Kind == ed::PinKind::Input)
                            std::swap(startPin, endPin);

                        s_Links.emplace_back(Link(NodosSession.GetNextId(), startPin->ID, endPin->ID));
                        s_Links.back().Color = GetIconColor(startPin->Type);

                        break;
                    }
                }
            }
        }

        ImGui::EndPopup();
    }
    else
        createNewNode = false;
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




    ed::End();

    // ====================================================================================================================================
    // NODOS DEV - outside-of-begin-end field.
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

