#ifndef EXAMPLE_NODE_SPAWNER_H
#define EXAMPLE_NODE_SPAWNER_H

#include <node_turnkey_types.h>

turnkey::types::Node* SpawnInputActionNode     (turnkey::types::SessionData* ctx);
turnkey::types::Node* SpawnBranchNode          (turnkey::types::SessionData* ctx);
turnkey::types::Node* SpawnDoNNode             (turnkey::types::SessionData* ctx);
turnkey::types::Node* SpawnOutputActionNode    (turnkey::types::SessionData* ctx);
turnkey::types::Node* SpawnPrintStringNode     (turnkey::types::SessionData* ctx);
turnkey::types::Node* SpawnMessageNode         (turnkey::types::SessionData* ctx);
turnkey::types::Node* SpawnSetTimerNode        (turnkey::types::SessionData* ctx);
turnkey::types::Node* SpawnLessNode            (turnkey::types::SessionData* ctx);
turnkey::types::Node* SpawnWeirdNode           (turnkey::types::SessionData* ctx);
turnkey::types::Node* SpawnTraceByChannelNode  (turnkey::types::SessionData* ctx);
turnkey::types::Node* SpawnTreeSequenceNode    (turnkey::types::SessionData* ctx);
turnkey::types::Node* SpawnTreeTaskNode        (turnkey::types::SessionData* ctx);
turnkey::types::Node* SpawnTreeTask2Node       (turnkey::types::SessionData* ctx);
turnkey::types::Node* SpawnComment             (turnkey::types::SessionData* ctx);
turnkey::types::Node* SpawnHoudiniTransformNode(turnkey::types::SessionData* ctx);
turnkey::types::Node* SpawnHoudiniGroupNode    (turnkey::types::SessionData* ctx);

#endif // EXAMPLE_NODE_SPAWNER_H
