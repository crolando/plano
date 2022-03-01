#include <node_turnkey_api.h>

using namespace turnkey::api;
using namespace turnkey::types;


Node* nodos_session_data::NewRegistryNode(const std::string& NodeName) {

    // Standard node spawner behavior, only we construct the objects
    // using the registry data.
    // NodeRegistry is a map, so we need the value.
    NodeDescription Desc = NodeRegistry[NodeName];

    // Create node object and pass the type name.
    s_Nodes.emplace_back(GetNextId(), Desc.Type.c_str());

    // Handle creating the pins
    for(PinDescription p : Desc.Inputs)
        s_Nodes.back().Inputs.emplace_back(GetNextId(), p.Label.c_str(), PinType::Flow);
    for(PinDescription p : Desc.Outputs)
        s_Nodes.back().Outputs.emplace_back(GetNextId(), p.Label.c_str(), PinType::Flow);

    Desc.InitializeDefaultProperties(s_Nodes.back().Properties);

    // Standard scrubber from examples.
    BuildNode(&s_Nodes.back());

    // "return" value from example spawner
    return &s_Nodes.back();
}

void nodos_session_data::RestoreRegistryNode(const std::string& NodeName,const std::string* Properties, int id)
{
    // Standard node spawner behavior, only we construct the objects
    // using the registry data.
    // NodeRegistry is a map, so we need the value.
    NodeDescription Desc = NodeRegistry[NodeName];

    // Create node object and pass the type name.
    s_Nodes.emplace_back(id, Desc.Type.c_str());
    // Because we didn't call GetNextID, we must manually put the ID counter where it
    // would be, had we done so.
    SetNextId(id + 1);

    // Handle creating the pins
    for(PinDescription p : Desc.Inputs)
        s_Nodes.back().Inputs.emplace_back(GetNextId(), p.Label.c_str(), PinType::Flow);
    for(PinDescription p : Desc.Outputs)
        s_Nodes.back().Outputs.emplace_back(GetNextId(), p.Label.c_str(), PinType::Flow);

    // Handle properties
    s_Nodes.back().Properties.deseralize(*Properties);


    // Standard scrubber from examples.
    BuildNode(&s_Nodes.back());
}
