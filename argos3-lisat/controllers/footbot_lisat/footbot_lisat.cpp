/* Include the controller definition */
#include "footbot_lisat.h"
/* Function definitions for XML parsing */
#include <argos3/core/utility/configuration/argos_configuration.h>
/* 2D vector definition */
#include <argos3/core/utility/math/vector2.h>

/* logging functions */
#include <argos3/core/utility/logging/argos_log.h>

/* Definition of the LEDs actuator */
#include <argos3/plugins/robots/generic/control_interface/ci_leds_actuator.h>

#include <loop_functions/lisat_loop_functions/lisat_loop_functions.h>

#include <string>


/****************************************/
/****************************************/

CFootBotLisat::CFootBotLisat() :
   m_pcWheels(NULL),
   m_pcProximity(NULL),
   m_cAlpha(10.0f),
   m_pcLEDs(NULL),
   m_finishedRobotsCount(0),
   m_isLeader(false),
   m_isFinished(false),
   m_broadcastFinishedFlag(false),
   m_fDelta(0.5f),
   m_fWheelVelocity(2.5f),
   m_cGoStraightAngleRange(-ToRadians(m_cAlpha),
                           ToRadians(m_cAlpha)) {}

/****************************************/
/****************************************/

void CFootBotLisat::Init(TConfigurationNode& t_node) {
   /*
    * Get sensor/actuator handles
    *
    * The passed string (ex. "differential_steering") corresponds to the
    * XML tag of the device whose handle we want to have. For a list of
    * allowed values, type at the command prompt:
    *
    * $ argos3 -q actuators
    *
    * to have a list of all the possible actuators, or
    *
    * $ argos3 -q sensors
    *
    * to have a list of all the possible sensors.
    *
    * NOTE: ARGoS creates and initializes actuators and sensors
    * internally, on the basis of the lists provided the configuration
    * file at the <controllers><footbot_diffusion><actuators> and
    * <controllers><footbot_diffusion><sensors> sections. If you forgot to
    * list a device in the XML and then you request it here, an error
    * occurs.
    */
   m_pcWheels    = GetActuator<CCI_DifferentialSteeringActuator>("differential_steering");
   m_pcProximity = GetSensor  <CCI_FootBotProximitySensor      >("footbot_proximity"    );
   /*
    * Parse the configuration file
    *
    * The user defines this part. Here, the algorithm accepts three
    * parameters and it's nice to put them in the config file so we don't
    * have to recompile if we want to try other settings.
    */
   GetNodeAttributeOrDefault(t_node, "alpha", m_cAlpha, m_cAlpha);
   m_cGoStraightAngleRange.Set(-ToRadians(m_cAlpha), ToRadians(m_cAlpha));
   GetNodeAttributeOrDefault(t_node, "delta", m_fDelta, m_fDelta);
   GetNodeAttributeOrDefault(t_node, "velocity", m_fWheelVelocity, m_fWheelVelocity);

   /* Store number of robots through parameters in .argos file */
   GetNodeAttributeOrDefault(t_node, "number_of_robots", m_number_of_robots, m_number_of_robots);

   /* Contains location information about other robots */
   m_otherRobotLocations = new RobotRelativeLocation[20]; //TODO: make dynamic (number_of_robots gives bad alloc)

   m_pcLEDs   = GetActuator<CCI_LEDsActuator                          >("leds");

}

/****************************************/
/****************************************/

/* The line correction algorithm */
WheelVelocities CFootBotLisat::LineCorrectionAlgorithm() {

    WheelVelocities wheelVelocities = {0, 0};

    RobotRelativeLocation leaderLocation = m_otherRobotLocations[0];
    float distanceToLeader = leaderLocation.distance;
    float angleToLeader = leaderLocation.angle;


    //argos::RLOG << "leader distance: " << distanceToLeader << std::endl;
    //argos::RLOG << "leader angle: " << angleToLeader << std::endl;

    /* If already finished stay stationary */
    if (this->m_isFinished) {
      return wheelVelocities;
    }
    if (distanceToLeader > INTER_ROBOT_DISTANCE_THRESHOLD) { //0.25
      if (angleToLeader < 5) {
        wheelVelocities = {3, 3};
      }
      else if (angleToLeader < 180) {
        wheelVelocities = {3, 1};

      } else {
        wheelVelocities = {1, 3};
      }
    } else {

      if (m_finishedRobotsCount < 1) { // If robot is the first one to arrive just stay stationary 
        this->SetFinishedStatus();
        wheelVelocities = {0, 0};
        return wheelVelocities;
      } else {
        wheelVelocities = {2.5, -2.5};
        return wheelVelocities;      }

    }
    // robotsCloserToStartpoint = CalculateRobotsCloserToStartpoint();

    // // If close to startpoint
    // if (disanceToStartpoint < interRobotDistanceValue) {
    //   if (robotsCloserToStartpoint.length() == 0) {
    //     //self.at_final_position = True;
    //     wheelVelocities = {0, 0};
    //     return wheelVelocities;
    //   }
    //   else {
    //     potential_positions = calculate_desired_positions(line.startpointAdjacentPositions, robotsCloserToStartpoint)
    //     distanceToPotentialPositions = calculate_distances(potentialPositions);
    //     return min(distanceToPotentialPositions) // return closest potential position 
    //   }
    // }
    // else {
    //   return startpoint;
    // }


  //WheelVelocities wheelVelocities = {5, 5};
  return wheelVelocities;
}

void CFootBotLisat::ControlStep() {

  // If the robot is a leader it remains stationary
  if (m_isLeader) return;

   /* Get readings from proximity sensor */
   const CCI_FootBotProximitySensor::TReadings& tProxReads = m_pcProximity->GetReadings();
   /* Sum them together */
   CVector2 cAccumulator;
   for(size_t i = 0; i < tProxReads.size(); ++i) {
      cAccumulator += CVector2(tProxReads[i].Value, tProxReads[i].Angle);
   }
   cAccumulator /= tProxReads.size();
   /* If the angle of the vector is small enough and the closest obstacle
    * is far enough, continue going straight, otherwise curve a little
    */
   CRadians cAngle = cAccumulator.Angle();
   if(m_cGoStraightAngleRange.WithinMinBoundIncludedMaxBoundIncluded(cAngle) &&
      cAccumulator.Length() < m_fDelta ) {


      /* Set wheel velocity for target location */
      WheelVelocities targetWheelVelocities = LineCorrectionAlgorithm();
      m_pcWheels->SetLinearVelocity(targetWheelVelocities.leftWheelVelocity, targetWheelVelocities.rightWheelVelocity);

   }
   else {
      /* Turn, depending on the sign of the angle */
      if(cAngle.GetValue() > 0.0f) {
         m_pcWheels->SetLinearVelocity(m_fWheelVelocity, 0.0f);
      }
      else {
         m_pcWheels->SetLinearVelocity(0.0f, m_fWheelVelocity);
      }
   }

  /* select on robot for debugging messages */
  std::string id = this->GetId(); 
  if (id == "fb2") {
    //argos::RLOG << "Robot id: " << id << std::endl;
    argos::RLOG << "Finished robots: " << m_finishedRobotsCount << std::endl;
  }

}

void CFootBotLisat::ReceiveLocationMessage(float distance, float angle, bool fromLeader) { //todo: add robot id, // use: RobotRelativeLocation relativeLocation instead?
  RobotRelativeLocation relativeLocation = {distance, angle};
  if (fromLeader) { // for leader use id 0
    m_otherRobotLocations[0] = relativeLocation;
  } else { // not leader so use robot_id
  }
};

void CFootBotLisat::GiveLeaderStatus() {
  m_isLeader = true;
  m_pcLEDs->SetAllColors(CColor::RED);
}

void CFootBotLisat::RemoveLeaderStatus() {
  m_isLeader = false;
  m_pcLEDs->SetAllColors(CColor::BLACK);
}

bool CFootBotLisat::hasLeaderStatus() {
  return m_isLeader;
}

void CFootBotLisat::SetFinishedStatus() {
  m_isFinished = true;
  m_broadcastFinishedFlag = true;
  m_pcLEDs->SetAllColors(CColor::YELLOW);

} 

void CFootBotLisat::UnsetFinishedStatus() {
  m_isFinished = false;
  m_pcLEDs->SetAllColors(CColor::BLACK);
}

bool CFootBotLisat::checkBroadcastFinishedStatus() {
  return m_broadcastFinishedFlag;
}

void CFootBotLisat::confirmBroadcastFinishedStatus() {
  argos::RLOG << "finished flag unset " << m_finishedRobotsCount << std::endl;
  m_broadcastFinishedFlag = false;
}

void CFootBotLisat::updateRobotsFinishedCount(int count) {
  //argos::RLOG << "Finished robots: " << m_finishedRobotsCount << std::endl;
  m_finishedRobotsCount += count;
}


/****************************************/
/****************************************/

/*
 * This statement notifies ARGoS of the existence of the controller.
 * It binds the class passed as first argument to the string passed as
 * second argument.
 * The string is then usable in the configuration file to refer to this
 * controller.
 * When ARGoS reads that string in the configuration file, it knows which
 * controller class to instantiate.
 * See also the configuration files for an example of how this is used.
 */
REGISTER_CONTROLLER(CFootBotLisat, "footbot_lisat_controller")
