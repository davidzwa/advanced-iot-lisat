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
   m_convergedToLeader(false),
   m_nearestFinishedRobot({-1,-1,-1}),
   m_secondNearestFinishedRobot({-1,-1,-1}),
   m_fWheelVelocity(2.5f),
   m_cGoStraightAngleRange(-ToRadians(m_cAlpha),
                           ToRadians(m_cAlpha)) {}

/****************************************/
/****************************************/

int StringIDtoInt(std::string strid) {
   strid.std::string::erase(0, 2);
   return std::stoi(strid);
}


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

    RobotRelativeLocation leaderLocation = m_otherRobotLocations[m_nearestFinishedRobot.id];
    float distanceToClosestFinished = leaderLocation.distance;
    float angleToClosestFinished = leaderLocation.angle;

    /* If already finished stay stationary */
    if (this->m_isFinished) {
      return wheelVelocities;
    }
    if ( (distanceToClosestFinished > INTER_ROBOT_DISTANCE_THRESHOLD) && !m_convergedToLeader) { //0.25
      if (angleToClosestFinished < 5) {
        wheelVelocities = {ROBOT_UPPER_SPEED, ROBOT_UPPER_SPEED};
      }
      else if (angleToClosestFinished < 180) {
        wheelVelocities = {ROBOT_UPPER_SPEED, ROBOT_LOWER_SPEED};

      } else {
        wheelVelocities = {ROBOT_LOWER_SPEED, ROBOT_UPPER_SPEED};
      }
      return wheelVelocities;
    } else {
      m_convergedToLeader = true;
      argos::RLOG << "Finished robots: " << m_finishedRobotsCount << std::endl;
        if (m_finishedRobotsCount < 1) { // If robot is the first one to arrive just stay stationary 
        this->SetFinishedStatus();
        wheelVelocities = {0, 0};
        return wheelVelocities;
      } else {
            argos::RLOG << "1st nearest angle: " << m_nearestFinishedRobot.angle << std::endl;
            argos::RLOG << "2nd nearest angle: " << m_secondNearestFinishedRobot.angle << std::endl;
          if (abs(m_nearestFinishedRobot.angle - m_secondNearestFinishedRobot.angle) < 0.5) { // angle margin
            this->SetFinishedStatus();
            wheelVelocities = {0, 0};
            return wheelVelocities;
          } 
          if (m_nearestFinishedRobot.distance > INTER_ROBOT_DISTANCE_THRESHOLD - PSI_MARGIN) { // distance margin?
              if (m_nearestFinishedRobot.angle < 180) {
                wheelVelocities = {ROBOT_UPPER_SPEED, ROBOT_LOWER_SPEED};
                return wheelVelocities;
              } else {
                wheelVelocities  = {ROBOT_LOWER_SPEED, ROBOT_UPPER_SPEED};
                return wheelVelocities;
              }
          }
          else if (m_nearestFinishedRobot.distance < INTER_ROBOT_DISTANCE_THRESHOLD + PSI_MARGIN) {
              if (m_nearestFinishedRobot.angle < 180) {
                wheelVelocities = {ROBOT_LOWER_SPEED, ROBOT_UPPER_SPEED};
                return wheelVelocities;
              } else {
                wheelVelocities  = {ROBOT_UPPER_SPEED, ROBOT_LOWER_SPEED};
                return wheelVelocities;
              }
          }
          wheelVelocities = {2, 2};
          return wheelVelocities;      
        }
      }
  return wheelVelocities;
}

void CFootBotLisat::ControlStep() {
    argos::RLOG << "1st nearest robot id: " << m_nearestFinishedRobot.id << std::endl;
    argos::RLOG << "2nd nearest robot id: " << m_secondNearestFinishedRobot.id << std::endl;
  
  // If the robot is a leader it remains stationary
  if (m_isLeader || m_isFinished) return;

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

  /* select one robot for debugging messages */
    //argos::RLOG << "Robot id: " << id << std::endl;
}

void CFootBotLisat::ReceiveLocationMessage(float senderDistance, float senderAngle, int senderId, bool senderIsFinished) { //todo: use RobotRelativeLocation relativeLocation instead?
  RobotRelativeLocation relativeLocation = {senderDistance, senderAngle};

  m_otherRobotLocations[senderId] = relativeLocation;

  if (senderIsFinished) {
      if (m_nearestFinishedRobot.id == -1) {
        m_nearestFinishedRobot = {senderId, senderDistance, senderAngle};
        return;
      }
      if (senderDistance < m_nearestFinishedRobot.distance) {
        if (senderId == m_nearestFinishedRobot.id) {
          m_nearestFinishedRobot.distance = senderDistance;
          m_nearestFinishedRobot.angle = senderAngle;
          return;
        }
        m_secondNearestFinishedRobot = m_nearestFinishedRobot; // push down 1st place to 2nd place
        m_nearestFinishedRobot = {senderId, senderDistance, senderAngle};
        return;
      }
      else {
         if (senderId == m_nearestFinishedRobot.id) {
          m_nearestFinishedRobot.distance = senderDistance;
          m_nearestFinishedRobot.angle = senderAngle;
          return;
        }        
      }

    if (senderId != m_nearestFinishedRobot.id ) { // to ensure same id does not take up 1st and 2nd place
      if (m_secondNearestFinishedRobot.id == -1) {
        m_secondNearestFinishedRobot = {senderId, senderDistance, senderAngle};
        return;
      }
      if (senderDistance < m_secondNearestFinishedRobot.distance) {
         if (senderId == m_secondNearestFinishedRobot.id) {
          m_secondNearestFinishedRobot.distance = senderDistance;
          m_secondNearestFinishedRobot.angle = senderAngle;
          return;
        }
        m_secondNearestFinishedRobot = {senderId, senderDistance, senderAngle};
      }
      else {
        if (senderId == m_secondNearestFinishedRobot.id) {
          m_secondNearestFinishedRobot.distance = senderDistance;
          m_secondNearestFinishedRobot.angle = senderAngle;
          return;
        }    
      }
    }
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

bool CFootBotLisat::IsFinished() {
  return m_isFinished;
}

bool CFootBotLisat::checkBroadcastFinishedStatus() {
  return m_broadcastFinishedFlag;
}

void CFootBotLisat::confirmBroadcastFinishedStatus() {
  argos::RLOG << "finished flag unset " << m_finishedRobotsCount << std::endl;
  m_broadcastFinishedFlag = false;
}

void CFootBotLisat::updateRobotsFinishedCount(int count) {
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
