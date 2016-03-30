#include <gtest/gtest.h>
#include <image_transport/image_transport.h>
#include <ros/ros.h>

class DepthCameraTest : public testing::Test
{
protected:
  virtual void SetUp()
  {
    has_new_image_ = false;
    has_new_depth_ = false;
  }

  ros::NodeHandle nh_;
  image_transport::Subscriber cam_sub_;
  image_transport::Subscriber depth_sub_;
  bool has_new_image_;
  ros::Time image_stamp_;
  bool has_new_depth_;
  ros::Time depth_stamp_;
public:
  void imageCallback(const sensor_msgs::ImageConstPtr& msg)
  {
    image_stamp_ = msg->header.stamp;
    has_new_image_ = true;
  }
  void depthCallback(const sensor_msgs::ImageConstPtr& msg)
  {
    depth_stamp_ = msg->header.stamp;
    has_new_depth_ = true;
  }
};

// Test if the camera image is published at all, and that the timestamp
// is not too long in the past.
TEST_F(DepthCameraTest, cameraSubscribeTest)
{
  image_transport::ImageTransport it(nh_);
  cam_sub_ = it.subscribe("camera1/image_raw", 1,
                          &DepthCameraTest::imageCallback,
                          dynamic_cast<DepthCameraTest*>(this));
  depth_sub_ = it.subscribe("camera1/depth/image_raw", 1,
                            &DepthCameraTest::depthCallback,
                            dynamic_cast<DepthCameraTest*>(this));

#if 0
  // wait for gazebo to start publishing
  // TODO(lucasw) this isn't really necessary since this test
  // is purely passive
  bool wait_for_topic = true;
  while (wait_for_topic)
  {
    // @todo this fails without the additional 0.5 second sleep after the
    // publisher comes online, which means on a slower or more heavily
    // loaded system it may take longer than 0.5 seconds, and the test
    // would hang until the timeout is reached and fail.
    if (cam_sub_.getNumPublishers() > 0)
       wait_for_topic = false;
    ros::Duration(0.5).sleep();
  }
#endif

  while (!has_new_image_ || !has_new_depth_)
  {
    ros::spinOnce();
    ros::Duration(0.1).sleep();
  }

  EXPECT_EQ(depth_stamp_.toSec(), image_stamp_.toSec());
  // This check depends on the update period being much longer
  // than the expected difference between now and the received image time
  // TODO(lucasw)
  // this likely isn't that robust - what if the testing system is really slow?
  double time_diff;
  time_diff = (ros::Time::now() - image_stamp_).toSec();
  ROS_INFO_STREAM(time_diff);
  EXPECT_LT(time_diff, 0.5);

  time_diff = (ros::Time::now() - depth_stamp_).toSec();
  ROS_INFO_STREAM(time_diff);
  EXPECT_LT(time_diff, 0.5);

  cam_sub_.shutdown();
  depth_sub_.shutdown();
}

int main(int argc, char** argv)
{
  ros::init(argc, argv, "gazebo_depth_camera_test");
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
