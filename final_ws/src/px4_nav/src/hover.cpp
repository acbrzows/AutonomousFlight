#include <ros/ros.h>
#include <std_msgs/String.h> 
#include <stdio.h>
#include "geometry_msgs/PoseStamped.h"
#include "geometry_msgs/Vector3Stamped.h"
#include <mavros_msgs/SetMode.h>
#include <mavros_msgs/State.h> 
#include <apriltags/AprilTagDetections.h>
#include <mavros_msgs/CommandBool.h>

geometry_msgs::PoseStamped CurrentPoseStamped;

void AprilMessageReceived(const apriltags::AprilTagDetections& detectionsMsg);

void AprilMessageReceived(const apriltags::AprilTagDetections& detectionsMsg) {

	if(&detectionsMsg.detections[0] != NULL) 
        {
        	CurrentPoseStamped.header = detectionsMsg.header;

		CurrentPoseStamped.pose.position.x = -detectionsMsg.detections[0].pose.position.x;
		CurrentPoseStamped.pose.position.y = detectionsMsg.detections[0].pose.position.y;
		CurrentPoseStamped.pose.position.z = detectionsMsg.detections[0].pose.position.z;

		CurrentPoseStamped.pose.orientation.x = detectionsMsg.detections[0].pose.orientation.w;
		CurrentPoseStamped.pose.orientation.y = -detectionsMsg.detections[0].pose.orientation.z;
		CurrentPoseStamped.pose.orientation.z = detectionsMsg.detections[0].pose.orientation.y;
		CurrentPoseStamped.pose.orientation.w = detectionsMsg.detections[0].pose.orientation.x;

		ROS_INFO("AprilTags Detected");
        }
	else
	{
		ROS_ERROR("No AprilTags Detected");
	}


	//include safety code if no april tags are detected
}

int main(int argc, char **argv)
{
   ros::init(argc, argv, "hover");
   ros::NodeHandle n;
   ros::NodeHandle n_apriltags;


ros::Publisher chatter_pub = n.advertise<geometry_msgs::PoseStamped>("/mavros/setpoint_position/local",100);
ros::Publisher current_pub = n.advertise<geometry_msgs::PoseStamped>("/mavros/mocap/pose",100);

   ros::Subscriber sub_apriltags = n_apriltags.subscribe("/apriltags/detections", 1000, &AprilMessageReceived);
   ros::ServiceClient set_mode_client = n.serviceClient<mavros_msgs::SetMode>("mavros/set_mode");
   ros::Rate loop_rate(100);
   ros::spinOnce();
 
   geometry_msgs::PoseStamped msg;
   int count = 1;
     
   while(ros::ok()){

       msg.header.stamp = ros::Time::now();
       msg.header.seq=count;
       msg.header.frame_id = 1;
       msg.pose.position.x = 0.0;
       msg.pose.position.y = 0.0;
       msg.pose.position.z = 1.0;
       msg.pose.orientation.x = 0;
       msg.pose.orientation.y = 0;
       msg.pose.orientation.z = 0;
       msg.pose.orientation.w = 1;
 
       if(count == -100)
       {
              mavros_msgs::SetMode offb_set_mode;
		offb_set_mode.request.base_mode = 0;              
		offb_set_mode.request.custom_mode = "OFFBOARD";
              //ROS_INFO("OFFBOARD mode set");
		
		if(set_mode_client.call(offb_set_mode)){
			ROS_INFO("setmode send ok %d value:", offb_set_mode.response.success);
		}else {
       			ROS_ERROR("Failed SetMode");
			return -1;
		}
		
              //mavros_msgs::CommandBool arm_cmd;
              //arm_cmd.request.value = true;
       }

       chatter_pub.publish(msg);
       //current_pub.publish(CurrentPoseStamped);
       ros::spinOnce();
       count++;
       loop_rate.sleep();
   }      
   return 0;
}
