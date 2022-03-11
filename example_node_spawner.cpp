#include <imgui.h> // for ImColor()
#include "utilities/builders.h"
#include <node_turnkey_internal.h>


using namespace turnkey::api;
using namespace turnkey::types;
using namespace turnkey::internal;

Node* SpawnInputActionNode()
{
    s_Session.s_Nodes.emplace_back(GetNextId(), "InputAction Fire", ImColor(255, 128, 128));
    s_Session.s_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Delegate);
    s_Session.s_Nodes.back().Outputs.emplace_back(GetNextId(), "Pressed", PinType::Flow);
    s_Session.s_Nodes.back().Outputs.emplace_back(GetNextId(), "Released", PinType::Flow);

    BuildNode(&s_Session.s_Nodes.back());

    return &s_Session.s_Nodes.back();
}

Node* SpawnBranchNode()
{
    s_Session.s_Nodes.emplace_back(GetNextId(), "Branch");
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "Condition", PinType::Bool);
    s_Session.s_Nodes.back().Outputs.emplace_back(GetNextId(), "True", PinType::Flow);
    s_Session.s_Nodes.back().Outputs.emplace_back(GetNextId(), "False", PinType::Flow);

    BuildNode(&s_Session.s_Nodes.back());

    return &s_Session.s_Nodes.back();
}

Node* SpawnDoNNode()
{
    s_Session.s_Nodes.emplace_back(GetNextId(), "Do N");
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "Enter", PinType::Flow);
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "N", PinType::Int);
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "Reset", PinType::Flow);
    s_Session.s_Nodes.back().Outputs.emplace_back(GetNextId(), "Exit", PinType::Flow);
    s_Session.s_Nodes.back().Outputs.emplace_back(GetNextId(), "Counter", PinType::Int);

    BuildNode(&s_Session.s_Nodes.back());

    return &s_Session.s_Nodes.back();
}

Node* SpawnOutputActionNode()
{
    s_Session.s_Nodes.emplace_back(GetNextId(), "OutputAction");
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "Sample", PinType::Float);
    s_Session.s_Nodes.back().Outputs.emplace_back(GetNextId(), "Condition", PinType::Bool);
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "Event", PinType::Delegate);

    BuildNode(&s_Session.s_Nodes.back());

    return &s_Session.s_Nodes.back();
}

Node* SpawnPrintStringNode()
{
    s_Session.s_Nodes.emplace_back(GetNextId(), "Print String");
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "In String", PinType::String);
    s_Session.s_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);

    BuildNode(&s_Session.s_Nodes.back());

    return &s_Session.s_Nodes.back();
}

Node* SpawnMessageNode()
{
    s_Session.s_Nodes.emplace_back(GetNextId(), "", ImColor(128, 195, 248));
    s_Session.s_Nodes.back().Type = NodeType::Simple;
    s_Session.s_Nodes.back().Outputs.emplace_back(GetNextId(), "Message", PinType::String);

    BuildNode(&s_Session.s_Nodes.back());

    return &s_Session.s_Nodes.back();
}

Node* SpawnSetTimerNode()
{
    s_Session.s_Nodes.emplace_back(GetNextId(), "Set Timer", ImColor(128, 195, 248));
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "Object", PinType::Object);
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "Function Name", PinType::Function);
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "Time", PinType::Float);
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "Looping", PinType::Bool);
    s_Session.s_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);

    BuildNode(&s_Session.s_Nodes.back());

    return &s_Session.s_Nodes.back();
}

Node* SpawnLessNode()
{
    s_Session.s_Nodes.emplace_back(GetNextId(), "<", ImColor(128, 195, 248));
    s_Session.s_Nodes.back().Type = NodeType::Simple;
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Float);
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Float);
    s_Session.s_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Float);

    BuildNode(&s_Session.s_Nodes.back());

    return &s_Session.s_Nodes.back();
}

Node* SpawnWeirdNode()
{
    s_Session.s_Nodes.emplace_back(GetNextId(), "o.O", ImColor(128, 195, 248));
    s_Session.s_Nodes.back().Type = NodeType::Simple;
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Float);
    s_Session.s_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Float);
    s_Session.s_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Float);

    BuildNode(&s_Session.s_Nodes.back());

    return &s_Session.s_Nodes.back();
}

Node* SpawnTraceByChannelNode()
{
    s_Session.s_Nodes.emplace_back(GetNextId(), "Single Line Trace by Channel", ImColor(255, 128, 64));
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "Start", PinType::Flow);
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "End", PinType::Int);
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "Trace Channel", PinType::Float);
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "Trace Complex", PinType::Bool);
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "Actors to Ignore", PinType::Int);
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "Draw Debug Type", PinType::Bool);
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "Ignore Self", PinType::Bool);
    s_Session.s_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);
    s_Session.s_Nodes.back().Outputs.emplace_back(GetNextId(), "Out Hit", PinType::Float);
    s_Session.s_Nodes.back().Outputs.emplace_back(GetNextId(), "Return Value", PinType::Bool);

    BuildNode(&s_Session.s_Nodes.back());

    return &s_Session.s_Nodes.back();
}

Node* SpawnTreeSequenceNode()
{
    s_Session.s_Nodes.emplace_back(GetNextId(), "Sequence");
    s_Session.s_Nodes.back().Type = NodeType::Tree;
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
    s_Session.s_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);

    BuildNode(&s_Session.s_Nodes.back());

    return &s_Session.s_Nodes.back();
}

Node* SpawnTreeTaskNode()
{
    s_Session.s_Nodes.emplace_back(GetNextId(), "Move To");
    s_Session.s_Nodes.back().Type = NodeType::Tree;
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);

    BuildNode(&s_Session.s_Nodes.back());

    return &s_Session.s_Nodes.back();
}

Node* SpawnTreeTask2Node()
{
    s_Session.s_Nodes.emplace_back(GetNextId(), "Random Wait");
    s_Session.s_Nodes.back().Type = NodeType::Tree;
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);

    BuildNode(&s_Session.s_Nodes.back());

    return &s_Session.s_Nodes.back();
}

Node* SpawnComment()
{
    s_Session.s_Nodes.emplace_back(GetNextId(), "Test Comment");
    s_Session.s_Nodes.back().Type = NodeType::Comment;
    s_Session.s_Nodes.back().Size = ImVec2(300, 200);

    return &s_Session.s_Nodes.back();
}

Node* SpawnHoudiniTransformNode()
{
    s_Session.s_Nodes.emplace_back(GetNextId(), "Transform");
    s_Session.s_Nodes.back().Type = NodeType::Houdini;
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
    s_Session.s_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);

    BuildNode(&s_Session.s_Nodes.back());

    return &s_Session.s_Nodes.back();
}

Node* SpawnHoudiniGroupNode()
{
    s_Session.s_Nodes.emplace_back(GetNextId(), "Group");
    s_Session.s_Nodes.back().Type = NodeType::Houdini;
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
    s_Session.s_Nodes.back().Inputs.emplace_back(GetNextId(), "", PinType::Flow);
    s_Session.s_Nodes.back().Outputs.emplace_back(GetNextId(), "", PinType::Flow);

    BuildNode(&s_Session.s_Nodes.back());

    return &s_Session.s_Nodes.back();
}
