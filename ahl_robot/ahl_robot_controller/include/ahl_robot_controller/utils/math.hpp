#ifndef __AHL_ROBOT_CONTROLLER_MATH_HPP
#define __AHL_ROBOT_CONTROLLER_MATH_HPP

#include <Eigen/Dense>

namespace ahl_robot
{
  namespace utils
  {  

    void convertRPYToRotationMatrix(double r, double p, double y, Eigen::Matrix3d& dst);
    Eigen::MatrixXd& getLastRowOfTransformationMatrix();
    void calculateInverseTransformationMatrix(const Eigen::Matrix4d& src, Eigen::Matrix4d& dst);

  }
}

#endif /* __AHL_ROBOT_CONTROLLER_MATH_HPP */