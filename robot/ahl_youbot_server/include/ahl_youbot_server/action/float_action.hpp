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

#ifndef __AHL_YOUBOT_SERVER_FLOAT_ACTION_HPP
#define __AHL_YOUBOT_SERVER_FLOAT_ACTION_HPP

#include <ahl_robot_srvs/Float.h>
#include <ahl_robot_controller/robot_controller.hpp>
#include <ahl_robot_controller/task/task.hpp>
#include "ahl_youbot_server/action/action.hpp"
#include "ahl_youbot_server/interface/interface.hpp"

namespace ahl_youbot
{

  class FloatAction : public Action
  {
  public:
    FloatAction(const std::string& action_name, const ahl_robot::RobotPtr& robot, const ahl_ctrl::RobotControllerPtr& controller, const ahl_youbot::InterfacePtr& interface);

    virtual void init();
    virtual void execute(void* goal);

  private:
    typedef ahl_robot_srvs::Float::Request FloatRequest;
    typedef boost::shared_ptr<FloatRequest> FloatRequestPtr;

    ahl_ctrl::RobotControllerPtr controller_;
    ahl_ctrl::TaskPtr task_;
    ahl_robot::RobotPtr robot_;
    InterfacePtr interface_;
  };

}

#endif /* __AHL_YOUBOT_SERVER_FLOAT_ACTION_HPP  */