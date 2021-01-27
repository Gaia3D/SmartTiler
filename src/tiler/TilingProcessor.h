/**
* TilingProcessor Header
*/
#ifndef _TILINGPROCESSOR_H_
#define _TILINGPROCESSOR_H_
#pragma once

#include <map>

#include <json/json.h>

class TilingProcessor
{
public:
	TilingProcessor();
	virtual ~TilingProcessor();

	static TilingProcessor *getProcessor() { return &tilingProcessor; }

	bool initialize(std::map<std::string, std::string> arguments);

	void process();

private:
	static TilingProcessor tilingProcessor;

	std::string inputDataPath;

	std::string outputPath;

	std::string geolocationPath;

	std::string dataGroupKey;

	bool bCreateSmartTile;

	bool bCreateSmartTileIndexFile;

private:
	void processSmartTileCreation();

	void makeSmartTileIndex(std::string targetFolder);

	bool loadGeolocationInfo(
		std::string &path,
		std::map<unsigned int, std::string> &dataKeys,
		std::map<unsigned int, std::string> &dataNames,
		std::map<unsigned int, std::string> &layerIds,
		std::map<unsigned int, unsigned int> &dataGroupIds,
		std::map<unsigned int, double> &longitudes,
		std::map<unsigned int, double> &latitudes,
		std::map<unsigned int, double> &heights,
		std::map<unsigned int, double> &headings,
		std::map<unsigned int, double> &pitches,
		std::map<unsigned int, double> &rolls,
		std::map<unsigned int, Json::Value> &attributes,
		std::string defaultLayerId);

	void collectCandidatesToBeTiled(std::string inputFolder, std::map<std::string, std::string> &targets);

	void crossCheckBetweenGeolocationInfoAndCandidates(
		std::map<unsigned int, std::string> &geolocationInfo,
		std::map<std::string, std::string> &candidates,
		std::map<unsigned int, std::string> &finalGeolocationInfo,
		std::map<std::string, std::string> &finalTargets);

	void matchTargetsToTileDepthAndIndices(
		std::map<unsigned int, std::string> &dataKeys,
		std::map<std::string, std::string> &dataToBeTiled,
		std::map<unsigned int, double> &longitudes,
		std::map<unsigned int, double> &latitudes,
		std::map<unsigned int, double> &heights,
		std::map<unsigned int, double> &headings,
		std::map<unsigned int, double> &pitches,
		std::map<unsigned int, double> &rolls,
		std::map<unsigned int, unsigned int> &depths,
		std::map<unsigned int, unsigned int> &xIndices,
		std::map<unsigned int, unsigned int> &yIndices);

	void findTileIndicesAtGivenDepth(
		unsigned int depth,
		double longitude,
		double latitude,
		unsigned int &xIndex,
		unsigned int &yIndex);

	bool makeSmartTiles(
		std::string outputFolder,
		std::map<unsigned int, std::string> &dataKeys,
		std::map<unsigned int, std::string> &dataNames,
		std::map<unsigned int, unsigned int> &dataGroupIds,
		std::map<unsigned int, std::string> &layerIds,
		std::map<unsigned int, double> &longitudes,
		std::map<unsigned int, double> &latitudes,
		std::map<unsigned int, double> &heights,
		std::map<unsigned int, double> &headings,
		std::map<unsigned int, double> &pitches,
		std::map<unsigned int, double> &rolls,
		std::map<unsigned int, Json::Value> &attributes,
		std::map<std::string, std::string> &dataToBeTiled,
		std::map<unsigned int, unsigned int> &depths,
		std::map<unsigned int, unsigned int> &xIndices,
		std::map<unsigned int, unsigned int> &yIndices);
};

#endif // _TILINGPROCESSOR_H_
