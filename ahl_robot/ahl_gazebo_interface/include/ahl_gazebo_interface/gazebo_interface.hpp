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

#ifndef __AHL_GAZEBO_INTERFACE_GAZEBO_INTERFACE_HPP
#define __AHL_GAZEBO_INTERFACE_GAZEBO_INTERFACE_HPP

#include <map>
#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <Eigen/Dense>
#include <ros/ros.h>
#include <gazebo_msgs/JointStates.h>
#include <gazebo_msgs/ModelState.h>
#include <gazebo_msgs/ModelStates.h>
#include <gazebo_msgs/ApplyJointEfforts.h>

namespace ahl_gazebo_if
{

  static const std::string TOPIC_PUB_JOINT_EFFORT = "/gazebo/apply_joint_efforts";
  static const std::string TOPIC_PUB_MODEL_STATE  = "/gazebo/set_model_state";
  static const std::string TOPIC_SUB_JOINT_STATES = "/gazebo/joint_states";
  static const std::string TOPIC_SUB_MODEL_STATES = "/gazebo/model_states";

  class GazeboInterface
  {
  public:
    GazeboInterface();

    void addJoint(const std::string& name);
    void addMobility2D(const std::string& robot);
    void setDuration(double duration);
    void connect();
    bool subscribed();
    void applyJointEfforts(const Eigen::VectorXd& tau);
    void applyVelocityCommand2D(const Eigen::Vector3d& v);

    const Eigen::VectorXd& getJointStates() const
    {
      return q_;
    }

  private:
    void subscribeJointStates(const gazebo_msgs::JointStates::ConstPtr& msg);
    void subscribeModelStates(const gazebo_msgs::ModelStates::ConstPtr& msg);

    boost::mutex mutex_;

    std::map<std::string, double> q_map_;
    std::map<std::string, int> name_to_idx_;
    std::vector<std::string> name_list_;
    unsigned int joint_num_;

    Eigen::VectorXd q_;
    gazebo_msgs::ApplyJointEfforts effort_;
    ros::Duration duration_;
    ros::Publisher pub_effort_;
    ros::Subscriber sub_joint_states_;
    bool subscribed_joint_states_;

    std::string robot_;
    gazebo_msgs::ModelState model_state_;
    int joint_idx_offset_;
    bool use_mobility_2d_;
    bool subscribed_model_states_;
    ros::Publisher pub_model_state_;
    ros::Subscriber sub_model_states_;
  };

  typedef boost::shared_ptr<GazeboInterface> GazeboInterfacePtr;
}

#endif /* __AHL_GAZEBO_INTERFACE_GAZEBO_INTERFACE_HPP */
