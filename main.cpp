// 11 Jan 2016
// File Parser
// Read a file line by line and word by word
#include <iostream>
#include <string>
#include <fstream> // Reading a file Word by Word.
#include <vector>
#include <map>
#include <set>
#include <sstream>
#include <cassert>
#include <cctype> // int isdigit ( int c );



////////////////////////////////////////////// Utility Functions  Declaration //////////////////////////////////////////////////////////////

std::string decimalToFraction(float fraction);
bool isNumber(const std::string& s);
template <typename T> std::string NumberToString (T num);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

typedef enum raceCardState_t {
	RC_TIME_RACECOURSENAME_STATE = 0,
	RC_GOING_RACENAME_STATE,
	RC_HORSE_NO_STATE,
	RC_DRAW_NO_STATE,
	RC_HORSE_NAME_STATE,
	RC_JOCKEY_TRAINER_NAME_STATE,
	RC_FORM_STATE,
	RC_AGE_WEIGHT_STATE,
	RC_RATING_STATE,
	RC_ODDS_STATE,
	RC_NEXT_HORSE_RECORD_STATE,
	RC_NEXT_RACE_STATE,
	RC_END_STATE,
	//RC_RACING_HORSE_INFO_STATE,
}raceCardState_t;

typedef struct raceInfo_t {
	std::string time;
	std::string raceCourse;
	std::string raceName;
	std::string going;
	std::string distance; // 1m 4f
	std::string raceClass;
}raceInfo_t;

typedef struct horse_t {
	horse_t()
	{
		horseNo = drawNo = rating = 0;
	}
	unsigned int horseNo;
	unsigned int drawNo;
	std::string horseName;
	std::string jockey;
	std::string trainer;
	std::string form;
	unsigned int age;
	std::string weight;
	unsigned int rating;
	std::string odds;
}horse_t;



typedef enum retValue_t {
	RET_NOT_FOUND,
	RET_CURT_FOUND, // Current Value Found
	RET_NEXT_FOUND, // Next Value Found
	RET_THIRD_FOUND,
}retValue_t;


std::vector<std::string> wordVector;
raceInfo_t raceInfo;
horse_t horse;
std::vector<horse_t> horseVector;
std::set<std::string> skipStringList;


void parseLine(std::string line)
{
	std::string delimiter ="\t=, "; // Multiple Delimiter - 1. Tabs Space 2. Equal to 3. Comma and 4. Space
	std::size_t prev = 0, pos;
	while ((pos = line.find_first_of(delimiter, prev)) != std::string::npos)
	{
		if (pos > prev)
			wordVector.push_back(line.substr(prev, pos-prev));
		prev = pos+1;
	}
	if (prev < line.length())
		wordVector.push_back(line.substr(prev, std::string::npos));
}

// 2:00 Kempton
bool find_Time_RaceCourse() // 2:00
{
	for(unsigned int i =0; i<wordVector.size(); ++i)
	{
		// Check for substring(yrs) in a string (BL,T 10yrs-9st-4lb)
		if (wordVector[i].find(":") != std::string::npos)
		//if(wordVector[i][1]==':' || wordVector[i][2]==':')
		{
			raceInfo.time = wordVector[i];
			raceInfo.raceCourse = wordVector[i+1];
			return true;
		}
	}
	return false;
}

// Handicap (Class 4) 1m 7f 169y Going: Heavy
// (Class 5) 1m 4f Going: Standard
bool find_Going()
{
	for(unsigned int i = 0; i < wordVector.size(); ++i)
	{
		if(wordVector[i]== "Going:")
		{
			raceInfo.going = wordVector[i+1];

			if(wordVector[i-1][wordVector[i-1].size() -1]=='y')
			{
				raceInfo.distance = wordVector[i-3] + " " + wordVector[i-2] + " " + wordVector[i-1]; // 1m 7f 169y
				i = i-3;
			}
			else if(wordVector[i-1][wordVector[i-1].size() -1]=='f')
			{
				raceInfo.distance = wordVector[i-2] + " " + wordVector[i-1]; // 1m 4f
				i = i-2;
			}
			else
			{
				raceInfo.distance = wordVector[i-1]; // 2m
				i = i-1;
			}

			raceInfo.raceClass = wordVector[i-1]; i = i-1;
			// Find and Replace ')'
			raceInfo.raceClass.replace(raceInfo.raceClass.find(")"),raceInfo.raceClass.length(),""); // 5) => 5
			if(i>1) raceInfo.raceName = wordVector[0];
			else raceInfo.raceName = "Unknown";

			return true;
		}
	}
	return false;
}

retValue_t  find_HorseNo()
{
	horse.horseNo = 0;
	if(wordVector.size() == 1)
	{
		if(isNumber(wordVector[0]))
		{
			horse.horseNo = atoi(wordVector[0].c_str());
			 return RET_CURT_FOUND;
		}
	}
	return RET_NOT_FOUND;
}

retValue_t  find_DrawNo()
{
	horse.drawNo = 0;
	if(wordVector.size() == 1)
	{
		if(wordVector[0][0] == '(')
		{
			// single digit character
			if(wordVector[0][2] == ')')
			{
				horse.drawNo = wordVector[0][1] - '0'; // Number char into Int; DO NOT USE atoi
				return RET_CURT_FOUND;
			}
			else
			{ // Double Digit character
				horse.drawNo = (wordVector[0][1] - '0')*10 + (wordVector[0][2] - '0');
				return RET_CURT_FOUND;
			}
		}
	}
	return RET_NEXT_FOUND;
}

std::string find_HorseName()
{
	std::string horseName = ""; // empty string
	for(unsigned int i = 0; i < wordVector.size(); ++i)
	{
		horseName = horseName + wordVector[i];
		if(i+1 < wordVector.size()) horseName = horseName + " ";
	}
	return horseName;
}

// Global Function: String Find and Split (into 2 parts)
std::pair <std::string,std::string> findSplitString(std::string inString, std::string splitIdentifier)
{
	std::pair <std::string,std::string> twoPartString;
	size_t pos;
    if ((pos = inString.find(splitIdentifier)) != std::string::npos)
	{
		twoPartString.first = inString.substr(0,pos);
		twoPartString.second = inString.substr(pos+splitIdentifier.length(), std::string::npos);
	}
	return twoPartString;
}

std::pair <std::string,std::string> find_JockyTrainerName()
{
	std::pair <std::string,std::string> jockeyTrainer;
	bool isJockeyFound = false;
	for(unsigned int i = 0; i < wordVector.size(); ++i)
	{
		if(!isJockeyFound && wordVector[i].at(0) != '/')
		{
		  jockeyTrainer.first = jockeyTrainer.first + wordVector[i]+" ";
		}
		else
		{
			isJockeyFound = true;
			if(wordVector[i] != "/") jockeyTrainer.second = jockeyTrainer.second + wordVector[i]+" ";
		}
	}
	// Remove the ending space
	jockeyTrainer.first.pop_back();
	jockeyTrainer.second.pop_back();

	return jockeyTrainer;
}



// BL,T 10yrs-9st-4lb
retValue_t find_ageWeight()
{
	for(unsigned int i = 0; i < wordVector.size(); ++i)
	{
		// Check for substring(yrs) in a string (BL,T 10yrs-9st-4lb)
		if (wordVector[i].find("yrs") != std::string::npos)
		{
			std::pair <std::string,std::string> ageWeight;
			ageWeight = findSplitString(wordVector[i],"yrs-"); // 10yrs-9st-4lb
			horse.age = atoi(ageWeight.first.c_str());  
			horse.weight = ageWeight.second;
			return RET_CURT_FOUND;
		}
	}
	return RET_NOT_FOUND;
}

// Optional
// 47322-
// 4-2
retValue_t  find_Form()
{
	for(unsigned int i = 0; i < wordVector.size(); ++i)
	{
		// Check skip string is found or not in SET elements
		if(skipStringList.find(wordVector[i]) == skipStringList.end()) // skip string is not found
		{
			// Check for substring(yrs) in a string (BL,T 10yrs-9st-4lb)
			if (wordVector[i].find("yrs") == std::string::npos) // yrs - is not found
			{
				horse.form = wordVector[i]; // 170865-
				return RET_CURT_FOUND;
			}
			else
			{
				horse.form = "Unknown";
				// 10yrs-9st-4lb
				if(RET_CURT_FOUND == find_ageWeight()) return RET_NEXT_FOUND;
				return RET_NOT_FOUND; 				
			}

		}
	}
	return RET_NOT_FOUND;
}

// Optional
/*
Odds
6/4 EVS 2.46
6/4 
*/
retValue_t find_odds()
{
	for(unsigned int i = 0; i < wordVector.size(); ++i)
	{
		// Check for substring(/) in a string (11/8)
		if ((wordVector[i].find("/") != std::string::npos) || (wordVector[i].find(".") != std::string::npos)|| (wordVector[i].find("EVS") != std::string::npos) || (wordVector[i].find("SP") != std::string::npos))
		{
			horse.odds = horse.odds + wordVector[i] + " "; // 11/8
			if(wordVector[i].find(".") != std::string::npos) return RET_CURT_FOUND; // found the last decimal odd			
		}
		else if(!horse.odds.empty()&& wordVector.size() == 1 && isNumber(wordVector[0])) // decimal odd is not found
		{
			horse.horseNo =  atoi(wordVector[0].c_str());
			return RET_NEXT_FOUND;
		}

	}
	return RET_NOT_FOUND;
}

// Optional 
retValue_t find_Rating()
{	
	
	if(wordVector.size() == 1 && isNumber(wordVector[0]) && wordVector[0].find("/") == std::string::npos && horse.odds.empty()) 
	{
		horse.rating = atoi(wordVector[0].c_str());
		return RET_CURT_FOUND; // Rating Found
	}
	else
	{		
		switch(find_odds())
		{
			case RET_CURT_FOUND: 
				return RET_NEXT_FOUND; // Odd found
				break;
			case RET_NEXT_FOUND:
				return RET_THIRD_FOUND; // Horse No found
				break;
			default:
				return  RET_NOT_FOUND;
		}		
	}	
}


bool is_NextHorseExist()
{
	if (wordVector[0].find("Favourite") != std::string::npos) 
		return false;
	else if(horse.horseNo != 0) 
	{
		//find_DrawNo();
		return true;
	}
	else if(RET_CURT_FOUND == find_HorseNo()) return true;	
	else return false;	 
}


void populateSkipStrings()
{
	std::string skipStrings[] = {"Spotlight", "Virtual", "Paddock", "jockey"};
	for(unsigned int i =0; i < sizeof skipStrings / sizeof skipStrings[0] ; ++i) skipStringList.insert(skipStrings[i]);
}

// Optional Fields:  Draw, Form, Rating, Prev. odds, sp

int main()
{
	// decimalToFraction();
	std::ifstream infile("textFile.txt");
	std::ofstream outfile("outfile.csv");
	std::string line;
	bool isNewRaceFound = false;
	bool isNewGoingFound = false;
	retValue_t ret;

	populateSkipStrings();

	raceCardState_t raceCardState = RC_TIME_RACECOURSENAME_STATE;
	std::pair <std::string,std::string> jockeyTrainer;

	while (std::getline(infile, line))
	{
		if (line.empty()) continue;

		ret = RET_NOT_FOUND;
		wordVector.clear();
		parseLine(line);		
		////////////////////////////////////////////////////////

		switch(raceCardState)
		{
		case RC_TIME_RACECOURSENAME_STATE:
			horse = horse_t(); // reset
			raceInfo = raceInfo_t();
			if(find_Time_RaceCourse())
			{
				raceCardState = RC_GOING_RACENAME_STATE;	
			}
			break;
		case RC_GOING_RACENAME_STATE:
			if(find_Going())
			{
				raceCardState = RC_HORSE_NO_STATE;				
			}
			break;
		case RC_HORSE_NO_STATE:			
			ret  = find_HorseNo();
			if(RET_NOT_FOUND != ret)  			
			{
				raceCardState = RC_DRAW_NO_STATE;				
			}			
			break;
		case RC_DRAW_NO_STATE: // Optional
			ret  = find_DrawNo();
			if(horse.drawNo || RET_NOT_FOUND != ret)
			{
				raceCardState = RC_HORSE_NAME_STATE;
				// Time, Race Course
				outfile << "\n";
				outfile << raceInfo.time;
				outfile << ",";
				outfile << raceInfo.raceCourse;
				outfile << ",";
				// Race Name, Class, Distance, Going
				outfile << raceInfo.raceName;
				outfile << ",";
				outfile << raceInfo.raceClass;
				outfile << ",";
				outfile << raceInfo.distance;
				outfile << ",";
				outfile << raceInfo.going;
				outfile << ",";
				// Horse Number				
				outfile << horse.horseNo;
				outfile << ",";
				// Draw Number
				outfile << horse.drawNo;
				outfile << ",";
				horse.drawNo = 0;
			}
			break;
		case RC_HORSE_NAME_STATE:
			horse.horseName = find_HorseName();
			if(!horse.horseName.empty()) // Check for empty string
			{
				raceCardState = RC_JOCKEY_TRAINER_NAME_STATE;
				outfile << horse.horseName;
				outfile << ",";
			}
			break;
		case RC_JOCKEY_TRAINER_NAME_STATE:
			jockeyTrainer = find_JockyTrainerName();
			horse.jockey = jockeyTrainer.first;
			horse.trainer = jockeyTrainer.second;
			if(!horse.jockey.empty()) // Check for empty string
			{
				raceCardState = RC_FORM_STATE;
				outfile << horse.jockey;
				outfile << ",";
				outfile << horse.trainer;
				outfile << ",";
			}
			break;
		case RC_FORM_STATE: // Optional
			ret  = find_Form();
			if(RET_NOT_FOUND != ret)  
			{
				if(RET_CURT_FOUND == ret) 
				{
					raceCardState = RC_AGE_WEIGHT_STATE; 
					outfile << horse.form;
					outfile << ",";
				}
				else if(RET_NEXT_FOUND == ret) 
				{
					raceCardState = RC_RATING_STATE;	
					outfile << horse.form;
					outfile << ",";
					outfile << horse.age;
				    outfile << ",";	
					outfile << horse.weight;
				    outfile << ",";				
				}				
			}
			break;
		case RC_AGE_WEIGHT_STATE:
			horse.horseNo = 0;
			ret  = find_ageWeight();
			if(RET_NOT_FOUND != ret) 
			{
				raceCardState = RC_RATING_STATE;
				outfile << horse.age;
				outfile << ",";
				outfile << horse.weight;
				outfile << ",";
			}
			break;
		case RC_RATING_STATE: // Optional		 	
			horse.rating = 0;
			ret  = find_Rating();
			if(RET_NOT_FOUND != ret) // Check for empty string
			{
				if(RET_THIRD_FOUND == ret) // Horse No found
				{
					raceCardState = RC_DRAW_NO_STATE; 
					outfile << horse.rating;
				    outfile << ",";
					outfile << horse.odds;
					outfile << ",";	
					horse.odds.clear();
				}
				else if(RET_NEXT_FOUND == ret) // Odds found
				{
					raceCardState = RC_ODDS_STATE;
					outfile << horse.rating;
					outfile << ",";
				}
				else if(RET_CURT_FOUND == ret) // Rank found
				{
					raceCardState = RC_ODDS_STATE;
					outfile << horse.rating;
					outfile << ",";
				}
			}
			break;
		case RC_ODDS_STATE:
			ret  = find_odds();
			if(RET_NOT_FOUND != ret)
			{	
				if(RET_CURT_FOUND == ret) raceCardState = RC_NEXT_HORSE_RECORD_STATE; 
				else if(RET_NEXT_FOUND == ret) raceCardState = RC_DRAW_NO_STATE;

				outfile << horse.odds;
				outfile << ",";	
				horse.odds.clear();
			}
			break;
		case RC_NEXT_HORSE_RECORD_STATE:
			if(is_NextHorseExist()) raceCardState = RC_DRAW_NO_STATE;
			else raceCardState = RC_TIME_RACECOURSENAME_STATE; //RC_NEXT_RACE_STATE;			
			break;
		case RC_NEXT_RACE_STATE:
			outfile <<"\n\n-------------------------------------------------------------------------\n";
		    raceCardState = RC_TIME_RACECOURSENAME_STATE;			
			break;
		case RC_END_STATE:
			outfile <<"\n\n-------------------------------------------------------------------------\n";
			//raceCardState = RC_TIME_RACECOURSENAME_STATE;
			//outfile.close();
			break;

		default:
			std::cout<<"Unhandled Race Card State";
		}		
	}
	outfile.close();
	return 0;
}




////////////////////////////////////////////// Utility Functions  //////////////////////////////////////////////////////////////
// Check string is a number or not before converting to integer !
bool isNumber(const std::string& s)
{
	std::string::const_iterator it = s.begin();
	while (it != s.end() && std::isdigit(*it)) ++it;
	return !s.empty() && it == s.end();
}

// Number to String
template <typename T>
std::string NumberToString (T num)
{
    std::ostringstream oss;
    oss<<num;
    return oss.str();
}

// Convert Fraction into Odd
std::string decimalToFraction(float fraction)
{
	// Odds Conversion Table
	std::map<float,std::string> oddFraction;
	oddFraction.insert(std::make_pair(1.10, "1/10"));
	oddFraction.insert(std::make_pair(1.11, "1/9"));
	oddFraction.insert(std::make_pair(1.12, "1/8"));
	oddFraction.insert(std::make_pair(1.14, "1/7"));
	oddFraction.insert(std::make_pair(1.17, "1/6"));
	oddFraction.insert(std::make_pair(1.20, "1/5"));
	oddFraction.insert(std::make_pair(1.22, "2/9"));
	oddFraction.insert(std::make_pair(1.25, "1/4"));
	oddFraction.insert(std::make_pair(1.29, "2/7"));
	oddFraction.insert(std::make_pair(1.30, "3/10"));
	oddFraction.insert(std::make_pair(1.33, "1/3"));
	oddFraction.insert(std::make_pair(1.36, "4/11"));
	oddFraction.insert(std::make_pair(1.40, "2/5"));
	oddFraction.insert(std::make_pair(1.44, "4/9"));
	oddFraction.insert(std::make_pair(1.50, "1/2"));
	oddFraction.insert(std::make_pair(1.53, "8/15"));
	oddFraction.insert(std::make_pair(1.57, "4/7"));
	oddFraction.insert(std::make_pair(1.62, "8/13"));
	oddFraction.insert(std::make_pair(1.67, "4/6"));
	oddFraction.insert(std::make_pair(1.73, "8/11"));
	oddFraction.insert(std::make_pair(1.80, "4/5"));
	oddFraction.insert(std::make_pair(1.83, "5/6"));
	oddFraction.insert(std::make_pair(1.91, "10/11"));
	oddFraction.insert(std::make_pair(2.00, "1/1"));
	oddFraction.insert(std::make_pair(2.10, "11/10"));
	oddFraction.insert(std::make_pair(2.20, "6/5"));
	oddFraction.insert(std::make_pair(2.25, "5/4"));
	oddFraction.insert(std::make_pair(2.38, "11/8"));
	oddFraction.insert(std::make_pair(2.50, "6/4"));
	oddFraction.insert(std::make_pair(2.63, "13/8"));
	oddFraction.insert(std::make_pair(2.75, "7/4"));
	oddFraction.insert(std::make_pair(2.80, "9/5"));
	oddFraction.insert(std::make_pair(2.86, "15/8"));
	oddFraction.insert(std::make_pair(3.00, "2/1"));
	oddFraction.insert(std::make_pair(3.12, "85/40"));
	oddFraction.insert(std::make_pair(3.20, "11/5"));
	oddFraction.insert(std::make_pair(3.25, "9/4"));
	oddFraction.insert(std::make_pair(3.40, "12/5"));
	oddFraction.insert(std::make_pair(3.50, "5/2"));
	oddFraction.insert(std::make_pair(3.60, "13/5"));
	oddFraction.insert(std::make_pair(3.75, "11/4"));
	oddFraction.insert(std::make_pair(4.00, "3/1"));
	oddFraction.insert(std::make_pair(4.33, "10/3"));
	oddFraction.insert(std::make_pair(4.50, "7/2"));
	oddFraction.insert(std::make_pair(5.00, "4/1"));
	oddFraction.insert(std::make_pair(5.50, "9/2"));
	oddFraction.insert(std::make_pair(6.00, "5/1"));
	oddFraction.insert(std::make_pair(6.50, "11/2"));
	oddFraction.insert(std::make_pair(7.00, "6/1"));
	oddFraction.insert(std::make_pair(7.50, "13/2"));
	oddFraction.insert(std::make_pair(8.00, "7/1"));
	oddFraction.insert(std::make_pair(8.50, "15/2"));
	oddFraction.insert(std::make_pair(9.00, "8/1"));
	oddFraction.insert(std::make_pair(9.50, "17/2"));
	oddFraction.insert(std::make_pair(10.00, "9/1"));

	double EPS = 1e-9; // Epsilon

	for (std::map<float,std::string>::iterator it=oddFraction.begin(); it!=oddFraction.end(); ++it)
	{
		if(std::fabs(fraction - it->first) < EPS) // fraction <= it->first
		{
			return it->second;
		}
	}
	// Not in the Odds Conversion Table
	// Checking whole number or decimal // 5.00 = 5
	if(fraction == float((int)fraction))
	{
		return NumberToString((int)fraction - 1)+"/1"; // 10.00 = 9/1; 11.00 = 10/1; 12.00 = 11/1
	}
	else return "Unknown";
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
