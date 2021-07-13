/*
This program reads in a csv file with each row containing information on a place name, its type (town or city), its population,
its latitude (degrees) and its longitude (degrees).

A company needs to make deliveries to the places listed in the csv file, and wants to know where the optimal position to place their hub is.

We can find this position by optimising a cost function we can define. This will lead to us finding the optimal coordinates (lat, long)
for the hub. 

We assume that the cost of travelling from A to B is well represented by the great-circle distance between those points. This distance can be
found using the Haversine formula. We also assume that the hub will only ever go out and back from the hub to a place. Finally, we assume that
the company only wants to or can only afford one hub.

After using those assumptions, I decided to change the second assumption and instead assume that the delivery can be conducted from location
to location instead of only from the hub. I displayed the new optimal values for this case after that for the first case

Also attempted to extend the number of hubs assumption to allow for two hubs. The new optimal values were displayed for this case.
It appears that the number of function evaluations requird to find the optimal positions of two hubs approximately halved, meaning that
it's quicker to find the optimal positions of two hubs rather than one hub.
*/

#include "pch.h"
#include <iostream>
#include <fstream>
#include <cmath>
#include <ctime>
#include <algorithm>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>
using namespace std;

#define PI 3.141592653589793
#define R 3958.75


// function defined to calculate the great-circle distance between two points using the Haversine formula
// the coordinates are in the form (latitude, longitude)
// we assume that the raw data for position is in degrees, so the function converts from degrees to radians
double calc_distance(double lat1, double long1, double lat2, double long2)
{

	double diffLat = lat2 - lat1;

	diffLat = diffLat * PI / 180;

	double diffLong = long2 - long1;

	diffLong = diffLong * PI / 180;

	double a = pow(sin(diffLat / 2), 2) + cos(lat1 * PI / 180)*cos(lat2 * PI / 180)*pow(sin(diffLong / 2), 2);

	double c = 2 * atan2(sqrt(a), sqrt(1 - a));

	return R * c;
}

// struct created to easily map the column variables to their correct data types
// also helps for clarity when reading the code
struct Columns
{
	string place;
	string type;
	double population;
	double latitude;
	double longitude;
};


int func_evals; // counts the number of function evaluations

// cost function defined for calculating the sum of the distances between specified points and then taking the reciprocal of that sum
// the value returned will be optimised by using hill climb
double cost_function(double x_cost, double y_cost, vector<Columns> &dataVector, bool improve)
{
	// function to return the 'value' of thing being optimized
	double sum_dist = 0;
	if (!improve)
	{
		for (auto const &row : dataVector)
		{
			sum_dist += calc_distance(row.latitude, row.longitude, x_cost, y_cost);
		}
	}
	else
	{
		double x1 = x_cost;
		double y1 = y_cost;
		double x2, y2;
		for (auto const &row : dataVector)
		{
			x2 = row.latitude;
			y2 = row.longitude;
			sum_dist += calc_distance(x1, y1, x2, y2);
			x1 = row.latitude;
			y1 = row.longitude;
		}
	}
	// calculates the sum of the Great Circle distances from point x,y to each place
	double f;
	f = 1 / sum_dist;
	func_evals++;
	return f;
}

// new cost function defined for the case where can use two hubs for delivery starting points
// using the calc_distance function, this function finds which hub is the closest to the place of interest, and then uses that hub as the 
// starting point when delivering to that place
// this should reduce the sum of distances 
double cost_function_2hubs(double hub1_x_cost, double hub1_y_cost, double hub2_x_cost, double hub2_y_cost, vector<Columns> &dataVector)
{
	// function to return the 'value' of thing being optimized
	double sum_dist = 0;
	for (auto const &row : dataVector)
	{
		if (calc_distance(row.latitude, row.longitude, hub1_x_cost, hub1_y_cost) < calc_distance(row.latitude, row.longitude, hub2_x_cost, hub2_y_cost))
		{
			sum_dist += calc_distance(row.latitude, row.longitude, hub1_x_cost, hub1_y_cost);
		}
		else
		{
			sum_dist += calc_distance(row.latitude, row.longitude, hub2_x_cost, hub2_y_cost);
		}
	}
	double f;
	f = 1 / sum_dist;
	func_evals++;
	return f;
}


// function defined for finding the minimum and maximum coordinates in the dataset
double min_max_finder(double min_x, double min_y, double max_x, double max_y, vector<Columns> &dataVector)
{
	for (auto const &row : dataVector) {
		if (row.latitude < min_x) min_x = row.latitude;
		if (row.latitude > max_x) max_x = row.latitude;

		if (row.longitude < min_y) min_y = row.longitude;
		if (row.longitude > max_y) max_y = row.longitude;
	}
	return min_x, min_y, max_x, max_y;
}

//function defined for generating a random number between two limits, 'lower' and 'upper'
double random_number_generator(double upper, double lower, int n) {
	double r;
	r = lower + (rand() % (n + 1) * (1. / n) * (upper - lower));
	return r;
}


int main()
{
	vector<Columns> data; // vector of structures which the data will be read into
	double x, y; // variables which will hold the updated and optimal values of the coordinates
	double step = 0.01; // step size for moving during the hill climb
	double value, oldValue, newValue, maxValue; 
	int iter = 0; // counts the number of iterations
	int dx, dy;

	srand(time(NULL)); // seed for random number generator

	ifstream dataFile("GBplaces.csv");

	if (dataFile.is_open())
	{
		// while it is not the end of file
		while (!dataFile.eof())
		{
			// local variables created for the intermediate step of reading the data into a vector of struct variables
			// row is a variable of the Columns struct
			// assumed a fixed size of 5 elements for dummy_array since we can assume we know the general structure of the csv files this program is made for

			string line;
			Columns row;
			string tempArray[5];

			// read in a line from the csv file
			getline(dataFile, line, '\n');

			// the line is discarded by moving on to the next iteration of the loop if the line starts with '%' or is empty
			// this helps us to deal with missing data and remove the header row
			// "%" is a const char pointer, meaning we use '*' to get the value of the character
			if (line == "" || line[0] == *"%") continue;

			
			std::istringstream ss(line);
			for (int i = 0; i < 5; i++) {
				getline(ss, tempArray[i], ',');
			}

			row.place = tempArray[0];  // the first member of the array is the name of the place
			row.type = tempArray[1];  // the second member of the array is the type of the place

			// the third member of the array is the population of the place and we convert it to an integer using stoi
			row.population = stoi(tempArray[2]);

			// the fourth member of the array is the latitude of the place and we convert it to a double using atof
			// but first we make it a c string so atof can convert it
			row.latitude = atof(tempArray[3].c_str());

			// the fifth member of the array is the longitude of the place and we convert it to a double using atof
			// but first we make it a c string so atof can convert it
			row.longitude = atof(tempArray[4].c_str());

			data.push_back(row);
		}
		// file no longer needed, so it is closed
		dataFile.close();
	}
	else
	{
		// if the file cannot be opened, an error message is displayed and then the program is terminated
		cout << "File does not exist or could not be opened" << endl;
		exit(1);
	}

	double min_lat = 0;
	double min_long = 0;
	double max_lat = 0;
	double max_long = 0;
	// the minimum and maximum coordinates are returned by the min_max_finder function
	min_lat, min_long, max_lat, max_long = min_max_finder(min_lat, min_long, max_lat, max_long, data);
	
	// intial hub position is randomly generated using the min-max range we calculated
	x = random_number_generator(max_lat, min_lat, 100);
	y = random_number_generator(max_long, min_long, 100);

	// intial value is calculated for initial hub position
	value = cost_function(x, y, data, false);

	// main loop to continually optimise x, y using the hill climb method
	do {

		// assign the current value 
		oldValue = value;
		maxValue = oldValue; // set the maxValue for the local search to be the current value

		// now look around the current point to see if there's a better one nearby
		for (int i = -1; i <= 1; i++) {
			for (int j = -1; j <= 1; j++) {
				// this gives 9 points including the current point (when i=0, j=0)
				if (i == 0 && j == 0) {
					// so maybe you want to miss that one and save some function evaluations
				}
				else {
					newValue = cost_function(x + step * i, y + step * j, data, false); // value at neighbouring point
					if (newValue >= maxValue) { // is it bigger than maxValue?
					  // yes so set maxValue and save point i,j values
						dx = i;
						dy = j;
						maxValue = newValue;
					}
				}
			}
		}

		// update x and y to new optimal values where the value is greater
		x += step * dx;
		y += step * dy;
		value = maxValue;

		iter++; // add one to the iteration counter

	} while (value > oldValue); // repeat all this while we can find a more optimal position


	// position, sum of distances and number of function evaluations are all displayed for the user of the program
	cout << "For the case of one hub, where the delivery is from hub to location for every location, the following is found:" << endl;
	cout << "Function evaluations: " << func_evals << endl;
	cout << "The latitude of the hub is optimal at: " << x << endl << "The longitude of the hub is optimal at: " << y << endl;
	cout << "The sum of distances to the hub is " << 1 / value << "miles." << endl;

	// the hill climb is repeated, but this time in the case where the delivery is conducted from location to location
    // rather than going back to the hub every time
	// to do this, the boolean argument in the cost_function is simply changed to 'true'
	iter = 0;
	
	x = random_number_generator(max_lat, min_lat, 100);
	y = random_number_generator(max_long, min_long, 100);

	value = cost_function(x, y, data, true);

	func_evals = 0;
	
	do {

		oldValue = value;
		maxValue = oldValue; 

		for (int i = -1; i <= 1; i++) {
			for (int j = -1; j <= 1; j++) {
				
				if (i == 0 && j == 0) {
					
				}
				else {
					newValue = cost_function(x + step * i, y + step * j, data, true); 
					if (newValue >= maxValue) {
						dx = i;
						dy = j;
						maxValue = newValue;
					}
				}
			}
		}

		x += step * dx;
		y += step * dy;
		value = maxValue;

		iter++;

	} while (value > oldValue); 

	cout << endl;
	cout << "For the case of one hub, where the delivery is from location to location rather than hub to location, the following is found:" << endl;
	cout << "Function evaluations: " << func_evals << endl;
	cout << "The latitude of the hub is optimal at: " << x << endl << "The longitude of the hub is optimal at: " << y << endl;
	cout << "The sum of distances from location to location is " << 1 / value << "miles." << endl;


	// attempted to use 2 hubs

	double hub1_x = random_number_generator(max_lat, min_lat, 100);
	double hub1_y = random_number_generator(max_long, min_long, 100);
	double hub2_x = random_number_generator(max_lat + 10, min_lat -10, 100);
	double hub2_y = random_number_generator(max_long + 0.5, min_long - 0.5, 100);


	value = cost_function_2hubs(hub1_x, hub1_y, hub2_x, hub2_y, data);

	func_evals = 0;
	// two different step values assigned, one for each hub
	// an attempt to get the hubs to move around more independently of each other during iterations
	double step1 = 0.01;
	double step2 = 0.02;

	do {

		oldValue = value;
		maxValue = oldValue;

		for (int i = -1; i <= 1; i++) {
			for (int j = -1; j <= 1; j++) {

				if (i == 0 && j == 0) {

				}
				else {
					newValue = cost_function_2hubs(hub1_x + step1 * i, hub1_y + step1 * j, hub2_x + step2 * i, hub2_y + step2 * j, data);
					if (newValue >= maxValue) {
						dx = i;
						dy = j;
						maxValue = newValue;
					}
				}
			}
		}

		hub1_x += step1 * dx;
		hub1_y += step1 * dy;
		hub2_x += step2 * dx;
		hub2_y += step2 * dy;
		value = maxValue;

		iter++;

	} while (value > oldValue);

	cout << endl;
	cout << "For the case where we have two hubs to make deliveries from, the following optimal values were found" << endl;
	cout << "Function evaluations: " << func_evals << endl;
	cout << "Hub 1: " << endl << "Latitude: " << hub1_x << endl << "Longitude " << hub1_y << endl;
	cout << "Hub 2: " << endl << "Latitude: " << hub2_x << endl << "Longitude " << hub2_y << endl;
	cout << "The sum of distances from the hubs to their closest places is: " << 1 / value << "miles." << endl;

	
	return 0;
}