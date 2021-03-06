# engage service converter

## Overview
This node converts a button action from ros topic to service for the Autoware interface.

## Input and Output
- input
  - from engage_/[button_manager](https://github.com/eve-autonomy/button_manager/)
    - `engage_button_manager/output/engage_button` \[[autoware_state_machine_msgs/msg/VehicleButton](https://github.com/eve-autonomy/autoware_state_machine_msgs/blob/main/msg/VehicleButton.msg)\]:<br>Departure button input. (this topic is remapped from `/input/engage_button`.)
- output
  - to [autoware_state_machine](https://github.com/eve-autonomy/autoware_state_machine/)
    - `/api/external/set/engage` \[[tier4_external_api_msgs/srv/Engage](https://github.com/tier4/tier4_autoware_msgs/blob/tier4/universe/tier4_external_api_msgs/srv/Engage.srv)\]:<br>Departure instructions to Autoware. This interface type is ROS2 service, not ROS2 message.

## Node Graph
![node graph](http://www.plantuml.com/plantuml/proxy?cache=no&src=https://raw.githubusercontent.com/eve-autonomy/engage_srv_converter/main/docs/node_graph.pu)

## Parameter description
This node has no parameters.
