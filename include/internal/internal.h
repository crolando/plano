#ifndef PLANO_INTERNAL_H
#define PLANO_INTERNAL_H
#include <plano_api.h>


namespace plano {
namespace internal {

extern types::ContextData *s_Session;      // ultimate, global, current session.  managed by public api calls.
static int context_id = 0; // Tracks unique ImGUI ids for contexts.  Used to seed the ContextData->beginID field.

// i/o functions with backend (imgui-node-editor) that generally facilitate serialization of its data.
bool static_config_save_settings(const char* data, size_t size, ax::NodeEditor::SaveReasonFlags reason, void* userPointer);
size_t static_config_load_settings(char* data, void* userPointer);
size_t static_config_load_node_settings(ax::NodeEditor::NodeId nodeId, char* data, void* userPointer);
bool static_config_save_node_settings(ax::NodeEditor::NodeId nodeId, const char* data, size_t size, ax::NodeEditor::SaveReasonFlags reason, void* userPointer);

} // internal 1 is done
// still in plano namesapce
namespace types {
struct ContextData {
                                                 // note: nodes carry the pins
    std::vector<types::Node>       s_Nodes;      // s_Nodes is the list of instantiated nodes in the running session
    std::vector<types::Link>       s_Links;      // s_Links is the list of instantiated links in the running session

    std::map<std::string,
             api::NodeDescription> NodeRegistry; // The Node Registry stores the prototypes.

         //std::vector<ImTextureID>  textures;     // Textures "own" the textures used.
                               int s_NextId = 1; // The session needs to keep track of what the next unclaimed ID for nodes, pins & links.


                               int s_PinIconSize = 24;
                       std::string s_BlueprintData;
    ax::NodeEditor::EditorContext* m_Editor = nullptr;
                       std::string beginID = "Editor";
                       ImTextureID s_HeaderBackground = nullptr;
                       ImTextureID s_SampleImage = nullptr;
                       ImTextureID s_SaveIcon = nullptr;
                       ImTextureID s_RestoreIcon = nullptr;
    
    
                      // Callbacks
                      ImTextureID  (*LoadTexture)(const char* path);  // Take a PNG and upload it to the graphics card. Return an ID.
                      void         (*DestroyTexture)(ImTextureID);    // Take an ID and free the memory
                      unsigned int (*GetTextureWidth)(ImTextureID);   // Take and ID and report the width in pixels
                      unsigned int (*GetTextureHeight)(ImTextureID);  // Take and ID and report the height in pixels
    
    const char* TexturePath;
                              bool IsProjectDirty;
                              bool m_ShowOrdinals;

    // Constructor
    ContextData(ContextCallbacks Callbacks, const char *texture_path):
        LoadTexture(Callbacks.LoadTexture),
        DestroyTexture(Callbacks.DestroyTexture),
        GetTextureWidth(Callbacks.GetTextureWidth),
    GetTextureHeight(Callbacks.GetTextureHeight),
    TexturePath(texture_path)
    {
        auto file = std::string(TexturePath);
        file.append("BlueprintBackground.png");
        s_HeaderBackground = LoadTexture(file.c_str());
        
        // Config structure holds callsbacks for backend-frontend serialization transactions.
        ax::NodeEditor::Config config;

        // https://stackoverflow.com/questions/19808054/convert-c-function-pointer-to-c-function-pointer/19808250#19808250
        //config.UserPointer = (void*) this;
        config.SaveSettings = plano::internal::static_config_save_settings;
        config.LoadSettings = plano::internal::static_config_load_settings;
        config.LoadNodeSettings = plano::internal::static_config_load_node_settings;
        config.SaveNodeSettings = plano::internal::static_config_save_node_settings;
        m_Editor = ax::NodeEditor::CreateEditor(&config);
        
        beginID += std::to_string(internal::context_id++);
        IsProjectDirty = false;
        m_ShowOrdinals = true;
    };
    // destructor
    ContextData()
    {
        ax::NodeEditor::DestroyEditor(m_Editor);
        DestroyTexture(s_HeaderBackground);
    };
};
} // end of plano::types namespace.

namespace internal {





// internal tools all operate on current session -------------------------------------
int          GetNextId();            // Get an unclaimed ID for runtime ID tracking
void         SetNextId(int Id);      // Deserializer will need to bump up the ID after it fills the ContextData with used IDs.
void         LogRestoredId(int Id);  // Probably a better way to do above, sets next id to 1 + max(id,input).

types::Node* FindNode(ax::NodeEditor::NodeId id);    // Convert a NodeId to a Node*
types::Link* FindLink(ax::NodeEditor::LinkId id);    // Convert a LinkId to a Link*
types::Pin*  FindPin(ax::NodeEditor::PinId id);      // Convert a PinId to a Pin*

        bool IsPinLinked(ax::NodeEditor::PinId id);  //
        bool isNodeAncestor(types::Node* Ancestor, types::Node* Decendent); // traversal tool

        // Draw and Construct tools.  Can we move these?
        void DrawPinIcon(const types::Pin& pin, bool connected, int alpha);
        void BuildNode(types::Node* node);
        void BuildNodes(void);




        // This is an internal function that is called when a save file is deserialized.
        // Used in Deserialization (loading material from a save file) routines.
        // Very similar to NewRegistryNode but with different ID
        // todo: make private
        types::Node* RestoreRegistryNode(const std::string& NodeName, int id, const std::vector<int>& pin_ids);

        // Create a fresh node at runtime, not through deserialization.
        types::Node* NewRegistryNode(const std::string& NodeName);







} // inner namespace
} // outer namespace

#endif // NODE_TURNKEY_INTERNAL_H
