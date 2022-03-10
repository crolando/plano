#include <node_turnkey_internal.h>



int turnkey::internal::GetNextId() {
    return s_Session.s_NextId++;
}

void turnkey::internal::SetNextId(int Id) {
    s_Session.s_NextId = Id;
}

void turnkey::internal::BuildNode(types::Node* node)
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

void turnkey::internal::BuildNodes(void)
{
    for (auto& node : s_Session.s_Nodes)
        BuildNode(&node);
}

