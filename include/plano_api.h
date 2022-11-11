#ifndef NODE_PLANO_API_H
#define NODE_PLANO_API_H
/* 
* Plano - A Plug-and-play Imgui Node Editor
*/
# include <plano_properties.h>
# include <plano_types.h>

namespace plano {
namespace api {    
    struct NodeDescription; // Forward declaration.
    struct PinDescription; // Forward declaration.

    // Plano Context Management 
    // These calls manipulate the global context variable, on which the other API calls operate on.
    types::SessionData* CreateContext();  // Typically used with: plano::api::SetContext(plano::api::CreateContext());
    types::SessionData* GetContext();
    void                SetContext(types::SessionData* context);
    void                DestroyContext(types::SessionData*);

    // Drawing Subsystem Routines 
    void Initialize(void); // Upload textures needed by the node system
    void Frame(void);      // Draws nodes and handles interactions. Call in your draw loop.
    void Finalize(void);   // Destroy textures

    // Node Prototype Registration     
    void RegisterNewNode(NodeDescription NewDescription);    // Call this to make the system aware of a node type. Called once per node type.

    // Texture Callbacks that you have to implement 
    // For an example, see the Nodos project.      
    ImTextureID  Application_LoadTexture(const char* path);  // Take a PNG and upload it to the graphics card. Return an ID.        
    void         Application_DestroyTexture(ImTextureID);    // Take an ID and free the memory         
    unsigned int Application_GetTextureWidth(ImTextureID);   // Take and ID and report the width in pixels    
    unsigned int Application_GetTextureHeight(ImTextureID);  // Take and ID and report the height in pixels    

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
