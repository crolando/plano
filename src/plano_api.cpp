// This source file must have credited to thedmd on github.  This is a heavily modified version of this file, retrieved 12/27/2020:
// https://github.com/thedmd/imgui-node-editor/blob/master/examples/blueprints-example/blueprints-example.cpp

#include "internal/widgets.h"

#include <imgui_node_editor.h>





#define IMGUI_DEFINE_MATH_OPERATORS

#include <imgui_internal.h>

#include <internal/internal.h>
#include <plano_types.h>
#include <plano_api.h>
#include <internal/draw_utils.h> // GetIconColor is needed to color links at link load time


#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <utility>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

using namespace plano::types;
using namespace plano::api;
using namespace plano::internal;

namespace ed = ax::NodeEditor;
namespace util = ax::NodeEditor::Utilities;
using namespace ax;
using ax::Widgets::IconType;

namespace plano {
namespace api {


// Context management.
types::ContextData* CreateContext(const types::ContextCallbacks& Config, const char *texture_path)
{
    types::ContextData* newctx = new ContextData(Config, texture_path); //CTOR handles NodeEditor object creation
    return newctx;
}

void DestroyContext(ContextData* context)
{
    ax::NodeEditor::DestroyEditor(context->m_Editor);
    delete context;
}

void SetContext(ContextData* context)
{
    ax::NodeEditor::SetCurrentEditor(context->m_Editor);
    s_Session = context;
}

void RegisterNewNode(api::NodeDescription NewDescription) {
    assert(s_Session != nullptr); // You forgot to call CreateContext();
    
    assert(s_Session->NodeRegistry.count(NewDescription.Type) < 1); // you can't register 2 nodes with the same name.
    s_Session->NodeRegistry[NewDescription.Type] = NewDescription;
}


#include <sstream>
void LoadNodesAndLinksFromBuffer(const size_t in_size, const char* buffer)
{
    // do nothing if there is no data
    if(in_size < 1)
        return;


    // Extremely bad deserialization system
    // PHASE ONE - READ FILE TO MEMORY --------------------------------------------
    std::string line; // tracks current line in file read loop

    std::stringstream inf;
    inf << std::string(buffer,in_size);

    int id = 0; // actual node id.  Note that the node id and s_Nodes[x] index are NOT THE SAME.

    std::string NodeName;  // Actually node type    
    std::string Properties; // Whole, intact, Properties table after the loop.
    int PropertiesCount; // Count of properties lines under a node section.

    // First line is config json.
    std::getline(inf, line);
    assert(s_Session != nullptr); // you forgot to call CreateContext();
    s_Session->s_BlueprintData = line;


    // second overall line is node count.
    std::getline(inf,line);

    int node_count = std::stol(line);

    // Processes each node in turn.  Note there are loads
    // of more getline statements inside this loop, so this outer loop ends up iterating on whole node boundaries.
    //while(std::getline(inf,line))
    for (int i = 0; i < node_count; i++)
    {
        std::stringstream PropBuffer; // Accumulator for properties lines in a loop.

        // Shuttle data into ID, NodeName, and Properties variables.

        // first line in the "node sub group" is ID
        std::getline(inf,line);
        id = std::stol(line);
        LogRestoredId(id); // Let the system know this ID is in use, so it doesn't try to use it for new items.

        // Next line is the node type.
        std::getline(inf,line);        
        NodeName = line;

        // next line is the count of pins
        std::getline(inf,line);
        int pin_count = std::stol(line);

        // Read in pin ids to a vector
        std::vector<int> pin_ids;
        if(pin_count > 0) {
            for(int pin_idx = 0; pin_idx < pin_count; pin_idx++)
            {
                std::getline(inf,line);
                int pin_id = std::stol(line);
                LogRestoredId(pin_id); // Let the system know this ID is in use, so it doesn't try to use it for new items.
                pin_ids.push_back(pin_id);
            }
        }

        // Next is the count of properties.
        std::getline(inf,line);
        PropertiesCount = std::stol(line);

        // Iterate over propreties
        for(int i = 0; i < PropertiesCount * 3; i++) {
            std::getline(inf,line);
            // note that we have to re-add the endline because getline consumes it.
            PropBuffer << line <<std::endl;
        }
        // now we have all the propreties!
        Properties = PropBuffer.str();

        // PHASE TWO - INSTANTIATE NODES ------------------------------------------
        // Use data in Nodename and Properties to instantiate nodes from the registry
        // (new node definition system)
        if (s_Session->NodeRegistry.count(NodeName) > 0)
        {
            Node* n = RestoreRegistryNode(NodeName,id,pin_ids);
            // Handle property through deserialization
            Prop_Deserialize(n->Properties, Properties);

        } // Done with node instantiation.
    } // Done with a node processing section.  Loop back if there's another node (more lines in getline)

    // Make pins and node reference reflective.
    BuildNodes();

    // lets read the link count now.
    std::getline(inf,line);
    int link_count = std::stol(line);

    // Iterate over N links
    for (int i = 0; i < link_count; i++)
    {
        // first is our id
        std::getline(inf,line);
        int link_id = std::stol(line);
        LogRestoredId(link_id); // Let the system know this ID is in use, so it doesn't try to use it for new items.

        // next is start pin id
        std::getline(inf,line);
        int start_pin_id = std::stol(line);

        // last is end pin id
        std::getline(inf,line);
        int end_pin_id = std::stol(line);


        // construct a link
        plano::types::Link l = plano::types::Link(link_id,start_pin_id,end_pin_id);
        l.Color = GetIconColor(FindPin(start_pin_id)->Type);

        // attach it to session
        s_Session->s_Links.push_back(std::move(l));
    }
}

#include <sstream>

// Caller owns return value for purposes of memory freeing.  Use delete on the return when you're done. Thank you!
char* SaveNodesAndLinksToBuffer(size_t* size)
{
    assert(s_Session != nullptr); // You forgot to call CreateContext()
    // Extremely bad serilzation system
    // std::ofstream out("nodos_project.txt");
    std::ostringstream out;

    // First line is the config data from the backend.  This data is automatically saved to s_Session.s_BlueprintData
    // using callbacks that were registered to the engine's config strucutre on engine initialization. 
    out << s_Session->s_BlueprintData << std::endl;

    // Second line is the write node count first
    out << s_Session->s_Nodes.size() << std::endl;

    // For every node in s_Nodes...
    for (unsigned long long i = 0; i < s_Session->s_Nodes.size(); i++)
    {
        // First line is ID
        out << s_Session->s_Nodes[i].ID.Get() << std::endl;

        // Next line is node type
        out << s_Session->s_Nodes[i].Name << std::endl;

        // the "count of pins" is next
        int output_pin_count = s_Session->s_Nodes[i].Outputs.size();
        int input_pin_count = s_Session->s_Nodes[i].Inputs.size();
        int pin_count = output_pin_count + input_pin_count;
        out << pin_count << std::endl;

        // dump the input pin ids
        for (int input_idx = 0; input_idx < input_pin_count; input_idx++ )
        {
            out << s_Session->s_Nodes[i].Inputs[input_idx].ID.Get() << std::endl;
        }

        // then dump out the output pin ids
        for (int output_idx = 0; output_idx < output_pin_count; output_idx++ )
        {
            out << s_Session->s_Nodes[i].Outputs[output_idx].ID.Get() << std::endl;
        }

        // The next line is a number describing the count of properties lines.
        unsigned long count;
        std::string props = Prop_Serialize(s_Session->s_Nodes[i].Properties, count);

        out << count << std::endl;

        // Then the next lines are the actual property lines.
        out << props;
    }

    // next write link count
    out << s_Session->s_Links.size() << std::endl;

    // For every link in s_Links...
    for (unsigned long long i = 0; i < s_Session->s_Links.size(); i++)
    {
        // First line is ID
        out << s_Session->s_Links[i].ID.Get() << std::endl;

        // next is start pin id
        out << s_Session->s_Links[i].StartPinID.Get() << std::endl;
        // next is end pin id
        out << s_Session->s_Links[i].EndPinID.Get() << std::endl;
    }

    *size = out.str().size();
    char* out_buf = new char[*size];
    memcpy(out_buf,out.str().c_str(),*size);
    return out_buf;
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


void Initialize(void)
{

    // NODOS DEV ===================================================
    // Register nodes from the user's node description forms.    
    //RegisterNewNode(node_defs::import_animal::ConstructDefinition());

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


    //auto& io = ImGui::GetIO();
}

void Finalize(void)
{
    s_Session->DestroyTexture(s_Session->s_RestoreIcon);
    s_Session->DestroyTexture(s_Session->s_SaveIcon);
    s_Session->DestroyTexture(s_Session->s_HeaderBackground);
    s_Session->s_RestoreIcon = nullptr;
    s_Session->s_SaveIcon = nullptr;
    s_Session->s_HeaderBackground = nullptr;

    if (s_Session->m_Editor)
    {
        ed::DestroyEditor(s_Session->m_Editor);
        s_Session->m_Editor = nullptr;
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

} // inner namespace
} // outer namespace
