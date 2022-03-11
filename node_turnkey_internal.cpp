#include <node_turnkey_internal.h>
#include <draw_utils.h>

using namespace turnkey::types;
namespace ed = ax::NodeEditor;

namespace turnkey {
namespace internal {

types::SessionData s_Session;

int GetNextId() {
    return s_Session.s_NextId++;
}

void SetNextId(int Id) {
    s_Session.s_NextId = Id;
}

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

void BuildNodes(void)
{
    for (auto& node : s_Session.s_Nodes)
        BuildNode(&node);
}

bool isNodeAncestor(types::Node* Ancestor, types::Node* Decendent) {
    auto decendent_inputs = Decendent->Inputs;

    // Early return for nodes that don't have inputs
    if(decendent_inputs.size() == 0) {
        return false;
    }

    // Handle nodes that have inputs.
    for(Pin p : decendent_inputs) {
        // Early return for unlinked pins
        if(!IsPinLinked(p.ID)) {
            continue;
        }
        // Handle a linked pin.  Pins do NOT know who they are attached to.
        // We have to search the links for the connected node.
        std::vector<Node*> AncestorNodes;
        for (auto& link : s_Session.s_Links) {
            if(link.EndPinID == p.ID) {
                auto n = FindPin(link.StartPinID)->Node;


                if (n->ID == Ancestor->ID) {
                    return true;
                } else {
                    AncestorNodes.push_back(n);
                }
            }
        }
        for(auto AncestorNode : AncestorNodes) {
            if(!AncestorNode) {
                return false;
            } else {
                if(AncestorNode->ID == Ancestor->ID) {
                    return true;
                } else {
                   if(isNodeAncestor(Ancestor,AncestorNode))
                       return true;
                } // here we let pass through!
            }
        } // Done searching ancestor nodes
    } // Done searching pins
}

ed::NodeId GetNextLinkId()
{
    return ed::NodeId(GetNextId());
}


Node* FindNode(ed::NodeId id)
{
    for (auto& node : s_Session.s_Nodes)
        if (node.ID == id)
            return &node;

    return nullptr;
}

Link* FindLink(ed::LinkId id)
{
    for (auto& link : s_Session.s_Links)
        if (link.ID == id)
            return &link;

    return nullptr;
}

Pin* FindPin(ed::PinId id)
{
    if (!id)
        return nullptr;

    for (auto& node : s_Session.s_Nodes)
    {
        for (auto& pin : node.Inputs)
            if (pin.ID == id)
                return &pin;

        for (auto& pin : node.Outputs)
            if (pin.ID == id)
                return &pin;
    }

    return nullptr;
}

bool IsPinLinked(ed::PinId id)
{
    if (!id)
        return false;

    for (auto& link : s_Session.s_Links)
        if (link.StartPinID == id || link.EndPinID == id)
            return true;

    return false;
}


void RegisterNewNode(api::NodeDescription NewDescription) {
    s_Session.NodeRegistry[NewDescription.Type] = NewDescription;
}

using ax::Drawing::IconType;

void DrawPinIcon(const Pin& pin, bool connected, int alpha)
{
    IconType iconType;
    ImColor  color = GetIconColor(pin.Type);
    color.Value.w = alpha / 255.0f;
    switch (pin.Type)
    {
        case PinType::Flow:     iconType = IconType::Flow;   break;
        case PinType::Bool:     iconType = IconType::Circle; break;
        case PinType::Int:      iconType = IconType::Circle; break;
        case PinType::Float:    iconType = IconType::Circle; break;
        case PinType::String:   iconType = IconType::Circle; break;
        case PinType::Object:   iconType = IconType::Circle; break;
        case PinType::Function: iconType = IconType::Circle; break;
        case PinType::Delegate: iconType = IconType::Square; break;
        default:
            return;
    }

    ax::Widgets::Icon(ImVec2(s_Session.s_PinIconSize, s_Session.s_PinIconSize), iconType, connected, color, ImColor(32, 32, 32, alpha));
};


} // inner namespace
} // outer namespace
