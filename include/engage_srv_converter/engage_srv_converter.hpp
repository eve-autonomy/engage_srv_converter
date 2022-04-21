// Copyright 2021 eve autonomy inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License


#ifndef ENGAGE_SRV_CONVERTER__ENGAGE_SRV_CONVERTER_HPP_
#define ENGAGE_SRV_CONVERTER__ENGAGE_SRV_CONVERTER_HPP_

#include "rclcpp/rclcpp.hpp"

#include "tier4_api_utils/tier4_api_utils.hpp"
#include "tier4_external_api_msgs/srv/engage.hpp"
#include "autoware_state_machine_msgs/msg/vehicle_button.hpp"

namespace engage_srv_converter
{

class EngageSrvConverter : public rclcpp::Node
{
public:
  explicit EngageSrvConverter(const rclcpp::NodeOptions & options);

private:
  using ExternalEngage = tier4_external_api_msgs::srv::Engage;
  using VehicleButton = autoware_state_machine_msgs::msg::VehicleButton;

  // Callback group
  rclcpp::CallbackGroup::SharedPtr group_;

  // Subscription
  rclcpp::Subscription<VehicleButton>::SharedPtr sub_vehicle_button_;
  // Client
  rclcpp::Client<ExternalEngage>::SharedPtr cli_engage_;

  // Callback
  void onVehicleButtonStatus(
    const autoware_state_machine_msgs::msg::VehicleButton::SharedPtr message);
};

}  // namespace engage_srv_converter

#endif  // ENGAGE_SRV_CONVERTER__ENGAGE_SRV_CONVERTER_HPP_
