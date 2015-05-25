#include <ros/ros.h>
#include "ahl_robot/exception.hpp"
#include "ahl_robot/robot/parser.hpp"
#include "ahl_robot/tf/tf_publisher.hpp"

using namespace ahl_robot;

int main(int argc, char** argv)
{
  ros::init(argc, argv, "parser_test");
  ros::NodeHandle nh;

  try
  {
    std::string name = "youbot";
    RobotPtr robot = RobotPtr(new Robot(name));

    ParserPtr parser = ParserPtr(new Parser());
    std::string path = "/home/daichi/Work/catkin_ws/src/ahl_ros_pkg/ahl_robot/ahl_robot/yaml/youbot.yaml";
    parser->load(path, robot);

    ros::MultiThreadedSpinner spinner;

    TfPublisherPtr tf_publisher = TfPublisherPtr(new TfPublisher());

    unsigned long cnt = 0;
    while(ros::ok())
    {
      ManipulatorPtr mnp = robot->getManipulator("mnp");
      Eigen::VectorXd q = Eigen::VectorXd::Constant(mnp->dof, 1.0);
      double coeff = 0.5 * sin(2.0 * M_PI * 0.1 * cnt * 0.1);
      mnp->q = coeff * q;
      mnp->q.coeffRef(0) = 0.0;
      mnp->q.coeffRef(1) = 0.0;
      mnp->q.coeffRef(2) = 0.0;
      ++cnt;
      mnp->computeFK();
      tf_publisher->publish(robot);
      ros::Duration(0.1).sleep();
    }
  }
  catch(ahl_robot::Exception& e)
  {
    ROS_ERROR_STREAM(e.what());
  }

  return 0;
}
