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

#include "ahl_robot_controller/exception.hpp"
#include "ahl_robot_controller/common/effective_mass_matrix3d.hpp"
#include "ahl_robot_controller/task/orientation_control.hpp"

using namespace ahl_ctrl;

OrientationControl::OrientationControl(const ahl_robot::ManipulatorPtr& mnp, const std::string& target_link, double eigen_thresh)
  : updated_(false), target_link_(target_link), eigen_thresh_(eigen_thresh)
{
  mnp_ = mnp;

  if(mnp_->name_to_idx.find(target_link) == mnp_->name_to_idx.end())
  {
    std::stringstream msg;
    msg << target_link << " was not found in mnp_->name_to_idx.";
    throw ahl_ctrl::Exception("OrientationControl::OrientationControl", msg.str());
  }

  idx_ = mnp_->name_to_idx[target_link];
  I_ = Eigen::MatrixXd::Identity(mnp_->dof, mnp_->dof);
  N_ = Eigen::MatrixXd::Identity(mnp_->dof, mnp_->dof);
}

void OrientationControl::setGoal(const Eigen::MatrixXd& Rd)
{
  if(Rd.rows() != 3)
  {
    std::stringstream msg;
    msg << "Rd.rows() != 3" << std::endl
        << "  Rd.rows : " << Rd.rows();
    throw ahl_ctrl::Exception("OrientationControl::setGoal", msg.str());
  }
  if(Rd.cols() != 3)
  {
    std::stringstream msg;
    msg << "Rd.cols() != 3" << std::endl
        << "  Rd.cols : " << Rd.cols();
    throw ahl_ctrl::Exception("OrientationControl::setGoal", msg.str());
  }

  Rd_ = Rd;
}

void OrientationControl::updateModel()
{
  Jw_ = mnp_->J0[idx_].block(3, 0, 3, mnp_->J0[idx_].cols());
  lambda_inv_ = Jw_ * mnp_->M_inv * Jw_.transpose();
  EffectiveMassMatrix3d::compute(lambda_inv_, lambda_, eigen_thresh_);
  J_dyn_inv_ = mnp_->M_inv * Jw_.transpose() * lambda_;
  N_ = I_ - Jw_.transpose() * J_dyn_inv_.transpose();

  updated_ = true;
}

void OrientationControl::computeGeneralizedForce(Eigen::VectorXd& tau)
{
  if(!updated_)
  {
    tau = Eigen::VectorXd::Zero(mnp_->dof);
    return;
  }

  Eigen::Matrix3d R = mnp_->T_abs[idx_].block(0, 0, 3, 3);
  Eigen::Quaternion<double> q;
  q = R * Rd_.inverse();
  double norm = sqrt(q.x() * q.x() + q.y() * q.y() + q.z() * q.z());
  Eigen::Vector3d del_phi;
  double c = 0.0;
  if(norm != 0.0)
  {
    c = 1.0 / norm * 2.0 * acos(q.w());
  }
  del_phi << q.x() * c, q.y() * c, q.z() * c;

  Eigen::VectorXd M_unit = -param_->getKp() * del_phi -param_->getKv() * Jw_ * mnp_->dq;
  Eigen::VectorXd M = lambda_ * M_unit;
  tau = Jw_.transpose() * M;
}

