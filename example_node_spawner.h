#ifndef EXAMPLE_NODE_SPAWNER_H
#define EXAMPLE_NODE_SPAWNER_H

#include <node_turnkey_types.h>

using namespace turnkey::types;
using namespace turnkey::api;

Node* SpawnInputActionNode(nodos_session_data& ctx);
Node* SpawnBranchNode(nodos_session_data& ctx);
Node* SpawnDoNNode(nodos_session_data& ctx);
Node* SpawnOutputActionNode(nodos_session_data& ctx);
Node* SpawnPrintStringNode(nodos_session_data& ctx);
Node* SpawnMessageNode(nodos_session_data& ctx);
Node* SpawnSetTimerNode(nodos_session_data& ctx);
Node* SpawnLessNode(nodos_session_data& ctx);
Node* SpawnWeirdNode(nodos_session_data& ctx);
Node* SpawnTraceByChannelNode(nodos_session_data& ctx);
Node* SpawnTreeSequenceNode(nodos_session_data& ctx);
Node* SpawnTreeTaskNode(nodos_session_data& ctx);
Node* SpawnTreeTask2Node(nodos_session_data& ctx);
Node* SpawnComment(nodos_session_data& ctx);
Node* SpawnHoudiniTransformNode(nodos_session_data& ctx);
Node* SpawnHoudiniGroupNode(nodos_session_data& ctx);

#endif // EXAMPLE_NODE_SPAWNER_H
