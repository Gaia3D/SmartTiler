/**
* SmartTile Header
*/
#ifndef _SMARTTILE_H_
#define _SMARTTILE_H_
#pragma once

#include <string>
#include <vector>
#include <json/json.h>

#define SmartTileType 2

class SmartTile
{
public:
	SmartTile();
	virtual ~SmartTile();

public:
	bool readTile(std::string tilePath);

	bool writeTile(std::string tilePath);

	bool addData(
		std::string dataPath,
		unsigned int dataId,
		unsigned int dataGroupId,
		std::string layerId,
		std::string dataKey,
		std::string dataName,
		double longitude,
		double latitude,
		float altitude,
		double pitch,
		double roll,
		double heading,
		Json::Value& attribute);

public:

	unsigned char tileLod;

	std::vector<unsigned int> dataIds;
	std::vector<unsigned int> dataGroupIds;
	std::vector<std::string> layerIds;
	std::vector<std::string> buildingIds;
	std::vector<std::string> dataNames;
	std::vector<unsigned int> metaDataSizes;
	std::vector<unsigned char*> metaData;
	std::vector<std::string> lodMeshNames;
	std::vector<unsigned int> meshDataSizes;
	std::vector<unsigned char*> meshData;
	std::vector<unsigned int> imageDataSizes;
	std::vector<unsigned char*> imageData;
	std::vector<double> longitudes;
	std::vector<double> latitudes;
	std::vector<float> altitudes;
	std::vector<float> headings;
	std::vector<float> pitches;
	std::vector<float> rolls;
	std::vector<Json::Value> attributes;
};

#endif // _SMARTTILE_H_