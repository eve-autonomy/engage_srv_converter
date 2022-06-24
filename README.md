# engage service converter

## Overview
This node converts a button action from ros topic to service for the Autoware interface.

## Input and Output
- input
  - from engage_/[button_manager](https://github.com/eve-autonomy/button_manager/)
    - `engage_button_manager/output/engage_button` : Departure button input. (this topic is remapped from `/input/engage_button`.)
- output
  - to [autoware_state_machine](https://github.com/eve-autonomy/autoware_state_machine/)
    - `/api/external/set/engage` : Departure instructions to Autoware. It is provided by the ROS service, not the ROS msg.

## Node Graph
![node graph](http://www.plantuml.com/plantuml/proxy?src=https://raw.githubusercontent.com/eve-autonomy/engage_srv_converter/docs/node_graph.pu)

## Parameter description
This node has no parameters.
