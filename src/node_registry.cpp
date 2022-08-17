#include <internal/internal.h> // This brings in types and api as well.
                                   // also critically brings in s_Session, which these functions manipulate.
using namespace plano::types;
using namespace plano::api;

namespace plano {
namespace internal {

// oh this is the worst haha
PinType ParsePinType(const std::string& TypeStr) {
    if (TypeStr == "Flow") {
        return PinType::Flow;
    }
    if (TypeStr == "Bool") {
        return PinType::Bool;
    }
    if (TypeStr == "Int") {
        return PinType::Int;
    }
    if (TypeStr == "Float") {
        return PinType::Float;
    }
    if (TypeStr == "String") {
        return PinType::String;
    }
    if (TypeStr == "Object") {
        return PinType::Object;
    }
    if (TypeStr == "Function") {
        return PinType::Function;
    }
    if (TypeStr == "Delegate") {
        return PinType::Delegate;
    }
    printf("whoops");
    return PinType::Bool;
    
}

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
    for (PinDescription p : Desc.Inputs) {
        PinType pt = ParsePinType(p.DataType);
        s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), p.Label.c_str(), pt);
    }
    for (PinDescription p : Desc.Outputs) {
        PinType pt = ParsePinType(p.DataType);
        s_Session.s_Nodes.back().Outputs.emplace_back(GetNextId(), p.Label.c_str(), pt);
    }

    Desc.InitializeDefaultProperties(s_Session.s_Nodes.back().Properties);

    // Standard scrubber from examples.
    BuildNode(&s_Session.s_Nodes.back());

    // "return" value from example spawner
    return &s_Session.s_Nodes.back();
}

// This restores a node using the node definitions loaded into the registry.
// The properties of the node are copied in via deserialization of the project file.
#include <vector>
void RestoreRegistryNode(const std::string& NodeName, const std::string* Properties, int id, const std::vector<int>& pin_ids)
{
    // Standard node spawner behavior, only we construct the objects
    // using the registry data.
    // NodeRegistry is a map, so we need the value.
    NodeDescription Desc = s_Session.NodeRegistry[NodeName];

    // Create node object and pass the type name.
    s_Session.s_Nodes.emplace_back(id, Desc.Type.c_str());

    // Handle creating the pins
    int pin_id_idx = 0;
    for(PinDescription p : Desc.Inputs)
        s_Session.s_Nodes.back().Inputs.emplace_back(pin_ids[pin_id_idx++], p.Label.c_str(), PinType::Flow);
    for(PinDescription p : Desc.Outputs)
        s_Session.s_Nodes.back().Outputs.emplace_back(pin_ids[pin_id_idx++], p.Label.c_str(), PinType::Flow);

    // Handle property through deserialization
    plano::api::Prop_Deserialize(s_Session.s_Nodes.back().Properties,*Properties);

    // Standard scrubber from examples.
    BuildNode(&s_Session.s_Nodes.back());
}
} // inner namespace
} // outer namespace
