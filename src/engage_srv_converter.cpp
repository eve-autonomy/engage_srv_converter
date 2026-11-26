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

  use_double_button_ = this->declare_parameter<bool>("use_double_button");
  DoubleButtonParams judge_params;
  judge_params.min_press_sec = this->declare_parameter<double>("min_press_sec");
  judge_params.max_press_sec = this->declare_parameter<double>("max_press_sec");
  judge_params.min_overlap_sec = this->declare_parameter<double>("min_overlap_sec");
  judge_ = std::make_unique<DoubleButtonJudge>(judge_params);

  // Subscription
  if (use_double_button_) {
    // Double button: the "pressed" states of both buttons are judged.
    // The conventional "input/engage_button" (engage button alone) is NOT used.
    sub_engage_pressed_ = this->create_subscription<std_msgs::msg::Bool>(
      "input/engage_button_pressed",
      rclcpp::QoS{1}.transient_local(),
      [this](const std_msgs::msg::Bool::ConstSharedPtr msg) {
        onButtonPressed(DoubleButtonJudge::ENGAGE, msg->data);
      },
      subscriber_option);
    sub_reservation_pressed_ = this->create_subscription<std_msgs::msg::Bool>(
      "input/delivery_reservation_button_pressed",
      rclcpp::QoS{1}.transient_local(),
      [this](const std_msgs::msg::Bool::ConstSharedPtr msg) {
        onButtonPressed(DoubleButtonJudge::RESERVATION, msg->data);
      },
      subscriber_option);
    pub_double_engage_accepted_ = this->create_publisher<VehicleButton>(
      "output/double_engage_accepted", rclcpp::QoS{10});
    RCLCPP_INFO(this->get_logger(), "[engage_srv_converter] use_double_button: true");
  } else {
    sub_vehicle_button_ =
      this->create_subscription<autoware_state_machine_msgs::msg::VehicleButton>(
      "input/engage_button",
      rclcpp::QoS{1}.transient_local(),
      std::bind(&EngageSrvConverter::onVehicleButtonStatus, this, _1),
      subscriber_option
    );
    RCLCPP_INFO(this->get_logger(), "[engage_srv_converter] use_double_button: false");
  }
  // Client
  cli_engage_ = this->create_client<tier4_external_api_msgs::srv::Engage>(
    "/api/external/set/engage");
}

void EngageSrvConverter::onVehicleButtonStatus(
  const autoware_state_machine_msgs::msg::VehicleButton::SharedPtr msg)
{
  sendEngage(msg->data);
}

void EngageSrvConverter::onButtonPressed(const DoubleButtonJudge::Button button, const bool pressed)
{
  const auto verdict = judge_->onButton(button, pressed, this->now().seconds());
  if (!verdict.judged) {
    return;
  }
  if (!verdict.accepted) {
    RCLCPP_INFO(
      this->get_logger(), "[engage_srv_converter] double button is not valid. Engage is ignored.");
    return;
  }
  RCLCPP_INFO(
    this->get_logger(), "[engage_srv_converter] double button is valid (overlap %.2f sec).",
    verdict.overlap_sec);

  VehicleButton accepted;
  accepted.stamp = this->now();
  accepted.data = true;
  accepted.hold_down_time = verdict.overlap_sec;
  pub_double_engage_accepted_->publish(accepted);

  sendEngage(true);
}

void EngageSrvConverter::sendEngage(const bool engage)
{
  auto req =
    std::make_shared<tier4_external_api_msgs::srv::Engage::Request>();
  req->engage = engage;

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
