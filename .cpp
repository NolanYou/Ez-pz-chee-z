/*
Strategy: 
For each row of debris, pick a point to go through. 
Try all possiblitites and see which path gives you the least amount of thruster damage and least amount of distance (least amount of damage is prioritized over distance).
*/

#define PENALTY_HIT_SMALL 0.1f
#define PENALTY_HIT_MEDIUM 0.25f
#define PENALTY_HIT_LARGE 0.5f

#define RADIUS_SMALL 0.03f
#define RADIUS_MEDIUM 0.045f
#define RADIUS_LARGE 0.09f

// Sphere radius is 0.111 but the position of the sphere is +/- 0.05 so it is added as a buffer
#define SPHERE_RAD 0.111f

#define INCREMENT 0.05f
#define COLLISION_INCREMENT 0.05f
#define TOLERANCE 0.05f
#define COLLISION_TOLERANCE 0.05f
#define LOWER -0.7f
#define UPPER 0.7f

float path[5][3];

int pos;

void init(){
	//This function is called once when your code is first loaded.

	//IMPORTANT: make sure to set any variables that need an initial value.
	//Do not assume variables will be set to 0 automatically!
	
    // The point we are travelling to next
	pos = 1;


    float debrisInfo[11][4];
	game.getDebris(debrisInfo);
	
    // The maximum end thruster speed of the paths so far
	float maxSpeed = 0.0f;
    // The minimum distance required given the maximum thruster speed (thruster speed is valued more over distance)
	float distMin = 100000.0f;
	
    // Loop through all possible points in each row of debris
	for(float i = LOWER; i < UPPER; i += INCREMENT){
	    for(float j = LOWER; j < UPPER; j += INCREMENT){
	        for(float k = LOWER; k < UPPER; k += INCREMENT){
	            // The path we are testing
	            float testPath[5][3] = {{0.0f, 0.75f, 0.0f}, {k, 0.5f, 0.0f}, {j, 0.3f, 0.0f}, {i, 0.1f, 0.0f}, {0.0f, -0.2f, 0.0f}};
	            // Which debris did we hit?
	            bool hit[11] = {false}; 
	            // Loop through each point on the path and debris
	            for(int pathPoint=0; pathPoint<=4; pathPoint++){
    	            for(int debrisNum=0; debrisNum<11; debrisNum++){
    	                float debrisPoint[3] = {debrisInfo[debrisNum][0], debrisInfo[debrisNum][1], debrisInfo[debrisNum][2]};
    	                // Check if the path from the current point to the next will hit a debris
    	                if(collidePath(testPath[pathPoint], testPath[pathPoint+1], SPHERE_RAD, debrisPoint, debrisInfo[debrisNum][3])){
    	                    hit[debrisNum] = true;
    	                }
    	            }
	            }
	            // Total the penalties to see what the end thruster speed is
	            float speed = 1.0f;
	            for(int l=0; l<11; l++){
	                if(hit[l]) speed-=radToDamage(debrisInfo[l][3]);
	            }
	            // Get the distance traveled
	            float dist = distancePath(testPath, 5);
	            // check if this thruster speed is the best and if is the same then if the distance is the least
	            if(speed > maxSpeed || (abs(speed - maxSpeed) < 0.001 && dist < distMin)){
	                maxSpeed = speed;
	                distMin = dist;
	                for(int i=0; i<5; i++) memcpy(path[i], testPath[i], sizeof(path[i]));
	            }
	        }
	    }   
	}
	DEBUG(("%f", maxSpeed));
	DEBUG(("%f", distMin));
	for(int i=0; i<4; i++){
	    DEBUG(("%f %f %f", path[i][0], path[i][1], path[i][2]));
	}
}

// Distance a path travels
float distancePath(float testPath[][3], int n){
    float distanceTotal = 0;
    for(int i=0; i<n-1; i++){
        distanceTotal += distance(testPath[i], testPath[i+1]);
    }
    return distanceTotal;
}

// Convert radius to damage
float radToDamage(float f){
    if(f==RADIUS_SMALL)
        return PENALTY_HIT_SMALL;
    if(f==RADIUS_MEDIUM)
        return PENALTY_HIT_MEDIUM;
    if(f==RADIUS_LARGE)
        return PENALTY_HIT_LARGE;
    return 1.0f;
}

float distance(float point1[3], float point2[3]){
    float distanceVec[3];
    mathVecSubtract(distanceVec, point1, point2, 3);
    float distance = mathVecMagnitude(distanceVec, 3);
    return distance;
}

// Checks if SPHERE will collide with debris from point 1 to point 2
bool collidePath(float pathPoint1[3], float pathPoint2[3], float radius, float debrisPoint[3], float radiusDebris){
    // TODO: Directly check cylinder and sphere collision
    float direction[3];
    mathVecSubtract(direction, pathPoint2, pathPoint1, 3);
    mathVecNormalize(direction, 3);
    for(int i=0; i<3; i++) direction[i] *= COLLISION_INCREMENT;
    
    // Keep checking until you reach point2 (note: point2 y will always be less than point1 y, so we compare y)
    float pathPoint1Temp[3];
    memcpy(pathPoint1Temp, pathPoint1, sizeof(pathPoint1Temp));
    while(pathPoint1Temp[1] > pathPoint2[1]){
        if(collideSphere(radius, pathPoint1Temp, radiusDebris, debrisPoint)) return true;
        mathVecAdd(pathPoint1Temp, pathPoint1Temp, direction, 3);
    }
    return false;
}

// Checks if SPHERE will collide with debris at a point
bool collideSphere(float radius1, float point1[3], float radius2, float point2[3]){
    return (distance(point1, point2)) < (radius1 + radius2 + COLLISION_TOLERANCE);
}

float moveTo(float movement[3], float myState[3]) {

    // Direction vector current state to movement
    float vecBet[3];
    mathVecSubtract(vecBet, movement, myState,3);
    
    float speed = 0.5f;
    // float dx = vecBet[0]*vecBet[0] + vecBet[1]*vecBet[1];
    // if (sqrtf(dx) > .3) {
    //     scalar = 1.01;
    // }
    // else {
    //     scalar = 1.45;
    // }
    
    for(int i=0; i<3; i++) vecBet[i] *= speed;
    // mathVecNormalize(vecBet, 3);    

    float distance = mathVecMagnitude(vecBet,3);
    if (distance > .1) {
        // vecBet[2] -= .068;
        api.setVelocityTarget(vecBet);
    }
    else {
        api.setPositionTarget(movement);
    }
    
    return distance;

}

void nextPosition(float position[3], float target[12], float t) {
    for(int i=0; i<3; i++) position[i] += target[i] + t * target[i+3] + 0.5*t*t*target[i+6];
}

void loop(){
	//This function is called once per second.  Use it to control the satellite.
	float state[12];
	api.getMyZRState(state);
	float position[3] = {state[0], state[1], state[2]};
	if(pos<5 && distance(position, path[pos]) < TOLERANCE){
	    pos++;
	}
	if(pos<5){
	    float attitude[3] = {0, -1, 0};
	    api.setPositionTarget(path[pos]);
        api.setAttitudeTarget(attitude);
	}else{
	    
	    float targetState[12];
	    api.getOtherZRState(targetState);
	    
	    // method 1, predict where the target is going
	    float targetPosition[3];
	    nextPosition(targetPosition, targetState, 1.0f);
	    
	    float attitude[3];
	    mathVecSubtract(attitude, targetPosition, position, 3);
        mathVecNormalize(attitude, 3);
        api.setAttitudeTarget(attitude);
        
	    api.setPositionTarget(path[4]);
	   // api.setVelocityTarget(velocity);
	    game.completeRendezvous();
	}
}

