#ifndef EXAMPLE_NODE_SPAWNER_H
#define EXAMPLE_NODE_SPAWNER_H

#include <node_turnkey_api.h>

turnkey::types::Node* SpawnInputActionNode     (turnkey::api::nodos_session_data* ctx);
turnkey::types::Node* SpawnBranchNode          (turnkey::api::nodos_session_data* ctx);
turnkey::types::Node* SpawnDoNNode             (turnkey::api::nodos_session_data* ctx);
turnkey::types::Node* SpawnOutputActionNode    (turnkey::api::nodos_session_data* ctx);
turnkey::types::Node* SpawnPrintStringNode     (turnkey::api::nodos_session_data* ctx);
turnkey::types::Node* SpawnMessageNode         (turnkey::api::nodos_session_data* ctx);
turnkey::types::Node* SpawnSetTimerNode        (turnkey::api::nodos_session_data* ctx);
turnkey::types::Node* SpawnLessNode            (turnkey::api::nodos_session_data* ctx);
turnkey::types::Node* SpawnWeirdNode           (turnkey::api::nodos_session_data* ctx);
turnkey::types::Node* SpawnTraceByChannelNode  (turnkey::api::nodos_session_data* ctx);
turnkey::types::Node* SpawnTreeSequenceNode    (turnkey::api::nodos_session_data* ctx);
turnkey::types::Node* SpawnTreeTaskNode        (turnkey::api::nodos_session_data* ctx);
turnkey::types::Node* SpawnTreeTask2Node       (turnkey::api::nodos_session_data* ctx);
turnkey::types::Node* SpawnComment             (turnkey::api::nodos_session_data* ctx);
turnkey::types::Node* SpawnHoudiniTransformNode(turnkey::api::nodos_session_data* ctx);
turnkey::types::Node* SpawnHoudiniGroupNode    (turnkey::api::nodos_session_data* ctx);

#endif // EXAMPLE_NODE_SPAWNER_H
