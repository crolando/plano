#include <imgui.h> // for ImColor()
#include "utilities/builders.h"
#include <node_turnkey_types.h>

using namespace turnkey::api;
using namespace turnkey::types;

Node* SpawnInputActionNode(nodos_session_data& ctx)
{
    ctx.s_Nodes.emplace_back(ctx.GetNextId(), "InputAction Fire", ImColor(255, 128, 128));
    ctx.s_Nodes.back().Outputs.emplace_back(ctx.GetNextId(), "", PinType::Delegate);
    ctx.s_Nodes.back().Outputs.emplace_back(ctx.GetNextId(), "Pressed", PinType::Flow);
    ctx.s_Nodes.back().Outputs.emplace_back(ctx.GetNextId(), "Released", PinType::Flow);

    ctx.BuildNode(&ctx.s_Nodes.back());

    return &ctx.s_Nodes.back();
}

Node* SpawnBranchNode(nodos_session_data& ctx)
{
    ctx.s_Nodes.emplace_back(ctx.GetNextId(), "Branch");
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "", PinType::Flow);
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "Condition", PinType::Bool);
    ctx.s_Nodes.back().Outputs.emplace_back(ctx.GetNextId(), "True", PinType::Flow);
    ctx.s_Nodes.back().Outputs.emplace_back(ctx.GetNextId(), "False", PinType::Flow);

    ctx.BuildNode(&ctx.s_Nodes.back());

    return &ctx.s_Nodes.back();
}

Node* SpawnDoNNode(nodos_session_data& ctx)
{
    ctx.s_Nodes.emplace_back(ctx.GetNextId(), "Do N");
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "Enter", PinType::Flow);
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "N", PinType::Int);
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "Reset", PinType::Flow);
    ctx.s_Nodes.back().Outputs.emplace_back(ctx.GetNextId(), "Exit", PinType::Flow);
    ctx.s_Nodes.back().Outputs.emplace_back(ctx.GetNextId(), "Counter", PinType::Int);

    ctx.BuildNode(&ctx.s_Nodes.back());

    return &ctx.s_Nodes.back();
}

Node* SpawnOutputActionNode(nodos_session_data& ctx)
{
    ctx.s_Nodes.emplace_back(ctx.GetNextId(), "OutputAction");
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "Sample", PinType::Float);
    ctx.s_Nodes.back().Outputs.emplace_back(ctx.GetNextId(), "Condition", PinType::Bool);
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "Event", PinType::Delegate);

    ctx.BuildNode(&ctx.s_Nodes.back());

    return &ctx.s_Nodes.back();
}

Node* SpawnPrintStringNode(nodos_session_data& ctx)
{
    ctx.s_Nodes.emplace_back(ctx.GetNextId(), "Print String");
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "", PinType::Flow);
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "In String", PinType::String);
    ctx.s_Nodes.back().Outputs.emplace_back(ctx.GetNextId(), "", PinType::Flow);

    ctx.BuildNode(&ctx.s_Nodes.back());

    return &ctx.s_Nodes.back();
}

Node* SpawnMessageNode(nodos_session_data& ctx)
{
    ctx.s_Nodes.emplace_back(ctx.GetNextId(), "", ImColor(128, 195, 248));
    ctx.s_Nodes.back().Type = NodeType::Simple;
    ctx.s_Nodes.back().Outputs.emplace_back(ctx.GetNextId(), "Message", PinType::String);

    ctx.BuildNode(&ctx.s_Nodes.back());

    return &ctx.s_Nodes.back();
}

Node* SpawnSetTimerNode(nodos_session_data& ctx)
{
    ctx.s_Nodes.emplace_back(ctx.GetNextId(), "Set Timer", ImColor(128, 195, 248));
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "", PinType::Flow);
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "Object", PinType::Object);
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "Function Name", PinType::Function);
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "Time", PinType::Float);
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "Looping", PinType::Bool);
    ctx.s_Nodes.back().Outputs.emplace_back(ctx.GetNextId(), "", PinType::Flow);

    ctx.BuildNode(&ctx.s_Nodes.back());

    return &ctx.s_Nodes.back();
}

Node* SpawnLessNode(nodos_session_data& ctx)
{
    ctx.s_Nodes.emplace_back(ctx.GetNextId(), "<", ImColor(128, 195, 248));
    ctx.s_Nodes.back().Type = NodeType::Simple;
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "", PinType::Float);
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "", PinType::Float);
    ctx.s_Nodes.back().Outputs.emplace_back(ctx.GetNextId(), "", PinType::Float);

    ctx.BuildNode(&ctx.s_Nodes.back());

    return &ctx.s_Nodes.back();
}

Node* SpawnWeirdNode(nodos_session_data& ctx)
{
    ctx.s_Nodes.emplace_back(ctx.GetNextId(), "o.O", ImColor(128, 195, 248));
    ctx.s_Nodes.back().Type = NodeType::Simple;
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "", PinType::Float);
    ctx.s_Nodes.back().Outputs.emplace_back(ctx.GetNextId(), "", PinType::Float);
    ctx.s_Nodes.back().Outputs.emplace_back(ctx.GetNextId(), "", PinType::Float);

    ctx.BuildNode(&ctx.s_Nodes.back());

    return &ctx.s_Nodes.back();
}

Node* SpawnTraceByChannelNode(nodos_session_data& ctx)
{
    ctx.s_Nodes.emplace_back(ctx.GetNextId(), "Single Line Trace by Channel", ImColor(255, 128, 64));
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "", PinType::Flow);
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "Start", PinType::Flow);
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "End", PinType::Int);
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "Trace Channel", PinType::Float);
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "Trace Complex", PinType::Bool);
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "Actors to Ignore", PinType::Int);
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "Draw Debug Type", PinType::Bool);
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "Ignore Self", PinType::Bool);
    ctx.s_Nodes.back().Outputs.emplace_back(ctx.GetNextId(), "", PinType::Flow);
    ctx.s_Nodes.back().Outputs.emplace_back(ctx.GetNextId(), "Out Hit", PinType::Float);
    ctx.s_Nodes.back().Outputs.emplace_back(ctx.GetNextId(), "Return Value", PinType::Bool);

    ctx.BuildNode(&ctx.s_Nodes.back());

    return &ctx.s_Nodes.back();
}

Node* SpawnTreeSequenceNode(nodos_session_data& ctx)
{
    ctx.s_Nodes.emplace_back(ctx.GetNextId(), "Sequence");
    ctx.s_Nodes.back().Type = NodeType::Tree;
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "", PinType::Flow);
    ctx.s_Nodes.back().Outputs.emplace_back(ctx.GetNextId(), "", PinType::Flow);

    ctx.BuildNode(&ctx.s_Nodes.back());

    return &ctx.s_Nodes.back();
}

Node* SpawnTreeTaskNode(nodos_session_data& ctx)
{
    ctx.s_Nodes.emplace_back(ctx.GetNextId(), "Move To");
    ctx.s_Nodes.back().Type = NodeType::Tree;
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "", PinType::Flow);

    ctx.BuildNode(&ctx.s_Nodes.back());

    return &ctx.s_Nodes.back();
}

Node* SpawnTreeTask2Node(nodos_session_data& ctx)
{
    ctx.s_Nodes.emplace_back(ctx.GetNextId(), "Random Wait");
    ctx.s_Nodes.back().Type = NodeType::Tree;
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "", PinType::Flow);

    ctx.BuildNode(&ctx.s_Nodes.back());

    return &ctx.s_Nodes.back();
}

Node* SpawnComment(nodos_session_data& ctx)
{
    ctx.s_Nodes.emplace_back(ctx.GetNextId(), "Test Comment");
    ctx.s_Nodes.back().Type = NodeType::Comment;
    ctx.s_Nodes.back().Size = ImVec2(300, 200);

    return &ctx.s_Nodes.back();
}

Node* SpawnHoudiniTransformNode(nodos_session_data& ctx)
{
    ctx.s_Nodes.emplace_back(ctx.GetNextId(), "Transform");
    ctx.s_Nodes.back().Type = NodeType::Houdini;
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "", PinType::Flow);
    ctx.s_Nodes.back().Outputs.emplace_back(ctx.GetNextId(), "", PinType::Flow);

    ctx.BuildNode(&ctx.s_Nodes.back());

    return &ctx.s_Nodes.back();
}

Node* SpawnHoudiniGroupNode(nodos_session_data& ctx)
{
    ctx.s_Nodes.emplace_back(ctx.GetNextId(), "Group");
    ctx.s_Nodes.back().Type = NodeType::Houdini;
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "", PinType::Flow);
    ctx.s_Nodes.back().Inputs.emplace_back(ctx.GetNextId(), "", PinType::Flow);
    ctx.s_Nodes.back().Outputs.emplace_back(ctx.GetNextId(), "", PinType::Flow);

    ctx.BuildNode(&ctx.s_Nodes.back());

    return &ctx.s_Nodes.back();
}
