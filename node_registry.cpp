#include <node_turnkey_internal.h> // This brings in types and api as well.
                                   // also critically brings in s_Session, which these functions manipulate.
using namespace turnkey::types;
using namespace turnkey::api;

namespace turnkey {
namespace internal {

// This spawns a fresh node using the node definitions loaded into the registry.
// The properties of the node are intialized with defaults from the definition.
Node* NewRegistryNode(const std::string& NodeName) {

    // Standard node spawner behavior, only we construct the objects
    // using the registry data.
    // NodeRegistry is a map, so we need the value.
    NodeDescription Desc = s_Session.NodeRegistry[NodeName];

    // Create node object and pass the type name.
    s_Session.s_Nodes.emplace_back(GetNextId(), Desc.Type.c_str());

    // Handle creating the pins
    for(PinDescription p : Desc.Inputs)
        s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), p.Label.c_str(), PinType::Flow);
    for(PinDescription p : Desc.Outputs)
        s_Session.s_Nodes.back().Outputs.emplace_back(GetNextId(), p.Label.c_str(), PinType::Flow);

    Desc.InitializeDefaultProperties(s_Session.s_Nodes.back().Properties);

    // Standard scrubber from examples.
    BuildNode(&s_Session.s_Nodes.back());

    // "return" value from example spawner
    return &s_Session.s_Nodes.back();
}

// This restores a node using the node definitions loaded into the registry.
// The properties of the node are copied in via deserialization of the project file.
void RestoreRegistryNode(const std::string& NodeName,const std::string* Properties, int id)
{
    // Standard node spawner behavior, only we construct the objects
    // using the registry data.
    // NodeRegistry is a map, so we need the value.
    NodeDescription Desc = s_Session.NodeRegistry[NodeName];

    // Create node object and pass the type name.
    s_Session.s_Nodes.emplace_back(id, Desc.Type.c_str());

    // Because we didn't call GetNextID, we must manually put the ID counter where it
    // would be, had we done so.
    SetNextId(id + 1);

    // Handle creating the pins
    for(PinDescription p : Desc.Inputs)
        s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), p.Label.c_str(), PinType::Flow);
    for(PinDescription p : Desc.Outputs)
        s_Session.s_Nodes.back().Outputs.emplace_back(GetNextId(), p.Label.c_str(), PinType::Flow);

    // Handle propertie through deserialization
    s_Session.s_Nodes.back().Properties.deseralize(*Properties);

    // Standard scrubber from examples.
    BuildNode(&s_Session.s_Nodes.back());
}
} // inner namespace
} // outer namespace
