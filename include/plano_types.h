#ifndef NODE_TURNKEY_TYPES_H
#define NODE_TURNKEY_TYPES_H

// Types Section ==============================================================
#include <internal/widgets.h>
#include <internal/builders.h>
#include <string>
#include <vector>
#include <map>

// The user needs to specify a type that stores all the node instance UI data.
// You will use this type in your node drawing routines to track strings and numbers
// in the widgets.
//
// In order to support saving and loading these strings and numbers to a "project file"
// and because this is so tightly coupled withe data representation, you must also expose
// a serailze and deserialize routine.
//
// Feel free to add your own container, or a void* and recompile.
// We give you this "slow but easy to use" generic container here:
#include <internal/attribute.h>
typedef attr_table Properties;


namespace turnkey {
namespace types {

// Context for the turnkey system.
// This is a forward declaration.
// This is fully declared in node_turnkey_internal.h
// and then implemented in node_turnkey_internal.cpp
struct SessionData;

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
    Properties  Properties;
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


}; // END OF NAMESPACE TYPES

} // end of namespace turnkey

#endif // NODE_TURNKEY_TYPES_H
