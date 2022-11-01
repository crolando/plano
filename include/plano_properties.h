#ifndef PLANO_PROPERTIES_H
#define PLANO_PROPERTIES_H

/* Plano Properties
* 
* Explanation:
* Plano Nodes can have widgets inside their bodies.  Widgets, naturally, manipulate data.
* "Properties" store all the widget's data for a node. It is involved in these critical bits:
* State, (de)serialization, and default widget values (when a new node is created at runtime)
* 
* Your Burden:
* If you want to use widgets inside your nodes, you should use properties to track the data.
* Plano comes with a simple & slow implementation to get you off the ground (attr_table).  If you have perf
* problems, re-implement the Properties implementation. 
* 
* Properties will only make sense if they are serialized and deserialized.  When you call these things:
* plano::api::LoadNodesAndLinksFromBuffer()
* plano::api::SaveNodesAndLinksToBuffer()
* Those functions (that you implement) will also need to read and write the properties table.
* (Note: Like how plano provides a properties default, it also provides a default implmentation for those calls).
*/

// This area lets you define the datatype for properties.
#include <internal/attribute.h>
typedef attr_table Properties;


#endif