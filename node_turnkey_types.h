#ifndef NODE_TURNKEY_TYPES_H
#define NODE_TURNKEY_TYPES_H

// Types Section ==============================================================
#include <widgets.h>
#include <builders.h>
#include <attribute.h>
#include <string>
#include <vector>
#include <map>

namespace ed = ax::NodeEditor;
namespace util = ax::NodeEditor::Utilities;

namespace turnkey {
namespace types {

enum class PinType
{
    Flow,
    Bool,
    Int,
    Float,
    String,
    Object,
    Function,
    Delegate,
};


enum class NodeType
{
    Blueprint,
    Simple,
    Tree,
    Comment,
    Houdini
};

struct Node;

struct Pin
{
    ed::PinId   ID;
    Node*     Node;
    std::string Name;
    PinType     Type;
    ed::PinKind     Kind;

    Pin(int id, const char* name, PinType type):
        ID(id), Node(nullptr), Name(name), Type(type), Kind(ed::PinKind::Input)
    {
    }
};

struct Node
{
    ed::NodeId ID;
    std::string Name;
    std::vector<Pin> Inputs;
    std::vector<Pin> Outputs;
    attr_table  Properties;
    ImColor Color;
    NodeType Type;
    ImVec2 Size;

    std::string State;      // State is buffer to store the backend's node specific data between frames, basically.  It mostly stores the node's position.
    std::string SavedState; // SavedState is only used in the leftpanel, so we can delete it if you want.

    Node(int id, const char* name, ImColor color = ImColor(255, 255, 255)):
        ID(id), Name(name), Color(color), Type(NodeType::Blueprint), Size(0, 0)
    {
    }
};

struct Link
{
    ed::LinkId ID;

    ed::PinId StartPinID;
    ed::PinId EndPinID;

    ImColor Color;

    Link(ed::LinkId id, ed::PinId startPinId, ed::PinId endPinId):
        ID(id), StartPinID(startPinId), EndPinID(endPinId), Color(255, 255, 255)
    {
    }
};

}; // END OF NAMESPACE TYPES

namespace api {

// NodeIDLess is a custom comparitor function for the s_NodeTouchTime map.
// TODO: move this.
struct NodeIdLess
{
    bool operator()(const ed::NodeId& lhs, const ed::NodeId& rhs) const
    {
        return lhs.AsPointer() < rhs.AsPointer();
    }
};


struct PinDescription {
    std::string Label;
    std::string DataType;
};

// Please fill out this form and then register it.
struct NodeDescription {
    std::string Type;
    std::vector<PinDescription> Inputs;
    std::vector<PinDescription> Outputs;

    // In this section we ask you give us function pointers.
    // Please define functions of the type required, then
    // add them to the forms.  These will be called
    // when the graph or user needs your node to do things.

    // Please provide a function of type
    // void function_name(attr_table&);
    // We will call it when a new node is created.
    // In your function, please add default key value pairs
    // into the attribute table.
    void (*InitializeDefaultProperties)(attr_table&);

    // Please provide a function of type
    // void function_name(attr_table&);
    // This is your IMGUI draw callback.  Your job is to read and
    // write properties values using IMGUI widgets.
    void (*DrawAndEditProperties)(attr_table&);

    // (For offline-update pattern) The system would like you to "execute" your node
    void (*Execute)(attr_table&,const std::vector<types::Link>& Inputs, const std::vector<types::Link>& Outputs);

    // (For offline-update pattern) The system wants you to kill the execution of your node.
    void (*KillExecution)(void);
};

struct nodos_session_data {

    std::vector<types::Node>    s_Nodes;   // The index is NOT the ID!
    std::map<std::string, NodeDescription> NodeRegistry;

    // The session needs to keep track of what the next allowed node ID is.
    // These are mostly private.
    int s_NextId = 1;
    int GetNextId() {
        return s_NextId++;
    }
    void SetNextId(int Id) {
        s_NextId = Id;
    }

    // BuildNode exists because during Spawn**Node(), the Pins are not fully
    // constructed since the information is reflective.
    // The missing information is specifically:
    // 1. What node contains me (pin.Node)
    // 2. What pin vector contains me (pin.Kind) - this is normally input or output.
    void BuildNode(types::Node* node)
    {
        for (auto& input : node->Inputs)
        {
            input.Node = node;
            input.Kind = ed::PinKind::Input;
        }

        for (auto& output : node->Outputs)
        {
            output.Node = node;
            output.Kind = ed::PinKind::Output;
        }
    }



    void RegisterNewNode(NodeDescription NewDescription) {
        NodeRegistry[NewDescription.Type] = NewDescription;
    }

    // Used in Deserialization (loading material from a save file) routines.
    // Very similar to NewRegistryNode but with different ID
    void RestoreRegistryNode(const std::string& NodeName,const std::string* Properties, int id);

    types::Node* NewRegistryNode(const std::string& NodeName);

};



}
}

#endif // NODE_TURNKEY_TYPES_H
