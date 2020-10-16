#ifndef FORAGING_LOOP_FUNCTIONS_H
#define FORAGING_LOOP_FUNCTIONS_H

#include <argos3/core/simulator/loop_functions.h>
#include <argos3/core/simulator/entity/floor_entity.h>
#include <argos3/core/utility/math/range.h>
#include <argos3/core/utility/math/rng.h>
<<<<<<< HEAD


#include<bits/stdc++.h> 
=======
#include <chrono>
>>>>>>> parent of b20f4ec... finished implementation to measure distance between a point and a line defined by two robot's location.

#include <random>

using namespace argos;

<<<<<<< HEAD

#define NUMBER_OF_MICS 8 // NEEDS TO BE HIGHER THAN 2 ELSE FORMULA IS IRREGULAR  

#define NOISE_MEAN 0.0
#define NOISE_STD 5.0

class Vector { 

public: 
    Vector(float x, float y, float z) 
    { 
        // Constructor 
        this->x = x; 
        this->y = y; 
        this->z = z; 
    } 
    Vector operator + (Vector v); // ADD 2 Vectors 
    Vector operator-(Vector v); // Subtraction 
    float operator^(Vector v); // Dot Product 
    Vector operator*(Vector v); // Cross Product 
    float magnitude() 
    { 
        return sqrt(pow(x, 2) + pow(y, 2) + pow(z, 2)); 
    } 
    // To output the Vector 
    float x, y, z; 

private: 
    // 3D Coordinates of the Vector 

}; 

// ADD 2 Vectors 
Vector Vector::operator+(Vector v) 
{ 
    float x1, y1, z1; 
    x1 = x + v.x; 
    y1 = y + v.y; 
    z1 = z + v.z; 
    return Vector(x1, y1, z1); 
} 
  
// Subtract 2 vectors 
Vector Vector::operator-(Vector v) 
{ 
    float x1, y1, z1; 
    x1 = x - v.x; 
    y1 = y - v.y; 
    z1 = z - v.z; 
    return Vector(x1, y1, z1); 
} 
  
// Dot product of 2 vectors 
float Vector::operator^(Vector v) 
{ 
    float x1, y1, z1; 
    x1 = x * v.x; 
    y1 = y * v.y; 
    z1 = z * v.z; 
    return (x1 + y1 + z1); 
} 
  
// Cross product of 2 vectors 
Vector Vector::operator*(Vector v) 
{ 
    float x1, y1, z1; 
    x1 = y * v.z - z * v.y; 
    y1 = z * v.x - x * v.z; 
    z1 = x * v.y - y * v.x; 
    return Vector(x1, y1, z1); 
} 
  
// calculate shortest dist. from point to line 
float shortDistance(Vector line_point1, Vector line_point2, 
                    Vector point) 
{ 
    Vector AB = line_point2 - line_point1; 
    Vector AC = point - line_point1; 
    float area = Vector(AB * AC).magnitude(); 
    float CD = area / AB.magnitude(); 
    return CD; 
} 

=======
>>>>>>> parent of b20f4ec... finished implementation to measure distance between a point and a line defined by two robot's location.
class CLisatLoopFunctions : public CLoopFunctions {

public:

   CLisatLoopFunctions();
   virtual ~CLisatLoopFunctions() {}

   virtual void Init(TConfigurationNode& t_tree);
   virtual void Reset();
   virtual void Destroy();
   //virtual CColor GetFloorColor(const CVector2& c_position_on_plane);
   virtual void PreStep();
    virtual void PostStep();
   virtual void BroadcastRobotFinished();

   virtual bool IsExperimentFinished();
   virtual void PostExperiment();

   virtual float TranslateAngleToPlane(float angle);

private:

   //std::vector<CVector2> m_cFoodPos;
   CFloorEntity* m_pcFloor;
   CRandom::CRNG* m_pcRNG;

   CRange<Real> m_distributeBounds;


   std::string m_strOutput;
   std::ofstream m_cOutput;

   std::string m_strErrorSumOutput;
   std::ofstream m_errorSumOutput;

   int m_robotCount;
   
   int m_finishedRobotsCount;
<<<<<<< HEAD

   Vector m_finalLeaderPosition;
   Vector m_finalFirstFinishedRobotPosition;

   float m_plane_degree_step;
   float m_number_of_planes;
   float m_plane_angles[NUMBER_OF_MICS*2];


};

=======
};

>>>>>>> parent of b20f4ec... finished implementation to measure distance between a point and a line defined by two robot's location.
#endif
