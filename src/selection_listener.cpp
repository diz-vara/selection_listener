#include "ros/ros.h"
#include "sensor_msgs/PointCloud2.h"
#include "stdio.h"
#include "iostream"
#include "fstream"
#include "xsens_simple/xsens_msg.h"
#include "geometry_msgs/Quaternion.h"

//#include <sensor_msgs.h>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/highgui/highgui.hpp>
#include <image_transport/image_transport.h>


int cnt;

geometry_msgs::Quaternion orientation;

void XSensCallback(const xsens_simple::xsens_msg& msg)
{
  ROS_INFO("orientation received");
  if (msg.has_orientation) {
    orientation = msg.orientation;
  }


}


cv::Mat snapshot;

void ImageCallback(const sensor_msgs::ImageConstPtr& msg)
{
  ROS_INFO("image received");
  snapshot=cv_bridge::toCvShare(msg,sensor_msgs::image_encodings::BGR8)->image;
}

void SelCallback(const sensor_msgs::PointCloud2& msg)
{
  char filename[80];
  sprintf(filename,"/media/undead/Data/Voxels/out/%06d.", cnt);
  ROS_INFO("I heard: PointCloud2 [height = %d, width = %d, %06d]",msg.height, msg.width,
							cnt);

  std::ofstream outFile;
  cv::String csvName(filename);
  csvName += "csv";
  outFile.open(csvName.c_str(), std::ios::out );

  const char *cp =(const char*) msg.data.data();
  const float *fp  = (const float*)(cp);
  size_t step = 32; //read from msg

  for (int pnt = 0; pnt < msg.width; ++pnt) {
    //outFile.write(cp, 16);
    //cp += step;
    
    for (int coord = 0; coord < 3; ++coord) {
      outFile << *fp++;
      if (coord < 2)
        outFile << ",  ";
    }
    outFile << std::endl;
  }
  outFile << std::endl << "Orientation:" << std::endl;
  outFile << orientation;

  cv::String jpgName(filename);
  jpgName += "jpg";
  if (snapshot.data) {
    cv::imwrite(jpgName.c_str(), snapshot);
  }

  cnt++;
}

int main(int argc, char **argv)
{
  /**
   * The ros::init() function needs to see argc and argv so that it can perform
   * any ROS arguments and name remapping that were provided at the command line.
   * For programmatic remappings you can use a different version of init() which takes
   * remappings directly, but for most command-line programs, passing argc and argv is
   * the easiest way to do it.  The third argument to init() is the name of the node.
   *
   * You must call one of the versions of ros::init() before using any other
   * part of the ROS system.
   */
  ros::init(argc, argv, "selection_listener");
  ROS_INFO("selection_listener starts");
  cnt = 0;


  /**
   * NodeHandle is the main access point to communications with the ROS system.
   * The first NodeHandle constructed will fully initialize this node, and the last
   * NodeHandle destructed will close down the node.
   */
  ros::NodeHandle n;

  /**
   * The subscribe() call is how you tell ROS that you want to receive messages
   * on a given topic.  This invokes a call to the ROS
   * master node, which keeps a registry of who is publishing and who
   * is subscribing.  Messages are passed to a callback function, here
   * called chatterCallback.  subscribe() returns a Subscriber object that you
   * must hold on to until you want to unsubscribe.  When all copies of the Subscriber
   * object go out of scope, this callback will automatically be unsubscribed from
   * this topic.
   *
   * The second parameter to the subscribe() function is the size of the message
   * queue.  If messages are arriving faster than they are being processed, this
   * is the number of messages that will be buffered up before beginning to throw
   * away the oldest ones.
   */
  ros::Subscriber subSel = n.subscribe("/rviz_selected_points", 1000, SelCallback);
  ros::Subscriber subXSens = n.subscribe("/XSens", 1000, XSensCallback);

  image_transport::ImageTransport it(n);
  image_transport::Subscriber subImage = it.subscribe("/camera/basler_camera/image_raw", 10, ImageCallback);


   /**
   * ros::spin() will enter a loop, pumping callbacks.  With this version, all
   * callbacks will be called from within this thread (the main one).  ros::spin()
   * will exit when Ctrl-C is pressed, or the node is shutdown by the master.
   */
  ros::spin();

  return 0;
}


