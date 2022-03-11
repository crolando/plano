#ifndef EXAMPLE_NODE_SPAWNER_H
#define EXAMPLE_NODE_SPAWNER_H
#include <node_turnkey_types.h>

turnkey::types::Node* SpawnInputActionNode     ();
turnkey::types::Node* SpawnBranchNode          ();
turnkey::types::Node* SpawnDoNNode             ();
turnkey::types::Node* SpawnOutputActionNode    ();
turnkey::types::Node* SpawnPrintStringNode     ();
turnkey::types::Node* SpawnMessageNode         ();
turnkey::types::Node* SpawnSetTimerNode        ();
turnkey::types::Node* SpawnLessNode            ();
turnkey::types::Node* SpawnWeirdNode           ();
turnkey::types::Node* SpawnTraceByChannelNode  ();
turnkey::types::Node* SpawnTreeSequenceNode    ();
turnkey::types::Node* SpawnTreeTaskNode        ();
turnkey::types::Node* SpawnTreeTask2Node       ();
turnkey::types::Node* SpawnComment             ();
turnkey::types::Node* SpawnHoudiniTransformNode();
turnkey::types::Node* SpawnHoudiniGroupNode    ();

#endif // EXAMPLE_NODE_SPAWNER_H
