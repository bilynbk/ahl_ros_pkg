/*********************************************************************
 *
 * Software License Agreement (BSD License)
 *
 *  Copyright (c) 2015, Daichi Yoshikawa
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of the Daichi Yoshikawa nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 * Author: Daichi Yoshikawa
 *
 *********************************************************************/

#include <stdexcept>
#include "ahl_youbot_server/exceptions.hpp"
#include "ahl_youbot_server/action/action_server.hpp"
#include "ahl_youbot_server/action/float_action.hpp"
#include "ahl_youbot_server/action/set_joint_action.hpp"
#include "ahl_youbot_server/action/joint_space_control_action.hpp"
#include "ahl_youbot_server/action/task_space_control_action.hpp"
#include "ahl_youbot_server/action/task_space_hybrid_control_action.hpp"
#include "ahl_youbot_server/interface/youbot_interface.hpp"
#include "ahl_youbot_server/interface/gazebo_interface.hpp"

using namespace ahl_youbot;

ActionServer::ActionServer(
  const std::string& robot_name, const std::string& mnp_name, const std::string& yaml,
  double period, double servo_period, bool use_real_robot)
  : updated_model_(false), mnp_name_(mnp_name)
{
  action_type_ = Action::FLOAT;

  // Initialize interface
  if(use_real_robot)
  {
    interface_ = InterfacePtr(new YoubotInterface());
  }
  else
  {
    std::vector<std::string> joint_list;
    joint_list.push_back("youbot::arm_joint_1");
    joint_list.push_back("youbot::arm_joint_2");
    joint_list.push_back("youbot::arm_joint_3");
    joint_list.push_back("youbot::arm_joint_4");
    joint_list.push_back("youbot::arm_joint_5");
    interface_ = InterfacePtr(new GazeboInterface(joint_list, servo_period * 5.0));
  }

  // Initialize robot
  using namespace ahl_robot;
  robot_ = RobotPtr(new Robot(robot_name));
  ParserPtr parser = ParserPtr(new Parser());
  parser->load(yaml, robot_);
  q_ = robot_->getJointPosition(mnp_name);
  dq_ = Eigen::VectorXd::Zero(q_.rows());

  const int xy_yaw = 3;
  q_base_.resize(xy_yaw);
  dq_base_.resize(q_base_.rows());

  if(q_base_.rows() > robot_->getDOF(mnp_name))
  {
    std::stringstream msg;
    msg << "q_base_.rows() > robot_->getDOF()" << std::endl
        << "  q_base_.rows   : " << q_base_.rows() << std::endl
        << "  robot_->getDOF : " << robot_->getDOF(mnp_name);
    throw ahl_youbot::Exception("ahl_youbot::ActionServer::ActionServer", msg.str());
  }

  for(unsigned int i = 0; i < q_base_.rows(); ++i)
  {
    q_base_.coeffRef(i)  = q_.coeff(i);
  }
  dq_base_ = Eigen::VectorXd::Zero(dq_base_.rows());

  q_arm_.resize(q_.rows() - q_base_.rows());
  dq_arm_.resize(q_arm_.rows());

  for(unsigned int i = q_base_.rows(); i < q_.rows(); ++i)
  {
    q_arm_.coeffRef(i - q_base_.rows())  = q_.coeff(i);
  }
  dq_arm_ = Eigen::VectorXd::Zero(dq_arm_.rows());

  // Initialize TfPublisher
  tf_pub_ = ahl_robot::TfPublisherPtr(new ahl_robot::TfPublisher());

  // Initialize robot controller
  controller_ = ahl_ctrl::RobotControllerPtr(new ahl_ctrl::RobotController());
  controller_->init(robot_, mnp_name);

  // Initialize action
  action_[Action::FLOAT] = ActionPtr(
    new FloatAction("float", robot_, controller_, interface_));
  action_[Action::SET_JOINT] = ActionPtr(
    new SetJointAction("set_joint_action", robot_));
  action_[Action::JOINT_SPACE_CONTROL] = ActionPtr(
    new JointSpaceControlAction("joint_space_control", robot_));
  action_[Action::TASK_SPACE_CONTROL] = ActionPtr(
    new TaskSpaceControlAction("task_space_control", robot_));
  action_[Action::TASK_SPACE_HYBRID_CONTROL] = ActionPtr(
    new TaskSpaceHybridControlAction("task_space_hybrid_control", robot_));

  // Initialize 2 timer loops
  ros::NodeHandle nh;

  timer_ = nh.createTimer(
    ros::Duration(period), &ActionServer::timerCB, this);
  servo_timer_ = nh.createTimer(
    ros::Duration(servo_period), &ActionServer::servoTimerCB, this);
}

void ActionServer::start()
{
  boost::mutex::scoped_lock lock(mutex_);
  timer_.start();
  servo_timer_.start();
}

void ActionServer::stop()
{
  boost::mutex::scoped_lock lock(mutex_);
  timer_.stop();
  servo_timer_.stop();
}

void ActionServer::timerCB(const ros::TimerEvent&)
{
  try
  {
    boost::mutex::scoped_lock lock(mutex_);
    // update robot model
    if(interface_->getJointStates(q_arm_))
    // TO DO : get base motion
    {
      for(unsigned int i = 0; i < q_base_.rows(); ++i)
      {
        q_.coeffRef(i) = q_base_.coeff(i);
      }
      for(unsigned int i = 0; i < q_.rows() - q_base_.rows(); ++i)
      {
        q_.coeffRef(i + q_base_.rows()) = q_arm_.coeff(i);
      }

      //std::cout << "q : " << std::endl;
      //std::cout << q_ << std::endl;

      robot_->update(mnp_name_, q_);
/*      ahl_robot::ManipulatorPtr mnp = robot_->getManipulator("mnp");
      for(unsigned int i = 0; i < q_.rows() - q_base_.rows(); ++i)
      {
        mnp->dq(i + q_base_.rows()) = dq_arm_.coeff(i);
      }
*/
      tf_pub_->publish(robot_, false);
      updated_model_ = true;
    }
    else
    {
      updated_model_ = false;
    }
  }
  catch(ahl_youbot::Exception& e)
  {
    ROS_ERROR_STREAM(e.what());
    return;
  }
  catch(ahl_robot::Exception& e)
  {
    ROS_ERROR_STREAM(e.what());
    return;
  }
  catch(std::exception& e)
  {
    ROS_ERROR_STREAM(e.what());
    return;
  }
  catch(...)
  {
    ROS_ERROR_STREAM("Unknown exception occurred.");
    return;
  }
}

void ActionServer::servoTimerCB(const ros::TimerEvent&)
{
  try
  {
    boost::mutex::scoped_lock lock(mutex_);
    if(!updated_model_)
      return;

    void* test;
    action_[action_type_]->execute(test);
  }
  catch(ahl_youbot::Exception& e)
  {
    ROS_ERROR_STREAM(e.what());
    return;
  }
  catch(ahl_robot::Exception& e)
  {
    ROS_ERROR_STREAM(e.what());
    return;
  }
  catch(std::exception& e)
  {
    ROS_ERROR_STREAM(e.what());
    return;
  }
  catch(...)
  {
    ROS_ERROR_STREAM("Unknown exception occurred.");
    return;
  }
}
