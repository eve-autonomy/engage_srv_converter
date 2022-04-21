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


#include <memory>
#include "engage_srv_converter/engage_srv_converter.hpp"

namespace engage_srv_converter
{

EngageSrvConverter::EngageSrvConverter(
  const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
: Node("engage_srv_converter", options)
{
  using namespace std::placeholders;

  // This type of callback group only allows one callback to be executed at a time
  group_ = this->create_callback_group(
    rclcpp::CallbackGroupType::MutuallyExclusive);
  auto subscriber_option = rclcpp::SubscriptionOptions();
  subscriber_option.callback_group = group_;

  // Subscription
  sub_vehicle_button_ = this->create_subscription<autoware_state_machine_msgs::msg::VehicleButton>(
    "input/engage_button",
    rclcpp::QoS{1}.transient_local(),
    std::bind(&EngageSrvConverter::onVehicleButtonStatus, this, _1),
    subscriber_option
  );
  // Client
  cli_engage_ = this->create_client<tier4_external_api_msgs::srv::Engage>(
    "/api/external/set/engage");
}

void EngageSrvConverter::onVehicleButtonStatus(
  const autoware_state_machine_msgs::msg::VehicleButton::SharedPtr msg)
{
  auto req =
    std::make_shared<tier4_external_api_msgs::srv::Engage::Request>();
  req->engage = msg->data;

  auto callback = [this](
    rclcpp::Client<tier4_external_api_msgs::srv::Engage>::SharedFuture future) {
      // Receiving a response
      const auto response = future.get();
      if (!tier4_api_utils::is_success(response->status)) {
        RCLCPP_WARN_THROTTLE(
          this->get_logger(),
          *this->get_clock(), 1.0,
          "[engage_srv_converter] Engage Response Error!!!");
      }
    };
  // Sending a request
  auto result = cli_engage_->async_send_request(req, callback);
}
}  // namespace engage_srv_converter

#include "rclcpp_components/register_node_macro.hpp"

RCLCPP_COMPONENTS_REGISTER_NODE(engage_srv_converter::EngageSrvConverter)
