#ifndef NODE_TURNKEY_API_H
#define NODE_TURNKEY_API_H

# include <node_turnkey_types.h>
# include <texture_manager.h>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

namespace turnkey {
namespace api {

// In order to support saving and loading these strings and numbers to a "project file"
// and because this is so tightly coupled withe data representation, you must expose
// a serailze and deserialize routine.
//
// We give you this "slow but easy to use" generic container here:
// Note these are implemented in attribute.cpp
std::string Prop_Serialize (const Properties& Prop_In, int& entries); // entries is the count of properties TODO: don't have entires
void        Prop_Deserialize(Properties& Prop_In, const std::string& serialized_table);

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
    // void function_name(Properties&);  (where properties are defined in node_turnkey_types.h)
    // We will call it when a new node is created.
    // In your function, please add default key value pairs
    // into the attribute table.
    void (*InitializeDefaultProperties)(Properties&);

    // Please provide a function of type
    // void function_name(Properties&); (where properties are defined in node_turnkey_types.h)
    // This is your IMGUI draw callback.  Your job is to read and
    // write properties values using IMGUI widgets.
    void (*DrawAndEditProperties)(Properties&);

    // (For offline-update pattern) The system would like you to "execute" your node
    void (*Execute)(Properties&,const std::vector<turnkey::types::Link>& Inputs, const std::vector<turnkey::types::Link>& Outputs);

    // (For offline-update pattern) The system wants you to kill the execution of your node.
    void (*KillExecution)(void);
};

// Context management.
// These calls set a global context variable, under which other API calls operate on.
types::SessionData* CreateContext();
void                DestroyContext(types::SessionData*);
types::SessionData* GetContext();
void                SetContext(types::SessionData* context);

// ~ Node Handling ~
void RegisterNewNode(NodeDescription NewDescription); // register your NodeDescriptions here to make the runtime aware of your node type.

// Overall System Start / Frame / Stop calls
// TODO: Init/Finailze should amost certainly be ported to the above context systems.
void Initialize(void); // Creates sets up and configures imgui_node_editor backend, deserializes project file.
void Frame(void);      // Draws nodes and handles interactions.
void Finalize(void);   // Cleanup.

// // Destroy Node (Node*)
// NodePrototypeID = RegisterNode (NodePrototype prototype)
// NodeInstanceID = Create Node (NodePrototypeID type);

// LinkInstanceID[] = GetLinkInstances()
// NodeInstanceID[] = GetNodeInstances()


// ~ Serialization ~
// LoadNodesAndLinksFromBuffer(void*)
// SaveNodesAndLinksToBuffer(out_size, void* buffer);



} // end api namespace
} // end turnkey namespace
#endif // NODE_TURNKEY_API_H
