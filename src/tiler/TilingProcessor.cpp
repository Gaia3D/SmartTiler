/**
* Implementation of the TilingProcessor class
*/
#include "TilingProcessor.h"

#include <iostream>

#ifdef __APPLE__
#include <sys/uio.h>
#elif defined WIN32
#include <direct.h>
#define S_ISREG(m) (((m)&S_IFMT) == S_IFREG)
#define S_ISDIR(m) (((m)&S_IFMT) == S_IFDIR)
#define mkdir(dirname, mode) _mkdir(dirname)
#else
#include <sys/io.h>
#endif

#include <filesystem>
#include <cstring>

#include <sys/stat.h>

#include "argdefinition.h"
#include "predefinition.h"
#include "../util/LogWriter.h"
#include "SmartTile.h"

TilingProcessor TilingProcessor::tilingProcessor;

TilingProcessor::TilingProcessor()
{
	bCreateSmartTile = false;

	bCreateSmartTileIndexFile = false;
}

TilingProcessor::~TilingProcessor()
{
}

bool TilingProcessor::initialize(std::map<std::string, std::string> arguments)
{
	struct stat status;

	///< test if input json file exists
	if (arguments.find(InputFile) != arguments.end())
	{
		inputDataFile = arguments[InputFile];

		FILE* file = NULL;
		file = fopen(inputDataFile.c_str(), "rt");

		if (file == NULL)
		{
			LogWriter::getLogWriter()->setStatus(false, std::string(ERROR_FLAG) + std::string("TilingProcessor::initialize : ") + std::string(CANNOT_OPEN_FILE) + inputDataFile);
			return false;
		}

		fclose(file);

		bCreateSmartTile = true;
	}
	else
	{
		///< test if input folder exist
		if (arguments.find(InputFolder) != arguments.end())
		{
			inputDataPath = arguments[InputFolder];

			bool inputFolderExist = false;
			if (stat(inputDataPath.c_str(), &status) == 0)
			{
				if (S_ISDIR(status.st_mode))
					inputFolderExist = true;
			}

			if (!inputFolderExist)
			{
				LogWriter::getLogWriter()->setStatus(false, std::string(ERROR_FLAG) + std::string("TilingProcessor::initialize : ") + std::string(NO_PATH) + inputDataPath);
				return false;
			}

			bCreateSmartTile = true;
		}

		///< test if geolocation file exist
		if (arguments.find(GeolocationPath) != arguments.end())
		{
			geolocationPath = arguments[GeolocationPath];

			bool geolocaitonFolderExist = false;
			if (stat(geolocationPath.c_str(), &status) == 0)
			{
				if (S_ISDIR(status.st_mode))
					geolocaitonFolderExist = true;
			}

			if (!geolocaitonFolderExist)
			{
				LogWriter::getLogWriter()->setStatus(false, std::string(ERROR_FLAG) + std::string("TilingProcessor::initialize : ") + std::string(NO_PATH) + geolocationPath);
				return false;
			}
		}

		if (arguments.find(DataGroupKey) != arguments.end())
		{
			dataGroupKey = arguments[DataGroupKey];
		}
	}

	///< test if output folder exist
	if (arguments.find(OutputFolder) != arguments.end())
	{
		outputPath = arguments[OutputFolder];

		bool outputFolderExist = false;
		if (stat(outputPath.c_str(), &status) == 0)
		{
			if (S_ISDIR(status.st_mode))
				outputFolderExist = true;
		}

		if (!outputFolderExist)
		{
			LogWriter::getLogWriter()->setStatus(false, std::string(ERROR_FLAG) + std::string("TilingProcessor::initialize : ") + std::string(NO_PATH) + outputPath);
			return false;
		}
	}

	if (arguments.find(CreateIndex) != arguments.end())
	{
		if (arguments[CreateIndex] == std::string("Y") || arguments[CreateIndex] == std::string("y"))
			bCreateSmartTileIndexFile = true;
		else if (arguments[CreateIndex] == std::string("N") || arguments[CreateIndex] == std::string("n"))
			bCreateSmartTileIndexFile = false;
	}

	return true;
}

void TilingProcessor::process()
{
	if (bCreateSmartTile)
		processSmartTileCreation();

	if (bCreateSmartTileIndexFile)
		makeSmartTileIndex(outputPath);
}

void scanSmartTiles(std::string targetFolder, std::map<std::string, std::vector<unsigned int>> &info)
{
	namespace fs = std::filesystem;

	char stringBuffer[1024];

	std::vector<std::string> subFolders;

	fs::path folderPath(targetFolder);
	folderPath = folderPath.lexically_normal();

	if (fs::is_directory(folderPath))
	{
		std::cout << "In directory: " << folderPath.string() << std::endl;
		fs::recursive_directory_iterator end;
		for (fs::recursive_directory_iterator it(folderPath); it != end; ++it)
		{
			try
			{
				if (fs::is_directory(*it))
				{
					continue;
					std::cout << "[directory]" << it->path() << std::endl;
				}

				std::string dataFile = it->path().filename().string();
				std::string fileExt = it->path().extension().string();
				if (fileExt == std::string(".sti"))
				{
					std::vector<std::string> splittedWords;
					memset(stringBuffer, 0x00, 1024);
					memcpy(stringBuffer, dataFile.c_str(), dataFile.size());
					char *token = std::strtok(stringBuffer, "_.");
					while (token != NULL)
					{
						splittedWords.push_back(std::string(token));
						token = std::strtok(NULL, " _.");
					}
					// smartTile_13_13972_2391.sti
					if (splittedWords.size() == 5)
					{
						info[dataFile] = std::vector<unsigned int>();
						info[dataFile].push_back(std::atoi(splittedWords[1].c_str()));
						info[dataFile].push_back(std::atoi(splittedWords[2].c_str()));
						info[dataFile].push_back(std::atoi(splittedWords[3].c_str()));
					}
				}

				std::cout << "[file]" << dataFile << std::endl;
			}
			catch (const std::exception &ex)
			{
				std::cout << "scanSmartTiles" << ex.what() << std::endl;
			}
		}
	}
}

void TilingProcessor::makeSmartTileIndex(std::string targetFolder)
{
	std::map<std::string, std::vector<unsigned int>> tileInfo;
	scanSmartTiles(targetFolder, tileInfo);

	if (tileInfo.empty())
		return;

	std::string indexFileFullPath = targetFolder + std::string("/smartTile_f4d_indexFile.sii");
	FILE *file = NULL;
	file = fopen(indexFileFullPath.c_str(), "wb");
	if (file == NULL)
	{
		LogWriter::getLogWriter()->setStatus(false, std::string(ERROR_FLAG) + std::string("TilingProcessor::makeSmartTileIndex : ") + std::string(CANNOT_OPEN_FILE));
		return;
	}

	unsigned int tileInfoCount = (unsigned int)tileInfo.size();
	fwrite(&tileInfoCount, sizeof(unsigned int), 1, file);

	std::map<std::string, std::vector<unsigned int>>::iterator iter = tileInfo.begin();
	unsigned char smartTileType = SmartTileType;
	for (; iter != tileInfo.end(); iter++)
	{
		std::string smartTileFileName = iter->first;
		unsigned short fileNameLength = (unsigned short)smartTileFileName.size();
		fwrite(&fileNameLength, sizeof(unsigned short), 1, file);
		fwrite(smartTileFileName.c_str(), sizeof(char), fileNameLength, file);

		unsigned char depth = (unsigned char)(iter->second)[0];
		unsigned int xIndex = (iter->second)[1];
		unsigned int yIndex = (iter->second)[2];

		fwrite(&depth, sizeof(unsigned char), 1, file);
		fwrite(&xIndex, sizeof(unsigned int), 1, file);
		fwrite(&yIndex, sizeof(unsigned int), 1, file);
		fwrite(&smartTileType, sizeof(unsigned char), 1, file);
	}

	fclose(file);
}

void TilingProcessor::processSmartTileCreation()
{
	std::map<unsigned int, Json::Value> inputDataInfo;

	if (inputDataFile.empty())
	{
		if (!makeInputDataInfo(inputDataPath, inputDataInfo))
		{
			return;
		}
	}
	else
	{
		if (!loadInputDataInfo(inputDataFile, inputDataInfo))
		{
			return;
		}
	}

	///< match each input data to be tiled into depth and x/y indices of tiles
	std::map<unsigned int, unsigned int> depths, xIndices, yIndices;
	matchTargetsToTileDepthAndIndices(inputDataInfo, depths, xIndices, yIndices);
	if (depths.empty() || xIndices.empty() || yIndices.empty())
	{
		return;
	}

	///< make smart tiles
	makeSmartTiles(outputPath, inputDataInfo, depths, xIndices, yIndices);
}

bool TilingProcessor::makeSmartTiles(
	std::string outputFolder,
	std::map<unsigned int, Json::Value>& targets,
	std::map<unsigned int, unsigned int>& depths,
	std::map<unsigned int, unsigned int>& xIndices,
	std::map<unsigned int, unsigned int>& yIndices)
{
	struct stat status;

	///< check if depth folder and x index folder exist, and if not, make them
	///< rearrange data to be tiled along name of tile data to be made
	std::map<std::string, std::vector<unsigned int>> dataGroupedByTile;
	std::map<std::string, std::string> tileFullPaths;
	std::map<unsigned int, unsigned int>::iterator depthIter = depths.begin();
	for (; depthIter != depths.end(); depthIter++)
	{
		unsigned int dataId = depthIter->first;
		std::string depthString = std::to_string(depthIter->second);
		std::string xIndexString = std::to_string(xIndices[dataId]);

		// check and create depth folder
		bool bDepthFolderExist = false;
		std::string depthFolder = outputFolder + std::string("/") + depthString;
		if (stat(depthFolder.c_str(), &status) == 0)
		{
			if (S_ISDIR(status.st_mode))
				bDepthFolderExist = true;
		}

		if (!bDepthFolderExist)
		{
			if (mkdir(depthFolder.c_str(), 0755) != 0)
			{
				LogWriter::getLogWriter()->setStatus(false, std::string("TilingProcessor::makeSmartTiles : ") + std::string(CANNOT_CREATE_DIRECTORY) + depthFolder);
				return false;
			}
		}
		// check and create x index folder
		bool bXIndexFolderExist = false;
		std::string xIndexFolder = depthFolder + std::string("/") + xIndexString;
		if (stat(xIndexFolder.c_str(), &status) == 0)
		{
			if (S_ISDIR(status.st_mode))
				bXIndexFolderExist = true;
		}

		if (!bXIndexFolderExist)
		{
			if (mkdir(xIndexFolder.c_str(), 0755) != 0)
			{
				LogWriter::getLogWriter()->setStatus(false, std::string("TilingProcessor::makeSmartTiles : ") + std::string(CANNOT_CREATE_DIRECTORY) + xIndexFolder);
				return false;
			}
		}

		// make tile name and group data inside this tile
		std::string tileName = depthString + std::string("_") + xIndexString + std::string("_") + std::to_string(yIndices[dataId]);
		if (dataGroupedByTile.find(tileName) == dataGroupedByTile.end())
			dataGroupedByTile[tileName] = std::vector<unsigned int>();

		dataGroupedByTile[tileName].push_back(dataId);
		tileFullPaths[tileName] = xIndexFolder + std::string("/smartTile_") + tileName;
	}

	///< check if tiles to be made already exist
	///< if so, load previous tile file to add or overwrite data into this tile
	///< if not, make a new tile
	std::map<std::string, std::vector<unsigned int>>::iterator dataGroupIter = dataGroupedByTile.begin();
	for (; dataGroupIter != dataGroupedByTile.end(); dataGroupIter++)
	{
		for (unsigned char lod = LodMax; lod >= LodMin; lod--)
		{
			SmartTile tile;
			tile.tileLod = lod;
			std::string tileFullPath;
			if (lod == 5)
			{
				tileFullPath = tileFullPaths[dataGroupIter->first] + std::string(".sti");
			}
			else
			{
				tileFullPath = tileFullPath = tileFullPaths[dataGroupIter->first] + std::string("_") + std::to_string((unsigned int)lod) + std::string(".sti");
			}

			// load tile file if exist
			tile.readTile(tileFullPath);
			if (tile.buildingIds.empty())
			{
				LogWriter::getLogWriter()->createNewJobLog(std::string("create a new tile"), tileFullPath);
			}
			else
			{
				LogWriter::getLogWriter()->createNewJobLog(std::string("update an existing tile"), tileFullPath);
			}

			// read data and add this into this tile
			for (size_t i = 0; i < dataGroupIter->second.size(); i++)
			{
				unsigned int dataId = (dataGroupIter->second)[i];
				unsigned int dataGroupId = targets[dataId]["dataGroupId"].asUInt();
				std::string layerId = targets[dataId]["dataGroupKey"].asString();
				std::string dataKey = targets[dataId]["dataKey"].asString();
				std::string dataName = targets[dataId]["dataName"].asString();
				std::string dataPath = targets[dataId]["dataGroupPath"].asString() + std::string("/") + std::string(BasicDataPrefix) + dataKey;

				double longitude = targets[dataId]["longitude"].asDouble(), latitude = targets[dataId]["latitude"].asDouble(), height = targets[dataId]["altitude"].asDouble();
				double heading = targets[dataId]["heading"].asDouble(), pitch = targets[dataId]["pitch"].asDouble(), roll = targets[dataId]["roll"].asDouble();

				if (!tile.addData(
					dataPath,
					dataId,
					dataGroupId,
					layerId,
					dataKey,
					dataName,
					longitude,
					latitude,
					(float)height,
					pitch,
					roll,
					heading,
					targets[dataId]["attributes"]))
				{
					LogWriter::getLogWriter()->changeCurrentJobStatus(LogWriter::JOB_STATUS::warning);
					LogWriter::getLogWriter()->addDescriptionToCurrentJobLog(std::string("failed to add data into tile(") + dataPath + std::string(")"));
				}
			}

			if (tile.buildingIds.empty())
			{
				LogWriter::getLogWriter()->changeCurrentJobStatus(LogWriter::JOB_STATUS::failure);
				LogWriter::getLogWriter()->addDescriptionToCurrentJobLog(std::string("No data tiled on this tile"));
				LogWriter::getLogWriter()->closeCurrentJobLog();
				continue;
			}

			// write this tile into file
			if (!tile.writeTile(tileFullPath))
			{
				LogWriter::getLogWriter()->changeCurrentJobStatus(LogWriter::JOB_STATUS::failure);
				LogWriter::getLogWriter()->addDescriptionToCurrentJobLog(std::string("failed to write tile data"));
			}

			LogWriter::getLogWriter()->closeCurrentJobLog();
		}
	}

	return true;
}

void TilingProcessor::matchTargetsToTileDepthAndIndices
(
	std::map<unsigned int, Json::Value>& targets,
	std::map<unsigned int, unsigned int>& depths,
	std::map<unsigned int, unsigned int>& xIndices,
	std::map<unsigned int, unsigned int>& yIndices
)
{
	std::map<unsigned int, Json::Value>::iterator iter = targets.begin();
	Json::Value defaultValue(Json::nullValue);
	for (; iter != targets.end(); iter++)
	{
		Json::Value dataInfo = iter->second;

		unsigned int dataId = dataInfo["dataId"].asUInt();
		std::string dataKey = dataInfo["dataKey"].asString();

		std::string dataFolderName = std::string(BasicDataPrefix) + dataKey;
		std::string dataFullPath = dataInfo["dataGroupPath"].asString() + std::string("/") + dataFolderName;

		///< access to and read .hed file to get bbox info
		std::string metaFile = dataFullPath + std::string("/") + std::string(MetaFileName);
		FILE* file = NULL;
		file = fopen(metaFile.c_str(), "rb");
		if (file == NULL)
		{
			LogWriter::getLogWriter()->addMessageToLog(std::string(WARNING_FLAG) + std::string("TilingProcessor::matchTargetsToTileDepthAndIndices : ") + std::string(CANNOT_OPEN_FILE) + metaFile);
			continue;
		}

		char dummyBuffer[64];
		int dummyInt;
		double dummyDouble;
		float dummyFloat;
		memset(dummyBuffer, 0x00, 64);
		fread(dummyBuffer, sizeof(char), 5, file); // version
		fread(&dummyInt, sizeof(int), 1, file); // guid length
		memset(dummyBuffer, 0x00, 64);
		fread(dummyBuffer, sizeof(char), dummyInt, file); // guid
		fread(&dummyDouble, sizeof(double), 1, file); // longitude
		fread(&dummyDouble, sizeof(double), 1, file); // latitude
		fread(&dummyFloat, sizeof(float), 1, file); // height

		float minX, minY, minZ, maxX, maxY, maxZ, xLength, yLength, zLength;
		fread(&minX, sizeof(float), 1, file);
		fread(&minY, sizeof(float), 1, file);
		fread(&minZ, sizeof(float), 1, file);
		fread(&maxX, sizeof(float), 1, file);
		fread(&maxY, sizeof(float), 1, file);
		fread(&maxZ, sizeof(float), 1, file);

		fclose(file);

		xLength = maxX - minX;
		yLength = maxY - minY;
		zLength = maxZ - minZ;
		double maxLength = (xLength > yLength) ? ((xLength > zLength) ? xLength : zLength) : ((yLength > zLength) ? yLength : zLength);
		unsigned int depth = InitialTileDepth;
		double threshold[ThresholdCount] = lengthThresholdsForTileDepth;
		for (int i = 0; i < ThresholdCount; i++, depth++)
			if (maxLength >= threshold[i])
				break;

		unsigned int xIndex, yIndex;
		findTileIndicesAtGivenDepth(depth, dataInfo["longitude"].asDouble(), dataInfo["latitude"].asDouble(), xIndex, yIndex);

		depths[dataId] = depth;
		xIndices[dataId] = xIndex;
		yIndices[dataId] = yIndex;
	}
}

void TilingProcessor::findTileIndicesAtGivenDepth(
	unsigned int depth,
	double longitude,
	double latitude,
	unsigned int &xIndex,
	unsigned int &yIndex)
{
	///< applied only WGS84 tile schema at this version
	if (longitude >= 180.0)
		longitude -= 360.0;

	unsigned int depthCounter = 0;
	double minLat = -90.0, maxLat = 90.0;
	yIndex = 0;
	double minLon, maxLon;
	if (longitude < 0.0)
	{
		xIndex = 0;
		minLon = -180.0, maxLon = 0.0;
	}
	else
	{
		xIndex = 1;
		minLon = 0.0, maxLon = 180.0;
	}

	double midLon, midLat;
	while (depthCounter < depth)
	{
		midLon = (minLon + maxLon) / 2.0;
		if (longitude < midLon)
		{
			maxLon = midLon;
			xIndex *= 2;
		}
		else
		{
			minLon = midLon;
			xIndex = xIndex * 2 + 1;
		}

		midLat = (minLat + maxLat) / 2.0;
		if (latitude < midLat)
		{
			maxLat = midLat;
			yIndex = yIndex * 2 + 1;
		}
		else
		{
			minLat = midLat;
			yIndex *= 2;
		}

		depthCounter++;
	}
}

void TilingProcessor::crossCheckBetweenGeolocationInfoAndCandidates(
	std::map<unsigned int, std::string> &geolocationInfo,
	std::map<std::string, std::string> &candidates,
	std::map<unsigned int, std::string> &finalGeolocationInfo,
	std::map<std::string, std::string> &finalTargets)
{
	std::map<unsigned int, std::string>::iterator itr = geolocationInfo.begin();
	for (; itr != geolocationInfo.end(); itr++)
	{
		std::string dataName = std::string(BasicDataPrefix) + itr->second;
		if (candidates.find(dataName) != candidates.end())
		{
			finalGeolocationInfo[itr->first] = itr->second;
			finalTargets[dataName] = candidates[dataName];
		}
	}

	if (geolocationInfo.size() > finalGeolocationInfo.size())
	{
		size_t missedItemCount = geolocationInfo.size() - finalGeolocationInfo.size();
		std::string countString = std::to_string(missedItemCount);
		std::string content = std::string("[") + countString + std::string(" items in geolocation info are filtered out during cross checking]");
		LogWriter::getLogWriter()->addMessageToLog(std::string(WARNING_FLAG) + std::string("TilingProcessor::crossCheckBetweenGeolocationInfoAndCandidates : ") + content);
	}
}

bool TilingProcessor::loadGeolocationInfo(
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
	std::string defaultLayerId)
{
	std::vector<std::string> geolocationFiles;

	namespace fs = std::filesystem;

	fs::path folderPath(path);

	///< scan all geolocation files first
	if (fs::is_directory(folderPath))
	{
		std::cout << "In directory(GeolocationInfo): " << folderPath.string() << std::endl;
		fs::directory_iterator end;
		for (fs::directory_iterator it(folderPath); it != end; ++it)
		{
			try
			{
				if (!fs::is_directory(*it))
				{
					std::string geoLocationFile = it->path().filename().string();
					std::string fileExt = it->path().extension().string();
					if (fileExt == std::string(".json"))
					{
						std::string geoLocationFileFullPath = path + std::string("/") + geoLocationFile;
						geolocationFiles.push_back(geoLocationFileFullPath);

						std::cout << "[file]" << geoLocationFile << std::endl;
					}
				}
			}
			catch (const std::exception &ex)
			{
				std::cout << it->path().filename() << " " << ex.what() << std::endl;
			}
		}
	}
	else
	{
		LogWriter::getLogWriter()->setStatus(false, std::string(ERROR_FLAG) + std::string("TilingProcessor::loadGeolocationInfo : ") + std::string(NO_GEOLOCATION_FILE));
		return false;
	}

	if (geolocationFiles.empty())
	{
		LogWriter::getLogWriter()->setStatus(false, std::string(ERROR_FLAG) + std::string("TilingProcessor::loadGeolocationInfo : ") + std::string(NO_GEOLOCATION_FILE));
		return false;
	}

	///< read all geolocation files
	for (size_t i = 0; i < geolocationFiles.size(); i++)
	{
		std::string contents;
		FILE *file = NULL;
		file = fopen(geolocationFiles[i].c_str(), "rt");

		if (file == NULL)
		{
			LogWriter::getLogWriter()->addMessageToLog(std::string(WARNING_FLAG) + std::string("TilingProcessor::loadGeolocationInfo : ") + std::string(INVALID_GEOLOCATION_FILE) + geolocationFiles[i]);
			continue;
		}

		char stringBlock[4096];
		while (!feof(file))
		{
			memset(stringBlock, 0x00, 4096);
			fgets(stringBlock, 4096, file);

			contents += std::string(stringBlock);
		}
		fclose(file);

		Json::Reader reader;
		Json::Value root(Json::objectValue);
		if (!reader.parse(contents, root, false))
		{
			LogWriter::getLogWriter()->addMessageToLog(std::string(WARNING_FLAG) + std::string("TilingProcessor::loadGeolocationInfo : ") + std::string(FAILURE_READING_GEOLOCATION) + geolocationFiles[i]);
			continue;
		}

		std::string layerId;
		Json::Value rootDataKey = root.get(std::string("data_key"), Json::Value(Json::nullValue));
		if (rootDataKey.type() == Json::nullValue)
		{
			layerId = dataGroupKey;
		}
		else
		{
			layerId = root["data_key"].asString();
		}

		Json::Value children = root["children"];
		unsigned int childCount = children.size();
		for (unsigned int j = 0; j < childCount; j++)
		{
			// i-th child
			Json::Value child(Json::objectValue);
			child = children.get(j, child);

			// data id
			unsigned int dataId = child["dataId"].asUInt();

			if (dataKeys.find(dataId) != dataKeys.end())
			{
				LogWriter::getLogWriter()->addMessageToLog(std::string(WARNING_FLAG) + std::string("TilingProcessor::loadGeolocationInfo : ") + std::string(DUPLICATED_DATA_ID) + std::to_string(dataId) + std::string(":") + geolocationFiles[i]);
				continue;
			}

			// data key
			dataKeys[dataId] = child["data_key"].asString();

			// layer id
			layerIds[dataId] = layerId;

			// data group id
			dataGroupIds[dataId] = child["dataGroupId"].asInt();

			// data name
			dataNames[dataId] = child["data_name"].asString();

			// longitude
			longitudes[dataId] = child["longitude"].asDouble();

			// latitude
			latitudes[dataId] = child["latitude"].asDouble();

			// height
			heights[dataId] = child["height"].asDouble();

			// heading
			headings[dataId] = child["heading"].asDouble();

			// pitch
			pitches[dataId] = child["pitch"].asDouble();

			// roll
			rolls[dataId] = child["roll"].asDouble();

			// attributes(with conversion of heightReference type from string to integer)
			Json::Value childAttribute(Json::objectValue), attributeNode(Json::objectValue);
			childAttribute = child["attributes"];
			if (childAttribute.isMember("heightReference"))
			{
				if (childAttribute["heightReference"].asString() == std::string("clampToGround"))
				{
					attributeNode["heightReference"] = 1;
				}
				else if (childAttribute["heightReference"].asString() == std::string("relativeToGround"))
				{
					attributeNode["heightReference"] = 2;
				}
				else
				{
					attributeNode["heightReference"] = 0;
				}
			}
			if (childAttribute.isMember("flipYTexCoords"))
				attributeNode["flipYTexCoords"] = childAttribute["flipYTexCoords"].asBool();

			attributes[dataId] = attributeNode;
		}
	}

	if (dataKeys.empty())
	{
		LogWriter::getLogWriter()->setStatus(false, std::string(ERROR_FLAG) + std::string("TilingProcessor::loadGeolocationInfo : ") + std::string(NO_GEOLOCATION_INFO));
		return false;
	}

	return true;
}

void TilingProcessor::collectCandidatesToBeTiled(std::string targetFolder, std::map<std::string, std::string> &info)
{
	namespace fs = std::filesystem;

	fs::path folderPath(targetFolder);
	folderPath = folderPath.lexically_normal();

	if (fs::is_directory(folderPath))
	{
		std::cout << "In directory: " << folderPath.string() << std::endl;
		fs::recursive_directory_iterator end;
		for (fs::recursive_directory_iterator it(folderPath); it != end; ++it)
		{
			const fs::path subFolderPath = (*it);
			std::string subFolder = subFolderPath.filename().string();

			try
			{
				if (!fs::is_directory(subFolderPath) || subFolder.find(std::string("F4D_")) != 0)
				{
					continue;
				}

				info[subFolder] = subFolderPath.string();
				std::cout << "[directory]" << subFolderPath << std::endl;
			}
			catch (const std::exception &ex)
			{
				std::cout << subFolder << " " << ex.what() << std::endl;
			}
		}
	}
}

bool TilingProcessor::loadInputDataInfo
(
	std::string& path,
	std::map<unsigned int, Json::Value>& result
)
{
	std::string contents;
	FILE* file = NULL;
	file = fopen(inputDataFile.c_str(), "rt");

	if (file == NULL)
	{
		LogWriter::getLogWriter()->addMessageToLog(std::string(WARNING_FLAG) + std::string("TilingProcessor::loadInputDataInfo : ") + std::string(CANNOT_OPEN_FILE) + inputDataFile);
		return false;
	}

	char stringBlock[4096];
	while (!feof(file))
	{
		memset(stringBlock, 0x00, 4096);
		fgets(stringBlock, 4096, file);

		contents += std::string(stringBlock);
	}
	fclose(file);

	Json::Reader reader;
	Json::Value root(Json::objectValue);
	if (!reader.parse(contents, root, false))
	{
		std::string lastError = reader.getFormatedErrorMessages();
		LogWriter::getLogWriter()->addMessageToLog(
			std::string(ERROR_FLAG) +
			std::string("TilingProcessor::loadInputDataInfo : ") +
			std::string(PARSING_ERROR) +
			inputDataFile +
			std::string("|") +
			lastError);

		return false;
	}

	if (!root.isMember("tileId"))
	{
		LogWriter::getLogWriter()->addMessageToLog(std::string(ERROR_FLAG) + std::string("TilingProcessor::loadInputDataInfo : ") + std::string(NO_INFO) + std::string("tileId"));
		return false;
	}

	if (LogWriter::getLogWriter()->isEmptyFileName())
	{
		unsigned int tileId = root["tileId"].asUInt();
		LogWriter::getLogWriter()->setFileNameByPrefix(std::to_string(tileId));
	}

	if (!root.isMember("tileDataGroupList"))
	{
		LogWriter::getLogWriter()->addMessageToLog(std::string(ERROR_FLAG) + std::string("TilingProcessor::loadInputDataInfo : ") + std::string(NO_INFO) + std::string("tileDataGroupList"));
		return false;
	}

	unsigned int tileDataGroupCount = root["tileDataGroupList"].size();
	if (tileDataGroupCount == 0)
	{
		LogWriter::getLogWriter()->addMessageToLog(std::string(ERROR_FLAG) + std::string("TilingProcessor::loadInputDataInfo : ") + std::string(NO_INFO) + std::string("tileDataGroupList"));
		return false;
	}

	Json::Value defaultValue(Json::nullValue);
	for (unsigned int i = 0; i < tileDataGroupCount; i++)
	{
		Json::Value dataGroup = root["tileDataGroupList"].get(i, defaultValue);

		if (!dataGroup.isMember("dataGroupId"))
			continue;
		unsigned int dataGroupId = dataGroup["dataGroupId"].asUInt();

		if (!dataGroup.isMember("dataGroupKey"))
			continue;
		std::string dataGroupKey = dataGroup["dataGroupKey"].asString();

		if (!dataGroup.isMember("absoluteDataGroupPath"))
			continue;
		std::string dataGroupPath = dataGroup["absoluteDataGroupPath"].asString();
		if (dataGroupPath.find_last_of('/') == dataGroupPath.length() - 1)
			dataGroupPath = dataGroupPath.substr(0, dataGroupPath.length() - 1);
		if (dataGroupPath.find_last_of('\\') == dataGroupPath.length() - 1)
			dataGroupPath = dataGroupPath.substr(0, dataGroupPath.length() - 1);

		if (!dataGroup.isMember("dataInfoList"))
			continue;
		unsigned int dataInfoCount = dataGroup["dataInfoList"].size();
		if (dataInfoCount == 0)
			continue;

		for (unsigned int j = 0; j < dataInfoCount; j++)
		{
			Json::Value dataInfo = dataGroup["dataInfoList"].get(j, defaultValue);
			Json::Value trimmedDataInfo(Json::objectValue);

			if (!dataInfo.isMember("dataId"))
				continue;
			trimmedDataInfo["dataId"] = dataInfo["dataId"].asUInt();

			if (!dataInfo.isMember("dataKey"))
				continue;
			trimmedDataInfo["dataKey"] = dataInfo["dataKey"].asString();

			if (!dataInfo.isMember("dataName"))
				continue;
			trimmedDataInfo["dataName"] = dataInfo["dataName"].asString();

			if (!dataInfo.isMember("longitude"))
				continue;
			trimmedDataInfo["longitude"] = dataInfo["longitude"].asDouble();

			if (!dataInfo.isMember("latitude"))
				continue;
			trimmedDataInfo["latitude"] = dataInfo["latitude"].asDouble();

			if (!dataInfo.isMember("altitude"))
				continue;
			trimmedDataInfo["altitude"] = dataInfo["altitude"].asDouble();

			if (!dataInfo.isMember("heading"))
				continue;
			trimmedDataInfo["heading"] = dataInfo["heading"].asDouble();

			if (!dataInfo.isMember("pitch"))
				continue;
			trimmedDataInfo["pitch"] = dataInfo["pitch"].asDouble();

			if (!dataInfo.isMember("roll"))
				continue;
			trimmedDataInfo["roll"] = dataInfo["roll"].asDouble();

			if (!dataInfo.isMember("metainfo"))
				continue;
			std::string attributeString = dataInfo["metainfo"].asString();

			Json::Value attributeRoot(Json::objectValue);
			if (!reader.parse(attributeString, attributeRoot, false))
				continue;

			Json::Value attributeNode(Json::objectValue);
			if (attributeRoot.isMember("flipYTexCoords"))
				attributeNode["flipYTexCoords"] = attributeRoot["flipYTexCoords"].asBool();
			if (attributeRoot.isMember("heightReference"))
			{
				if (attributeRoot["heightReference"].asString() == std::string("clampToGround"))
					attributeNode["heightReference"] = 1;
				else if (attributeRoot["heightReference"].asString() == std::string("relativeToGround"))
					attributeNode["heightReference"] = 2;
				else
					attributeNode["heightReference"] = 0;
			}
			trimmedDataInfo["attributes"] = attributeNode;

			trimmedDataInfo["dataGroupId"] = dataGroupId;
			trimmedDataInfo["dataGroupKey"] = dataGroupKey;
			trimmedDataInfo["dataGroupPath"] = dataGroupPath;

			result[trimmedDataInfo["dataId"].asUInt()] = trimmedDataInfo;
		}
	}

	if (result.empty())
	{
		LogWriter::getLogWriter()->setStatus(false, std::string(ERROR_FLAG) + std::string("TilingProcessor::loadInputDataInfo : ") + std::string(NO_INFO));
		return false;
	}

	return true;
}

bool TilingProcessor::makeInputDataInfo(std::string& path, std::map<unsigned int, Json::Value>& result)
{
	namespace fs = std::filesystem;

	std::map<unsigned int, std::string> dataNames, dataKeys, layerIds;
	std::map<unsigned int, unsigned int> dataGroupIds;
	std::map<unsigned int, double> longitudes, latitudes, heights, headings, pitches, rolls;
	std::map<unsigned int, Json::Value> attributes;
	if (!loadGeolocationInfo(geolocationPath, dataKeys, dataNames, layerIds, dataGroupIds, longitudes, latitudes, heights, headings, pitches, rolls, attributes, dataGroupKey))
	{
		return false;
	}

	///< scan all candidates to be tiled
	std::map<std::string, std::string> candidatesToBeTiled;
	collectCandidatesToBeTiled(inputDataPath, candidatesToBeTiled);

	if (candidatesToBeTiled.empty())
	{
		LogWriter::getLogWriter()->setStatus(false, std::string(ERROR_FLAG) + std::string("TilingProcessor::processSmartTileCreation : ") + std::string(NO_INPUT_DATA));
		return false;
	}

	///< extract cross-section between geolocation info and candidates to confirm targets to be tiled
	std::map<unsigned int, std::string> finalDataKeys;
	std::map<std::string, std::string> dataToBeTiled;
	crossCheckBetweenGeolocationInfoAndCandidates(dataKeys, candidatesToBeTiled, finalDataKeys, dataToBeTiled);


	Json::Value trimmedDataInfo(Json::objectValue);
	std::map<unsigned int, std::string>::iterator keyIter = finalDataKeys.begin();
	for (; keyIter != finalDataKeys.end(); keyIter++)
	{
		unsigned int dataId = keyIter->first;
		std::string dataKey = keyIter->second;

		std::string dataFolderName = std::string(BasicDataPrefix) + dataKey;
		std::string dataFullPath = dataToBeTiled[dataFolderName];
		fs::path dataGroupPath(dataFullPath);
		
		trimmedDataInfo["dataId"] = dataId;
		trimmedDataInfo["dataKey"] = dataKey;
		trimmedDataInfo["dataName"] = dataNames[dataId];
		trimmedDataInfo["longitude"] = longitudes[dataId];
		trimmedDataInfo["latitude"] = latitudes[dataId];
		trimmedDataInfo["altitude"] = heights[dataId];
		trimmedDataInfo["heading"] = headings[dataId];
		trimmedDataInfo["pitch"] = pitches[dataId];
		trimmedDataInfo["roll"] = rolls[dataId];
		trimmedDataInfo["attributes"] = attributes[dataId];
		trimmedDataInfo["dataGroupId"] = dataGroupIds[dataId];
		trimmedDataInfo["dataGroupKey"] = dataGroupKey;
		trimmedDataInfo["dataGroupPath"] = dataGroupPath.parent_path().string();

		result[trimmedDataInfo["dataId"].asUInt()] = trimmedDataInfo;
	}

	return true;
}