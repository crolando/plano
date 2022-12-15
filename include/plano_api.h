#ifndef NODE_PLANO_API_H
#define NODE_PLANO_API_H
/* 
* Plano - A Plug-and-play Imgui Node Editor
*/
# include <plano_properties.h>
# include <plano_types.h>

namespace plano {
namespace types {
    struct ContextCallbacks // Platform dependent, user-defined function pointer callback variables. See Nodos project for sample.
    {
        ImTextureID  (*LoadTexture)(const char* path);  // Take a PNG and upload it to the graphics card. Return an ID.
        void         (*DestroyTexture)(ImTextureID);    // Take an ID and free the memory
        unsigned int (*GetTextureWidth)(ImTextureID);   // Take and ID and report the width in pixels
        unsigned int (*GetTextureHeight)(ImTextureID);  // Take and ID and report the height in pixels
    };
}

namespace api {    
    struct NodeDescription; // Forward declaration.
    struct PinDescription; // Forward declaration.

    // Plano Context Management 
    // These calls manipulate the global context variable, on which the other API calls operate on.
    types::ContextData* CreateContext(const types::ContextCallbacks& Config, const char *texture_path );
    const types::ContextData* GetContext();
    void                SetContext(types::ContextData* context);
    void                DestroyContext(types::ContextData*);

    // Drawing Subsystem Routines 
    void Frame(void);      // Draws nodes and handles interactions. Call in your draw loop.
    void Finalize(void);   // Destroy textures

    // Node Prototype Registration     
    void RegisterNewNode(NodeDescription NewDescription);    // Call this to make the system aware of a node type. Called once per node type.



    // Project Save and Load functions
    char* SaveNodesAndLinksToBuffer(size_t* size);           // Serialize the graph to a char*.  Writes length to "size". You must manually free the return value with delete.
    void  LoadNodesAndLinksFromBuffer(const size_t in_size,  const char *buffer);  // Opposite of above.  

    // Node Description Struct
    // Think of this as an application you fill out that describes a node type.  Register with RegisterNewNode().
    struct NodeDescription {
        std::string Type;                       // The name of the node. Used to spawn the node.
        std::vector<PinDescription> Inputs;     // Input pin descriptions.  Element 0 is at the top left of the node 
        std::vector<PinDescription> Outputs;    // Output pin descriptions. Element 0 is at the top right of the node
        ImColor Color = ImColor(255, 255, 255); // The style color added at the top of the node
        
        // NodeDescrption Function Pointers
        // You must implement these per node to define widget behavior and values.  See Nodos project for examples.
        void (*InitializeDefaultProperties)(Properties&); // Set default values for node widget values. Called when constructing a fresh node at runtime (not deserialization).        
        void (*DrawAndEditProperties)(Properties&);       // Called when it is time to draw the node's widgets.  Restore, edit and save the widget values in the properties table.
    };

    // Pin Description Struct
    struct PinDescription {
        std::string Label;               // Words printed next to the pin icon
        plano::types::PinType DataType;  // Controls the icon, icon color, and connection enforcement (eg you can't connect a string to a float).

        // Constructor
        PinDescription(std::string Label, plano::types::PinType DataType):
            Label(Label),
            DataType(DataType){};
    };

} // end api namespace
} // end plano namespace
#endif // PLANO_API_H
