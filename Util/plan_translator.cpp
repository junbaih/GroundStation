//-ljsoncpp
#include "plan_translator.h"
#include <iostream>
#include <fstream>
#include <jsoncpp/json/json.h>
#include <jsoncpp/json/writer.h>
#include <string>

using namespace std;


void translate(){
	ifstream fly_file("flyFile.json");
	ofstream plan_file("planFile.json");

	Json::Reader reader;
	Json::Value root;
	Json::StyledStreamWriter writer;
	Json::Value write_root;	

	if (! reader.parse(fly_file, root)){
		cout << "Failed to parse file\n" << reader.getFormattedErrorMessages();
		return;
	}
	
	string geoFence_type;
	Json::Value geoFences(Json::arrayValue);

	for (int i=0; i<root.size(); ++i){
		
		Json::Value fly_mission = root[i];
		const Json::Value geoFence = fly_mission["fly_zones"][0]["boundary_pts"];

		if (geoFence.size() == 3)	geoFence_type = "circles";
		else if (geoFence.size() == 4)	geoFence_type = "polygons";

		for (int index=0; index < geoFence.size(); ++index){
			Json::Value tmp(Json::arrayValue);
			tmp.append(geoFence[index]["latitude"]);
			tmp.append(geoFence[index]["longitude"]);
			geoFences.append(tmp);
//			geoFences[index][0] = stod(geoFence[index]["latitude"].asString());
//			geoFences[index][1] = stod(geoFence[index]["longitude"].asString());
		}
//		for (int j=0; j < sizeof(geoFences)/sizeof(geoFences[0]); ++j)
//			cout << geoFences[j][0] << geoFences[j][1] << endl;
	}

//	Json::Value geo;
//	geo[geoFence_type] = geoFences;
//	geo["version"] = 2;


	// write plan file
	write_root["fileType"] = "Plan";
	write_root["geoFence"][geoFence_type] = geoFences;
	write_root["geoFence"]["version"] = 2;
	write_root["groundStation"] = "QGroundControl";
	write_root["mission"] = "{}";
	write_root["rallyPoints"] = "{}";
	write_root["version"] = 1;

	writer.write(plan_file, write_root);
	plan_file.close();
}

int main(){
	translate();
	return 0;
}
