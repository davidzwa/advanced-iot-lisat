/*
 * AUTHOR: Carlo Pinciroli <cpinciro@ulb.ac.be>
 *
 * An example diffusion controller for the foot-bot.
 *
 * This controller makes the robots behave as gas particles. The robots
 * go straight until they get close enough to another robot, in which
 * case they turn, loosely simulating an elastic collision. The net effect
 * is that over time the robots diffuse in the environment.
 *
 * The controller uses the proximity sensor to detect obstacles and the
 * wheels to move the robot around.
 *
 * This controller is meant to be used with the XML files:
 *    experiments/diffusion_1.argos
 *    experiments/diffusion_10.argos
 */

#ifndef FOOTBOT_LISAT_H
#define FOOTBOT_LISAT_H
  
/*
 * Include some necessary headers.
 */
/* Definition of the CCI_Controller class. */
#include <argos3/core/control_interface/ci_controller.h>
/* Definition of the differential steering actuator */
#include <argos3/plugins/robots/generic/control_interface/ci_differential_steering_actuator.h>
/* Definition of the foot-bot proximity sensor */
#include <argos3/plugins/robots/foot-bot/control_interface/ci_footbot_proximity_sensor.h>
/* Definition of the LEDs actuator */
#include <argos3/plugins/robots/generic/control_interface/ci_leds_actuator.h>

#define INTER_ROBOT_DISTANCE_THRESHOLD  0.60 // distance robot should keep between each other      (in meters)
#define PSI_MARGIN                      0.01 // margin for push/pull around nearest finished robot (in meters)


#define ROBOT_UPPER_SPEED 4
#define ROBOT_LOWER_SPEED 2
/*
 * All the ARGoS stuff in the 'argos' namespace.
 * With this statement, you save typing argos:: every time.
 */
using namespace argos;

/* 
 * A structure which stores location 
 * info which an individual robot has 
 * about other robots 
 */
typedef struct {
  float distance;
  float angle;
} RobotRelativeLocation;

typedef struct {
  float leftWheelVelocity;
  float rightWheelVelocity;
} WheelVelocities;

typedef struct {
  int id;
  float distance;
  float angle;
} NearestFinishedRobot;

/*
 * A controller is simply an implementation of the CCI_Controller class.
 */
class CFootBotLisat : public CCI_Controller {

public:

   /* Class constructor. */
   CFootBotLisat();

   /* Class destructor. */
   virtual ~CFootBotLisat() {}

   /*
    * This function initializes the controller.
    * The 't_node' variable points to the <parameters> section in the XML
    * file in the <controllers><footbot_diffusion_controller> section.
    */
   virtual void Init(TConfigurationNode& t_node);

   /*
    * This function is called once every time step.
    * The length of the time step is set in the XML file.
    */
   virtual void ControlStep();

   /*
    * This function resets the controller to its state right after the
    * Init().
    * It is called when you press the reset button in the GUI.
    * In this example controller there is no need for resetting anything,
    * so the function could have been omitted. It's here just for
    * completeness.
    */
   virtual void Reset() {}

   /*
    * Called to cleanup what done by Init() when the experiment finishes.
    * In this example controller there is no need for clean anything up,
    * so the function could have been omitted. It's here just for
    * completeness.
    */
   virtual void Destroy() {}

   virtual void GiveLeaderStatus();
   virtual void RemoveLeaderStatus();
   virtual bool hasLeaderStatus();

   virtual void SetFinishedStatus();
   virtual void UnsetFinishedStatus();
   virtual bool IsFinished();

   virtual bool checkBroadcastFinishedStatus();
   virtual void confirmBroadcastFinishedStatus();
   virtual void updateRobotsFinishedCount(int count);

   virtual WheelVelocities LineCorrectionAlgorithm();

   virtual void ReceiveLocationMessage(float distance, float angle, int senderId, bool isFinished); 

private:

   /* Pointer to the differential steering actuator */
   CCI_DifferentialSteeringActuator* m_pcWheels;
   /* Pointer to the foot-bot proximity sensor */
   CCI_FootBotProximitySensor* m_pcProximity;

   /*
    * The following variables are used as parameters for the
    * algorithm. You can set their value in the <parameters> section
    * of the XML configuration file, under the
    * <controllers><footbot_diffusion_controller> section.
    */

   /* Maximum tolerance for the angle between
    * the robot heading direction and
    * the closest obstacle detected. */
   CDegrees m_cAlpha;
   /* Maximum tolerance for the proximity reading between
    * the robot and the closest obstacle.
    * The proximity reading is 0 when nothing is detected
    * and grows exponentially to 1 when the obstacle is
    * touching the robot.
    */
   Real m_fDelta;
   /* Wheel speed. */
   Real m_fWheelVelocity;
   /* Angle tolerance range to go straight.
    * It is set to [-alpha,alpha]. */
   CRange<CRadians> m_cGoStraightAngleRange;

   // The total number of robots in the experiment
   int m_number_of_robots;

   bool m_isLeader;
   bool m_isFinished; // indicates whether the robot is finished and at desired location
   /* indicates whether the robot recently finished 
    * meaning the loop controller still needs to 
    * broacast this to the other robots */
   bool m_broadcastFinishedFlag;

   RobotRelativeLocation* m_otherRobotLocations;

    /* Pointer to the LEDs actuator */
   CCI_LEDsActuator* m_pcLEDs;

   /* Number of other finished robots (excluding leader) */
   int m_finishedRobotsCount;

   NearestFinishedRobot m_nearestFinishedRobot;
   NearestFinishedRobot m_secondNearestFinishedRobot;

   // Whether robot has converged to leader at least once
   bool m_convergedToLeader;


};

#endif
