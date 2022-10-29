# plano
refactor &amp; augmentation of thedmd's imgui-node-editor blueprint example to be a more plug-and-play library.

Loads of code taken from: https://github.com/thedmd/imgui-node-editor

The above library is a "way to make your own node graph system".
I found that the blueprint examples, while compelling, were not really plug-and-play solutions.

This project aims to provide a layer that rides on top of thedmd/imgui-node-editor that provides these changes from thedmd's "blueprint example":
1. The blueprint example functions aren't dependent on the example framework, eg imgui-node-editor\examples\application\
2. The implementation details of a node type is reorganized to a small api.  So to make a new node type known to the editor, you implement simple callbacks, and then the turnkey layer handles runtime and savefile instantation, drawing, interaction, etc.
3. Full serialization and deserialzation is implemented
4. Individual node instances track their own personal configuration data using a simple attribute system.  this makes saving and loading node configuration simple, and writing the node gui widget interactions simple to implement.
5. Moves the blueprint example's context variables from a pile of static variables to a context container. Passes this context container function calls. 



