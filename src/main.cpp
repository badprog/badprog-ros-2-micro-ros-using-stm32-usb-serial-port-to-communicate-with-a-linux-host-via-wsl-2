#include <Arduino.h>
#include <micro_ros_platformio.h>

#include <rcl/rcl.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>
#include <std_msgs/msg/int32.h>
#include <std_msgs/msg/bool.h>

// ROS objects
rcl_publisher_t publisher;
std_msgs__msg__Int32 msg;
rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_timer_t timer;

#define LED_PIN_OK PE11 // Green
#define LED_PIN_BAD PE9 // Red

// ----------------------------------------------------------------------------
// timer_callback
// ----------------------------------------------------------------------------
void timer_callback(rcl_timer_t *timer, int64_t last_call_time)
{
  RCLC_UNUSED(last_call_time);
  if (timer != NULL)
  {
    rcl_ret_t rc = rcl_publish(&publisher, &msg, NULL);

    if (RCL_RET_OK == rc)
    {
      msg.data++;
      digitalWrite(LED_PIN_OK, !digitalRead(LED_PIN_OK)); // Fait clignoter la LED
      digitalWrite(LED_PIN_BAD, LOW);
    }
    else
    {
      digitalWrite(LED_PIN_BAD, HIGH);
    }
  }
}

// ----------------------------------------------------------------------------
// check_if_ok
// ----------------------------------------------------------------------------
void check_if_ok()
{
  digitalWrite(LED_PIN_OK, HIGH);
  digitalWrite(LED_PIN_BAD, HIGH);
  digitalWrite(PE15, HIGH);
  delay(500);
  digitalWrite(LED_PIN_OK, LOW);
  digitalWrite(LED_PIN_BAD, LOW);
  digitalWrite(PE15, LOW);
}

// ----------------------------------------------------------------------------
// setup
// ----------------------------------------------------------------------------
void setup()
{
  pinMode(LED_PIN_OK, OUTPUT);
  pinMode(LED_PIN_BAD, OUTPUT);

  // Configure serial transport (USB/ST-Link)
  set_microros_serial_transports(SerialUSB);

  // Allocator creation
  allocator = rcl_get_default_allocator();

  // Init
  rclc_support_init(&support, 0, NULL, &allocator);

  //
  check_if_ok();

  // Node creation
  rclc_node_init_default(&node, "badprog_node_stm32", "", &support);

  // Publisher creation on the topic "/badprog_topic_counter"
  rclc_publisher_init_default(
      &publisher,
      &node,
      ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
      "badprog_topic_counter");

  // Timer creation (every 1000 ms)
  rclc_timer_init_default(
      &timer,
      &support,
      RCL_MS_TO_NS(1000),
      timer_callback);


  // Executors
  const size_t number_of_handles = 1;
  rclc_executor_init(&executor, &support.context, number_of_handles, &allocator);
  rclc_executor_add_timer(&executor, &timer);

  // Reset message
  msg.data = 0;
}

// ----------------------------------------------------------------------------
// loop
// ----------------------------------------------------------------------------
void loop()
{
  rclc_executor_spin_some(&executor, RCL_MS_TO_NS(10));
}
