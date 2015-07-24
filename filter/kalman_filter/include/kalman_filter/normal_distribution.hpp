#ifndef __KALMAN_FILTER_NORMAL_DISTRIBUTION_HPP
#define __KALMAN_FILTER_NORMAL_DISTRIBUTION_HPP

#include <boost/shared_ptr.hpp>
#include <Eigen/Dense>

namespace kf
{

  class NormalDistribution
  {
  public:
    NormalDistribution() {}
    NormalDistribution(const Eigen::MatrixXd& mean, const Eigen::MatrixXd& variance)
      : mean_(mean), variance_(variance) {}

    void set(const Eigen::MatrixXd& mean, const Eigen::MatrixXd& variance)
    {
      mean_ = mean;
      variance_ = variance;
    }

    const Eigen::MatrixXd& getMean()
    {
      return mean_;
    }

    const Eigen::MatrixXd& getVariance()
    {
      return variance_;
    }

  private:
    Eigen::MatrixXd mean_;
    Eigen::MatrixXd variance_;
  };

  typedef boost::shared_ptr<NormalDistribution> NormalDistributionPtr;
}

#endif /* __KALMAN_FILTER_NORMAL_DISTRIBUTION_HPP */