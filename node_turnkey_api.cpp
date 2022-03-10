// This source file must have credited to thedmd on github.  This is a heavily modified version of this file, retrieved 12/27/2020:
// https://github.com/thedmd/imgui-node-editor/blob/master/examples/blueprints-example/blueprints-example.cpp

#include "utilities/widgets.h"

#include <imgui_node_editor.h>
#include <example_node_spawner.h>



#include <node_turnkey_types.h>
#include <node_turnkey_api.h>

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
using namespace turnkey::internal;

namespace ed = ax::NodeEditor;
namespace util = ax::NodeEditor::Utilities;
using namespace ax;
using ax::Widgets::IconType;


// Context management.
Context* CreateContext()
{
    return new Context();
}

void DestroyContext(Context* context)
{
    delete context;
}

Context* GetContext()
{
    return &s_Session;
}


void SetContext(Context* context)
{
    s_Session = *context;
}







void turnkey::api::nodos_session_data::RegisterNewNode(NodeDescription NewDescription) {
    NodeRegistry[NewDescription.Type] = NewDescription;
}




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

ax::NodeEditor::NodeId turnkey::api::nodos_session_data::GetNextLinkId()
{
    return ax::NodeEditor::NodeId(GetNextId());
}


turnkey::types::Node* turnkey::api::nodos_session_data::FindNode(ax::NodeEditor::NodeId id)
{
    for (auto& node : s_Nodes)
        if (node.ID == id)
            return &node;

    return nullptr;
}

turnkey::types::Link* turnkey::api::nodos_session_data::FindLink(ax::NodeEditor::LinkId id)
{
    for (auto& link : s_Links)
        if (link.ID == id)
            return &link;

    return nullptr;
}

turnkey::types::Pin* turnkey::api::nodos_session_data::FindPin(ax::NodeEditor::PinId id)
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

bool turnkey::api::nodos_session_data::IsPinLinked(ax::NodeEditor::PinId id)
{
    if (!id)
        return false;

    for (auto& link : s_Links)
        if (link.StartPinID == id || link.EndPinID == id)
            return true;

    return false;
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

bool turnkey::internal::isNodeAncestor(types::Node* Ancestor, types::Node* Decendent) {
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


void turnkey::internal::nodos_session_data::Initialize(void)
{

    // NODOS DEV ===================================================
    // Register nodes from the user's node description forms.    
    RegisterNewNode(node_defs::import_animal::ConstructDefinition());

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

    // https://stackoverflow.com/questions/19808054/convert-c-function-pointer-to-c-function-pointer/19808250#19808250
    config.UserPointer = (void*) this;
    config.SaveSettings = turnkey::api::nodos_session_data::static_config_save_settings;
    config.LoadSettings = turnkey::api::nodos_session_data::static_config_load_settings;
    config.LoadNodeSettings = turnkey::api::nodos_session_data::static_config_load_node_settings;
    config.SaveNodeSettings = turnkey::api::nodos_session_data::static_config_save_node_settings;

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
        if (NodeRegistry.count(NodeName) > 0)
        {
            RestoreRegistryNode(NodeName,&Properties,id);
        } else {
            // This mess is only here to support the old examples. We can remove this
            // when the old examples are ported to the new node_defs system.
            //
            // Call the appropriate example node spawner.  You must do this
            // because the spawners have the pin layout information, and pin instantiation
            // must be done to keep the ID alignment.
                     if (NodeName == "InputAction Fire") {SpawnInputActionNode(this);}
                else if (NodeName == "Branch")           {SpawnBranchNode(this);}
                else if (NodeName == "Do N")             {SpawnDoNNode(this);}
                else if (NodeName == "OutputAction")     {SpawnOutputActionNode(this);}
                else if (NodeName == "Print String")     {SpawnPrintStringNode(this);}
                else if (NodeName == "")                 {SpawnMessageNode(this);}
                else if (NodeName == "Set Timer")        {SpawnSetTimerNode(this);}
                else if (NodeName == "<")                {SpawnLessNode(this);}
                else if (NodeName == "o.O")              {SpawnWeirdNode(this);}
                else if (NodeName == "Single Line Trace by Channel") {SpawnTraceByChannelNode(this);}
                else if (NodeName == "Sequence")         {SpawnTreeSequenceNode(this);}
                else if (NodeName == "Move To")          {SpawnTreeTaskNode(this);}
                else if (NodeName == "Random Wait")      {SpawnTreeTask2Node(this);}
                else if (NodeName == "Test Comment")     {SpawnComment(this);}
                else if (NodeName == "Transform")        {SpawnHoudiniTransformNode(this);}
                else if (NodeName == "Group")            {SpawnHoudiniGroupNode(this);}
                else {  throw std::invalid_argument("Deserializer encountered a unrecognized legacy node name: " + NodeName);}
                s_Nodes.back().Properties.deseralize(Properties);
        } // Done with node instantiation.
    } // Done with a node processing section.  Loop back if there's another node (more lines in getline)
    // Make pins and node reference reflective.
    BuildNodes();
    //auto& io = ImGui::GetIO();
}

void turnkey::api::nodos_session_data::Finalize(void)
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


