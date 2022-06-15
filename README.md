# engage_srv_converter

## Overview
This node acts as an interface to the autoware and outputs the input of pressing the departure button as the departure instruction of the autoware.

## Input and Output
- input
  - from engage_/[button_manager](https://github.com/eve-autonomy/button_manager)
    - `engage_button_manager/output/engage_button` : Departure button input. (this topic is remapping from /input/engage_button)
- output
  - to [autoware_state_machine](https://github.com/eve-autonomy/autoware_state_machine)
    - `/api/external/set/engage` : Departure instructions to Autoware. It is provided by the ROS service, not the ROS msg.

## Node Graph
![node graph](http://www.plantuml.com/plantuml/proxy?src=https://raw.githubusercontent.com/eve-autonomy/engage_srv_converter/docs/node_graph.pu)

## Parameter discription
This node has no parameters.
