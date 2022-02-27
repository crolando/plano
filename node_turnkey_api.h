#ifndef NODE_TURNKEY_API_H
#define NODE_TURNKEY_API_H

# include <node_turnkey_types.h>

namespace turnkey {
namespace api {

// NodeIDLess is a custom comparitor function for the s_NodeTouchTime map.
// TODO: move this.
struct NodeIdLess
{
    bool operator()(const ax::NodeEditor::NodeId& lhs, const ax::NodeEditor::NodeId& rhs) const
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
    // s_Nodes is the list of instantiated nodes in the running session
    std::vector<types::Node>    s_Nodes;

    // The Node Registry stores the prototypes for nodes.
    std::map<std::string, NodeDescription> NodeRegistry;

    // Use this to register a new node to the system
    void RegisterNewNode(NodeDescription NewDescription) {
        NodeRegistry[NewDescription.Type] = NewDescription;
    }

    // This is an internal function that is called when a save file is deserialized.
    // Used in Deserialization (loading material from a save file) routines.
    // Very similar to NewRegistryNode but with different ID
    // todo: make private
    void RestoreRegistryNode(const std::string& NodeName,const std::string* Properties, int id);

    // Internal function that
    // todo: make private
    types::Node* NewRegistryNode(const std::string& NodeName);

    // The session needs to keep track of what the next allowed node ID is.
    // These are mostly private.
    // todo: make private, I think.
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
    // todo: make private
    void BuildNode(types::Node* node)
    {
        for (auto& input : node->Inputs)
        {
            input.Node = node;
            input.Kind = ax::NodeEditor::PinKind::Input;
        }

        for (auto& output : node->Outputs)
        {
            output.Node = node;
            output.Kind = ax::NodeEditor::PinKind::Output;
        }
    }
};



} // end api namespace
} // end turnkey namespace
#endif // NODE_TURNKEY_API_H
