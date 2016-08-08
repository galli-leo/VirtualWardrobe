#pragma once
#include "Core.h"
#include "Engine.h"



DECLARE_STATS_GROUP(TEXT("KINECT"), STATGROUP_Kinect, STATCAT_Advanced);


DECLARE_CYCLE_STAT(TEXT("Kinect sensor Run Time"), STAT_KINECT_SENSOR_RunTime, STATGROUP_Kinect);
DECLARE_CYCLE_STAT(TEXT("Kinect Color Process Time"), STAT_KINECT_SENSOR_ColorProcessTime, STATGROUP_Kinect);
DECLARE_CYCLE_STAT(TEXT("Kinect Body Process Time"), STAT_KINECT_SENSOR_BodyProcessTime, STATGROUP_Kinect);
DECLARE_CYCLE_STAT(TEXT("Kinect Depth Process Time"), STAT_KINECT_SENSOR_DepthProcessTime, STATGROUP_Kinect);
DECLARE_CYCLE_STAT(TEXT("Kinect Infrared Process Time"), STAT_KINECT_SENSOR_InfraredProcessTime, STATGROUP_Kinect);
DECLARE_CYCLE_STAT(TEXT("Kinect Infrared Update Time"), STAT_KINECT_SENSOR_InfraredUpdateTime, STATGROUP_Kinect);
DECLARE_CYCLE_STAT(TEXT("Kinect Color Update Time"), STAT_KINECT_SENSOR_ColorUpdateTime, STATGROUP_Kinect);
DECLARE_CYCLE_STAT(TEXT("Kinect Depth Update Time"), STAT_KINECT_SENSOR_DepthUpdateTime, STATGROUP_Kinect);
DECLARE_CYCLE_STAT(TEXT("Kinect Body Index Process Time"), STAT_KINECT_SENSOR_BodyIndexProcessTime, STATGROUP_Kinect);
DECLARE_CYCLE_STAT(TEXT("Kinect Body Index Update Time"), STAT_KINECT_SENSOR_BodyIndexUpdateTime, STATGROUP_Kinect);
