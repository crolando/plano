#ifndef NODE_TURNKEY_TYPES_H
#define NODE_TURNKEY_TYPES_H

// Types Section ==============================================================
#include <widgets.h>
#include <builders.h>
#include <attribute.h>
#include <string>
#include <vector>
#include <map>
#include <node_turnkey_internal.h>

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
    ax::NodeEditor::PinId   ID;
    Node*     Node;
    std::string Name;
    PinType     Type;
    ax::NodeEditor::PinKind     Kind;

    Pin(int id, const char* name, PinType type):
        ID(id), Node(nullptr), Name(name), Type(type), Kind(ax::NodeEditor::PinKind::Input)
    {
    }
};

struct Node
{
    ax::NodeEditor::NodeId ID;
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
    ax::NodeEditor::LinkId ID;

    ax::NodeEditor::PinId StartPinID;
    ax::NodeEditor::PinId EndPinID;

    ImColor Color;

    Link(ax::NodeEditor::LinkId id, ax::NodeEditor::PinId startPinId, ax::NodeEditor::PinId endPinId):
        ID(id), StartPinID(startPinId), EndPinID(endPinId), Color(255, 255, 255)
    {
    }
};

// Treat this as opaque.
typedef turnkey::internal::SessionData Context;


}; // END OF NAMESPACE TYPES

} // end of namespace turnkey

#endif // NODE_TURNKEY_TYPES_H
