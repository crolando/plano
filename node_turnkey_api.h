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


// NodeIDLess is a custom comparitor function for the s_NodeTouchTime map.
struct NodeIdLess
{
    bool operator()(const ax::NodeEditor::NodeId& lhs, const ax::NodeEditor::NodeId& rhs) const
    {
        return lhs.AsPointer() < rhs.AsPointer();
    }
};



struct nodos_session_data {

    // Call before first frame
    void Initialize(void);

    // Call per frame
    void Frame(void);

    // Destroy
    void Finalize(void);

    // s_Nodes is the list of instantiated nodes in the running session
    std::vector<types::Node>    s_Nodes;

    // s_Links is the list of instantiated nodes in the running session
    std::vector<types::Link>    s_Links;

    // The Node Registry stores the prototypes for nodes.
    std::map<std::string, NodeDescription> NodeRegistry;

    //
    texture_manager textures;

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
    // internal utilities
    ax::NodeEditor::NodeId GetNextLinkId();
    void TouchNode(ax::NodeEditor::NodeId id);
    float GetTouchProgress(ax::NodeEditor::NodeId id);
    types::Node* FindNode(ax::NodeEditor::NodeId id);
    turnkey::types::Link* FindLink(ax::NodeEditor::LinkId id);
    turnkey::types::Pin* FindPin(ax::NodeEditor::PinId id);
    bool IsPinLinked(ax::NodeEditor::PinId id);
    bool isNodeAncestor(types::Node* Ancestor, types::Node* Decendent);
    void DrawPinIcon(const types::Pin& pin, bool connected, int alpha);


    std::map<ax::NodeEditor::NodeId, float, NodeIdLess> s_NodeTouchTime;
    void UpdateTouch()
    {
        const auto deltaTime = ImGui::GetIO().DeltaTime;
        for (auto& entry : s_NodeTouchTime)
        {
            if (entry.second > 0.0f)
                entry.second -= deltaTime;
        }
    }

    // i/o functions with backend (imgui-node-editor) that generally facilitate serialization of its
    // data.  Note this uses a new technique that i could have learned when I was 20, had I gone to a cs college!
    // this technique is explained here:
    // https://stackoverflow.com/questions/19808054/convert-c-function-pointer-to-c-function-pointer/19808250#19808250
    bool non_static_config_save_settings(const char* data, size_t size, ax::NodeEditor::SaveReasonFlags reason)
    {
        s_BlueprintData.reserve(size); //maybe not needed
        s_BlueprintData.assign(data);
        std::ofstream out("project.txt");
        out << s_BlueprintData;
        return true;
    };
    static bool static_config_save_settings(const char* data, size_t size, ax::NodeEditor::SaveReasonFlags reason, void* userPointer)
    {
        nodos_session_data* obj = (nodos_session_data*) userPointer;
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
        nodos_session_data* obj = (nodos_session_data*) userPointer;
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
        nodos_session_data* obj = (nodos_session_data*) userPointer;
        return obj->non_static_config_load_node_settings(nodeId,data);
    };


    bool non_static_config_save_node_settings(ax::NodeEditor::NodeId nodeId, const char* data, size_t size, ax::NodeEditor::SaveReasonFlags reason)
    {
        auto node = FindNode(nodeId);
        if (!node)
            return false;

        node->State.assign(data, size);

        TouchNode(nodeId);

        return true;
    };

    static bool static_config_save_node_settings(ax::NodeEditor::NodeId nodeId, const char* data, size_t size, ax::NodeEditor::SaveReasonFlags reason, void* userPointer)
    {
        nodos_session_data* obj = (nodos_session_data*) userPointer;
        return obj->non_static_config_save_node_settings(nodeId,data,size,reason);
    };






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

    void BuildNodes(void);


    const int            s_PinIconSize = 24;

    std::string          s_BlueprintData;
    ax::NodeEditor::EditorContext* m_Editor = nullptr;
    ImTextureID          s_HeaderBackground = nullptr;
    ImTextureID          s_SampleImage = nullptr;
    ImTextureID          s_SaveIcon = nullptr;
    ImTextureID          s_RestoreIcon = nullptr;
    const float          s_TouchTime = 1.0f;
};



} // end api namespace
} // end turnkey namespace
#endif // NODE_TURNKEY_API_H
