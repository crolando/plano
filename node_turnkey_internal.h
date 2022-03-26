#ifndef NODE_TURNKEY_INTERNAL_H
#define NODE_TURNKEY_INTERNAL_H
#include <node_turnkey_api.h>
#include <texture_manager.h>

namespace turnkey {

namespace types {
struct SessionData {
                                                 // note: nodes carry the pins
    std::vector<types::Node>       s_Nodes;      // s_Nodes is the list of instantiated nodes in the running session
    std::vector<types::Link>       s_Links;      // s_Links is the list of instantiated links in the running session

    std::map<std::string,
             api::NodeDescription> NodeRegistry; // The Node Registry stores the prototypes.

                   texture_manager textures;     // Textures "own" the textures used.
                              int  s_NextId = 1; // The session needs to keep track of what the next unclaimed ID for nodes, pins & links.


                               int s_PinIconSize = 24;
                       std::string s_BlueprintData;
    ax::NodeEditor::EditorContext* m_Editor = nullptr;
                       ImTextureID s_HeaderBackground = nullptr;
                       ImTextureID s_SampleImage = nullptr;
                       ImTextureID s_SaveIcon = nullptr;
                       ImTextureID s_RestoreIcon = nullptr;
};
} // end of turnkey::types namespace.

namespace internal {


extern types::SessionData s_Session;      // ultimate, global, current session.  managed by public api calls.



// internal tools all operate on current session -------------------------------------
int          GetNextId();            // Get an unclaimed ID for runtime ID tracking
void         SetNextId(int Id);      // Deserializer will need to bump up the ID after it fills the SessionData with used IDs.
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
        void RestoreRegistryNode(const std::string& NodeName,const std::string* Properties, int id, const std::vector<int>& pin_ids);

        // Create a fresh node at runtime, not through deserialization.
        types::Node* NewRegistryNode(const std::string& NodeName);



/*
// i/o functions with backend (imgui-node-editor) that generally facilitate serialization of its
// data.  Note this uses a new technique that i could have learned when I was 20, had I gone to a cs college!
// this technique is explained here:
// https://stackoverflow.com/questions/19808054/convert-c-function-pointer-to-c-function-pointer/19808250#19808250
bool non_static_config_save_settings(const char* data, size_t size, ax::NodeEditor::SaveReasonFlags reason)
{
    s_Session.s_BlueprintData.reserve(size); //maybe not needed
    s_Session.s_BlueprintData.assign(data);
    std::ofstream out("project.txt");
    out << s_Session.s_BlueprintData;
    return true;
};

static bool static_config_save_settings(const char* data, size_t size, ax::NodeEditor::SaveReasonFlags reason, void* userPointer)
{
    SessionData* obj = (SessionData*) userPointer;
    return obj->non_static_config_save_settings(data,size,reason);
};

size_t non_static_config_load_settings(char* data)
{
    std::ifstream in("project.txt");
    std::stringstream b;
    b << in.rdbuf();
    s_BlueprintData = b.str();

    size_t size = s_BlueprintData.size();
    if(data) {
        memcpy(data,s_BlueprintData.c_str(),size);
    }
    return size;
};

static size_t static_config_load_settings(char* data, void* userPointer)
{
    SessionData* obj = (SessionData*) userPointer;
    return obj->non_static_config_load_settings(data);
};


size_t non_static_config_load_node_settings(ax::NodeEditor::NodeId nodeId, char* data)
{
    auto node = FindNode(nodeId);
    if (!node)
        return 0;

    if (data != nullptr)
        memcpy(data, node->State.data(), node->State.size());
    return node->State.size();
};

static size_t static_config_load_node_settings(ax::NodeEditor::NodeId nodeId, char* data, void* userPointer)
{
    SessionData* obj = (SessionData*) userPointer;
    return obj->non_static_config_load_node_settings(nodeId,data);
};


bool non_static_config_save_node_settings(ax::NodeEditor::NodeId nodeId, const char* data, size_t size, ax::NodeEditor::SaveReasonFlags reason)
{
    auto node = FindNode(nodeId);
    if (!node)
        return false;

    node->State.assign(data, size);

    return true;
};

static bool static_config_save_node_settings(ax::NodeEditor::NodeId nodeId, const char* data, size_t size, ax::NodeEditor::SaveReasonFlags reason, void* userPointer)
{
    SessionData* obj = (SessionData*) userPointer;
    return obj->non_static_config_save_node_settings(nodeId,data,size,reason);
};
*/

} // inner namespace
} // outer namespace
#endif // NODE_TURNKEY_INTERNAL_H
