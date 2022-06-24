#ifndef NODE_TURNKEY_API_H
#define NODE_TURNKEY_API_H

# include <node_turnkey_types.h>


namespace turnkey {
namespace api {
// Texture functions. ================================================================= 
// the burden is on YOU to implement these because they're platform dependent, and this
// library is platform independent.  
 
// LoadTexture should read a png file address, upload the image to the platform,
// then return a texture ID.
ImTextureID Application_LoadTexture(const char* path);

// DestroyTexture should purge all memory used to store the texture built in LoadTexture
void Application_DestroyTexture(ImTextureID);


// GetTextureWidth returns the height of the texture in pixels
unsigned int Application_GetTextureWidth(ImTextureID);

// GetTextureHeight returns the height of the texture in pixels
unsigned int Application_GetTextureHeight(ImTextureID);

// Serialize & Deserialze Properties Callbacks =========================================
//
// In order to support saving and loading these strings and numbers to a "project file"
// and because this is so tightly coupled withe data representation, you must expose
// a serailze and deserialize routine.
//
// We give you this "slow but easy to use" generic container here:
// Note these are implemented in attribute.cpp
std::string Prop_Serialize (const Properties& Prop_In);
void        Prop_Deserialize(Properties& Prop_In, const std::string& serialized_table);

// Serialze & Deserialize EVERYTHING Functions =========================================
//
// handles: node types, positions, pin connections, properties, the whole shebang.
//
// All the other serizliation stuff, like node positions, what node types are in the graph,
// where the links are connected to, etc etc etc are handled automatically.  These calls
// end up calling the above ones.  These are your master load/save IO for turnkey.
// write this shit to a file, and then load it and then back you are.





void LoadNodesAndLinksFromBuffer(const size_t in_size,  const char *buffer);

// SaveNodesAndLinksToBuffer()
// Move all the graph's data to a buffer.  Put this data into your "project" files!
//
// This returns a heap address that YOU are responsible for clearing.
// You MUST use "delete"; you cannot use "free".
//
// This function is an example of a "multiple return value" pattern. It writes to
// the arguement, and includes a return value.  Below is an example.
//
// useage example:
//    size_t size;
//    char* cbuffer  = turnkey::api::SaveNodesAndLinksToBuffer(&size);
//    // Save "size" count characters from "cbuffer" to a file.
//    delete cbuffer;
//
char* SaveNodesAndLinksToBuffer(size_t* size);


struct PinDescription {
    std::string Label;
    std::string DataType;

    // Basic init constuctor
    PinDescription():
        Label(""),
        DataType(""){};

    // Concise constructor
    PinDescription(std::string Label,std::string DataType):
        Label(Label),
        DataType(DataType){};
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
void Initialize(void); // Creates sets up and configures imgui_node_editor backend
void Frame(void);      // Draws nodes and handles interactions.
void Finalize(void);   // Cleanup.


// // Destroy Node (Node*)
// NodePrototypeID = RegisterNode (NodePrototype prototype)
// NodeInstanceID = Create Node (NodePrototypeID type);

// LinkInstanceID[] = GetLinkInstances()
// NodeInstanceID[] = GetNodeInstances()



} // end api namespace
} // end turnkey namespace
#endif // NODE_TURNKEY_API_H
