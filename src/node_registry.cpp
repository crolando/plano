#include <internal/internal.h> // This brings in types and api as well.
                                   // also critically brings in s_Session, which these functions manipulate.
using namespace plano::types;
using namespace plano::api;

namespace plano {
namespace internal {

// This spawns a fresh node using the node definitions loaded into the registry.
// The properties of the node are intialized with defaults from the definition.
Node* NewRegistryNode(const std::string& NodeName) {

    // Standard node spawner behavior, only we construct the objects
    // using the registry data.
    // NodeRegistry is a map, so we need the value.
    NodeDescription Desc = s_Session.NodeRegistry[NodeName];

    // Create node object and pass the type name & color
    s_Session.s_Nodes.emplace_back(GetNextId(), Desc.Type.c_str(),Desc.Color);

    // Handle creating the pins
    for(PinDescription p : Desc.Inputs)
        s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), p.Label.c_str(), p.DataType);
    for(PinDescription p : Desc.Outputs)
        s_Session.s_Nodes.back().Outputs.emplace_back(GetNextId(), p.Label.c_str(), p.DataType);

    Desc.InitializeDefaultProperties(s_Session.s_Nodes.back().Properties);

    // Standard scrubber from examples.
    BuildNode(&s_Session.s_Nodes.back());

    // "return" value from example spawner
    return &s_Session.s_Nodes.back();
}

// This restores a node using the node definitions loaded into the registry.
// The properties are NOT restored - The caller must restore the properties onto
// the Node* that is returned.  
#include <vector>
plano::types::Node* RestoreRegistryNode(const std::string& NodeName, int id, const std::vector<int>& pin_ids)
{
    // Standard node spawner behavior, only we construct the objects
    // using the registry data.
    // NodeRegistry is a map, so we need the value.
    NodeDescription Desc = s_Session.NodeRegistry[NodeName];

    // Create node object and pass the type name and color.
    s_Session.s_Nodes.emplace_back(id, Desc.Type.c_str(),Desc.Color);

    // Handle creating the pins
    int pin_id_idx = 0;
    for(PinDescription p : Desc.Inputs)
        s_Session.s_Nodes.back().Inputs.emplace_back(pin_ids[pin_id_idx++], p.Label.c_str(), p.DataType);
    for(PinDescription p : Desc.Outputs)
        s_Session.s_Nodes.back().Outputs.emplace_back(pin_ids[pin_id_idx++], p.Label.c_str(), p.DataType);

    // Standard scrubber from examples.
    BuildNode(&s_Session.s_Nodes.back());

    return &s_Session.s_Nodes.back();
}
} // inner namespace
} // outer namespace
