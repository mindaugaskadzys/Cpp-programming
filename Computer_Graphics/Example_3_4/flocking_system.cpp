#include "StdAfx.h"
#include "flocking_system.h"
#include "my_square.h"
#include "my_triangle.h"

flocking_system::flocking_system(float newX, float newY, float newZ, int noOfAgents)
{
	//boid constants
	AGENT_SPEED = 100;
	MOVEMENT_MAGNITUDE = 50;
	ALIGNMENT = 0.8;
	COHESION = 0.4;
	SEPARATION = 0.2;

	float range_of_sight = 70;

	//noOfAgents = 2;

	//list to store the agents in
	agents = new std::vector<flocking_agent*>();

	for(int i = 0; i < noOfAgents; i++) {
		//initialise positions at random offsets from a point
		float xPos = rand() % 600 + newX - 300;
		float yPos = rand() % 600 + newY - 300;

		//initialise the direction of each boid
		float angle = (rand() % 360)*M_PI/180.0;
		float xDir = cos(angle)*AGENT_SPEED;
		float yDir = sin(angle)*AGENT_SPEED;
		my_vector directionVector = my_vector(xDir,yDir,0);

		//flocking_agent* a = new flocking_agent(new my_square(xPos,yPos,newZ,1.0,0),directionVector,70, AGENT_SPEED);
		flocking_agent* a = new flocking_agent(new my_triangle(xPos, yPos, newZ, 5.0, angle), directionVector, range_of_sight, AGENT_SPEED);

		agents->push_back(a);
	}
}


flocking_system::~flocking_system(void)
{
	delete agents;
}

void flocking_system::draw() {
	std::vector<flocking_agent*> agentArray = *agents;

	//draw each of the boids
	for(int i = 0; i < agentArray.size(); i++) {
		agentArray[i]->draw();
	}
}

void flocking_system::updateFlocking(float deltaTime, int w_width, int w_height) {
	std::vector<flocking_agent*> agentArray = *agents;

	for(int i = 0; i < agentArray.size(); i++) {
		flocking_agent* a1 = agentArray[i];
		
		int neighbourCount = 0;

		//init vectors for the different steering forces
		my_vector alignment = my_vector(0,0,0);
		my_vector cohesion = my_vector(0,0,0);
		my_vector separation = my_vector(0,0,0);

		for(int j = 0; j < agentArray.size(); j++) {
			//do not include yourself in the neighbourhood
			if(i == j)
				continue;
			flocking_agent* a2 = agentArray[j];
			my_vector distanceVec = a1->getDistanceVector(a2,w_width,w_height);
			if(distanceVec.length() < a1->radius) {

				//FOV refinement
				float angle = acos(a1->getVelocity().normalise().dotProduct(distanceVec.normalise()))*180.0/M_PI-180;
				if(angle >= -120 && angle <= 120) {
					//compute alignment
					alignment = alignment + a2->getVelocity();

					//compute cohesion
					cohesion = cohesion + a2->getDisplacement();

					//compute separation
					separation = separation + (distanceVec).normalise();

					neighbourCount++;
				}
			}
		}
		//check if our neighbourhood is not empty
		if(neighbourCount != 0) {
			//set the colour to match the flock
			a1->setColour(my_vector(255,255,0));

			//compute alignment
			alignment = (alignment / neighbourCount*1.0) - a1->getVelocity();

			//compute cohesion
			cohesion = cohesion / neighbourCount*1.0;
			cohesion = (cohesion - a1->getDisplacement()).normalise()*AGENT_SPEED - a1->getVelocity();
			//cohesion = (cohesion - a1->getDisplacement());

			//prioritised dithering
			float probability = rand() / (RAND_MAX + 1.);
			if(probability < ALIGNMENT) {
				a1->changeDirection(alignment.normalise()*MOVEMENT_MAGNITUDE,deltaTime);
			}
			else {
				probability = rand() / (RAND_MAX + 1.);
				if(probability < COHESION) {
					a1->changeDirection(cohesion.normalise()*MOVEMENT_MAGNITUDE, deltaTime);
				}
				else {
					probability = rand() / (RAND_MAX + 1.);
					if(probability < SEPARATION) {
						a1->changeDirection(separation.normalise()*MOVEMENT_MAGNITUDE, deltaTime);
					}
					else
						a1->wander(deltaTime);
				}
			}
		}
		//wander - go where you were going
		else {
			//indicate wander behaviour by colour
			a1->setColour(my_vector(0,0,255));
			a1->wander(deltaTime);
		}

		//check screen boundaries
		my_vector displacement = a1->getDisplacement();
		if(displacement.x > w_width)
			a1->setDisplacement(my_vector(0,displacement.y,displacement.z));
		if(displacement.x < 0)
			a1->setDisplacement(my_vector(w_width,displacement.y,displacement.z));
		if(displacement.y > w_height)
			a1->setDisplacement(my_vector(displacement.x,0,displacement.z));
		if(displacement.y < 0)
			a1->setDisplacement(my_vector(displacement.x,w_height,displacement.z));
	}

	
}
