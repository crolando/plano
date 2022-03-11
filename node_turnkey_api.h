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

struct SessionData; //actually defined in node_turnkey_internal.h

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
    void (*Execute)(attr_table&,const std::vector<turnkey::types::Link>& Inputs, const std::vector<turnkey::types::Link>& Outputs);

    // (For offline-update pattern) The system wants you to kill the execution of your node.
    void (*KillExecution)(void);
};

// Context management.
types::SessionData* CreateContext();
void                DestroyContext(types::SessionData*);
types::SessionData* GetContext();
void                SetContext(types::SessionData* context);

// In which these calls set a global context variable, under which other API calls operate under:


// ~ Node Handling ~


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
