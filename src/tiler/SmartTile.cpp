/**
* Implementation of the SmartTile class
*/
#include "SmartTile.h"

///< string buffer size for reading
#define StringBufferSize 1024

SmartTile::SmartTile()
{

}

SmartTile::~SmartTile()
{
	for (size_t i = 0; i < metaData.size(); i++)
		delete[] metaData[i];

	for (size_t i = 0; i < meshData.size(); i++)
		delete[] meshData[i];

	for (size_t i = 0; i < imageData.size(); i++)
		delete[] imageData[i];
}

bool SmartTile::readTile(std::string tilePath)
{
	FILE* file = NULL;
	file = fopen(tilePath.c_str(), "rb");
	if (file == NULL)
		return false;

	// smart tile type
	int smartTileF4dType = -1;
	fread(&smartTileF4dType, sizeof(int), 1, file);

	// count of data in this tile
	unsigned int f4dBuildingCount = 0;
	fread(&f4dBuildingCount, sizeof(unsigned int), 1, file);

	char stringBuffer[StringBufferSize];
	for (unsigned int i = 0; i < f4dBuildingCount; i++)
	{
		// layer(project) id length
		unsigned short layerIdLength;
		fread(&layerIdLength, sizeof(unsigned short), 1, file);

		if (layerIdLength != 0) {
			// layer(project) id
			memset(stringBuffer, 0x00, sizeof(char) * StringBufferSize);
			fread(stringBuffer, sizeof(char), layerIdLength, file);
			layerIds.push_back(std::string(stringBuffer));
		}
		// building id length
		unsigned short buildingIdLength;
		fread(&buildingIdLength, sizeof(unsigned short), 1, file);
		if (buildingIdLength != 0) {
			// building id
			memset(stringBuffer, 0x00, sizeof(char) * StringBufferSize);
			fread(stringBuffer, sizeof(char), buildingIdLength, file);
			buildingIds.push_back(std::string(stringBuffer));
		}
		// metadata size
		unsigned int metaDataSize;
		fread(&metaDataSize, sizeof(unsigned int), 1, file);
		metaDataSizes.push_back(metaDataSize);

		// metadata
		unsigned char* metaDataBuffer = new unsigned char[metaDataSize];
		memset(metaDataBuffer, 0x00, sizeof(unsigned char) * metaDataSize);
		fread(metaDataBuffer, sizeof(unsigned char), metaDataSize, file);
		metaData.push_back(metaDataBuffer);

		// lod
		unsigned char lod;
		fread(&lod, sizeof(unsigned char), 1, file);

		// lod name length
		unsigned short lodMeshNameLength;
		fread(&lodMeshNameLength, sizeof(unsigned short), 1, file);
		if (lodMeshNameLength != 0) {
			// lod name
			memset(stringBuffer, 0x00, sizeof(char) * StringBufferSize);
			fread(stringBuffer, sizeof(char), lodMeshNameLength, file);
			lodMeshNames.push_back(std::string(stringBuffer));
		}
		// mesh data size
		unsigned int meshDataSize;
		fread(&meshDataSize, sizeof(unsigned int), 1, file);
		meshDataSizes.push_back(meshDataSize);

		// mesh data
		unsigned char* meshDataBuffer = new unsigned char[meshDataSize];
		memset(meshDataBuffer, 0x00, sizeof(unsigned char) * meshDataSize);
		fread(meshDataBuffer, sizeof(unsigned char), meshDataSize, file);
		meshData.push_back(meshDataBuffer);

		// image data size
		unsigned int imageDataSize;
		fread(&imageDataSize, sizeof(unsigned int), 1, file);
		imageDataSizes.push_back(imageDataSize);
		if (imageDataSize > 0) {
			// image data
			unsigned char* imageDataBuffer = new unsigned char[imageDataSize];
			memset(imageDataBuffer, 0x00, sizeof(unsigned char) * imageDataSize);
			fread(imageDataBuffer, sizeof(unsigned char), imageDataSize, file);
			imageData.push_back(imageDataBuffer);
		}
		// longitude
		double longitude;
		fread(&longitude, sizeof(double), 1, file);
		longitudes.push_back(longitude);

		// latitude
		double latitude;
		fread(&latitude, sizeof(double), 1, file);
		latitudes.push_back(latitude);

		// altitude
		float altitude;
		fread(&altitude, sizeof(float), 1, file);
		altitudes.push_back(altitude);

		// pitch
		float pitch;
		fread(&pitch, sizeof(float), 1, file);
		pitches.push_back(pitch);

		// roll
		float roll;
		fread(&roll, sizeof(float), 1, file);
		rolls.push_back(roll);

		// heading
		float heading;
		fread(&heading, sizeof(float), 1, file);
		headings.push_back(heading);

		// data id
		unsigned int dataId;
		fread(&dataId, sizeof(unsigned int), 1, file);
		dataIds.push_back(dataId);

		// data group id
		unsigned int dataGroupId;
		fread(&dataGroupId, sizeof(unsigned int), 1, file);
		dataGroupIds.push_back(dataGroupId);

		// attribute padding
		char endMark;
		fread(&endMark, sizeof(char), 1, file);

		if (endMark == 5)
		{
			unsigned short dataNameLength;
			fread(&dataNameLength, sizeof(unsigned short), 1, file);
			memset(stringBuffer, 0x00, sizeof(char) * StringBufferSize);
			fread(stringBuffer, sizeof(char), dataNameLength, file);
			dataNames.push_back(std::string(stringBuffer));
		}

		// attribute
		Json::Value attribute(Json::objectValue);
		fread(&endMark, sizeof(char), 1, file);
		while (endMark != 0)
		{
			// data key length
			unsigned short attributeKeyLength;
			fread(&attributeKeyLength, sizeof(unsigned short), 1, file);

			// data key
			memset(stringBuffer, 0x00, sizeof(char) * StringBufferSize);
			fread(stringBuffer, sizeof(char), attributeKeyLength, file);
			std::string attributeKey(stringBuffer);

			switch (endMark)
			{
			case 1: // boolean value
			{
				bool bValue;
				fread(&bValue, sizeof(bool), 1, file);

				attribute[attributeKey] = bValue;
			}
			break;
			case 2: // 1byte int value(char)
			{
				char value;
				fread(&value, sizeof(char), 1, file);

				attribute[attributeKey] = (int)value;
			}
			break;
			case 3: // 2bytes int value(short)
			{
				short value;
				fread(&value, sizeof(short), 1, file);

				attribute[attributeKey] = (int)value;
			}
			break;
			case 4: // 4bytes int value
			{
				int value;
				fread(&value, sizeof(int), 1, file);

				attribute[attributeKey] = value;
			}
			break;
			case 5: // string value
			{
				// value length
				unsigned short attributeValueLength;
				fread(&attributeValueLength, sizeof(unsigned short), 1, file);

				// value
				memset(stringBuffer, 0x00, sizeof(char) * StringBufferSize);
				fread(stringBuffer, sizeof(char), attributeValueLength, file);

				attribute[attributeKey] = std::string(stringBuffer);
			}
			break;
			case 6: // floating value
			{
				float value;
				fread(&value, sizeof(float), 1, file);

				attribute[attributeKey] = (double)value;
			}
			break;
			}

			fread(&endMark, sizeof(char), 1, file);
		}

		attributes.push_back(attribute);
	}

	fclose(file);

	return true;
}

bool SmartTile::writeTile(std::string tilePath)
{
	FILE* file = NULL;
	file = fopen(tilePath.c_str(), "wb");
	if (file == NULL)
		return false;

	int smartTileType = SmartTileType;
	fwrite(&smartTileType, sizeof(int), 1, file);

	unsigned int buildingCount = (unsigned int)buildingIds.size();
	fwrite(&buildingCount, sizeof(unsigned int), 1, file);

	for (size_t i = 0; i < dataIds.size(); i++)
	{
		unsigned short layerIdLength = (unsigned short)layerIds[i].size();
		fwrite(&layerIdLength, sizeof(unsigned short), 1, file);
		if(layerIdLength != 0)
			fwrite(layerIds[i].c_str(), sizeof(char), layerIdLength, file);

		unsigned short buildingIdLength = (unsigned short)buildingIds[i].size();
		fwrite(&buildingIdLength, sizeof(unsigned short), 1, file);
		if(buildingIdLength != 0)
			fwrite(buildingIds[i].c_str(), sizeof(char), buildingIdLength, file);

		unsigned int metaDataSize = metaDataSizes[i];
		fwrite(&metaDataSize, sizeof(unsigned int), 1, file);
		fwrite(metaData[i], sizeof(unsigned char), metaDataSize, file);

		fwrite(&tileLod, sizeof(unsigned char), 1, file);

		unsigned short lodMeshNameLength = (unsigned short)lodMeshNames[i].size();
		fwrite(&lodMeshNameLength, sizeof(unsigned short), 1, file);
		if(lodMeshNameLength != 0)
			fwrite(lodMeshNames[i].c_str(), sizeof(char), lodMeshNameLength, file);

		unsigned int meshDataSize = meshDataSizes[i];
		fwrite(&meshDataSize, sizeof(unsigned int), 1, file);
		fwrite(meshData[i], sizeof(unsigned char), meshDataSize, file);

		unsigned int imageDataSize = imageDataSizes[i];
		fwrite(&imageDataSize, sizeof(unsigned int), 1, file);
		if(imageDataSize > 0)
			fwrite(imageData[i], sizeof(unsigned char), imageDataSize, file);

		double longitude = longitudes[i];
		fwrite(&longitude, sizeof(double), 1, file);

		double latitude = latitudes[i];
		fwrite(&latitude, sizeof(double), 1, file);

		float altitude = altitudes[i];
		fwrite(&altitude, sizeof(float), 1, file);

		float pitch = pitches[i];
		fwrite(&pitch, sizeof(float), 1, file);

		float roll = rolls[i];
		fwrite(&roll, sizeof(float), 1, file);

		float heading = headings[i];
		fwrite(&heading, sizeof(float), 1, file);

		unsigned int dataId = dataIds[i];
		fwrite(&dataId, sizeof(unsigned int), 1, file);

		unsigned int dataGroupId = dataGroupIds[i];
		fwrite(&dataGroupId, sizeof(unsigned int), 1, file);

		char endMark = 5;
		fwrite(&endMark, sizeof(char), 1, file);
		std::string dataNameKey = std::string("data_name");
		unsigned short dataNameKeyLength = (unsigned short)dataNameKey.size();
		fwrite(&dataNameKeyLength, sizeof(unsigned short), 1, file);
		fwrite(dataNameKey.c_str(), sizeof(char), dataNameKeyLength, file);

		unsigned short dataNameValueLength = (unsigned short)dataNames[i].size();
		fwrite(&dataNameValueLength, sizeof(unsigned short), 1, file);
		fwrite(dataNames[i].c_str(), sizeof(char), dataNameValueLength, file);

		if (!attributes[i].empty())
		{
			std::vector<std::string> keys = attributes[i].getMemberNames();
			for (size_t j = 0; j < keys.size(); j++)
			{
				if ((attributes[i])[keys[j]].isBool())
					endMark = 1;
				else if ((attributes[i])[keys[j]].isDouble())
					endMark = 6;
				else if ((attributes[i])[keys[j]].isString())
					endMark = 5;
				else
					endMark = 4;

				fwrite(&endMark, sizeof(char), 1, file);

				unsigned short keyLength = (unsigned short)keys[j].size();
				fwrite(&keyLength, sizeof(unsigned short), 1, file);
				fwrite(keys[j].c_str(), sizeof(char), keyLength, file);

				switch (endMark)
				{
				case 1:
				{
					bool value = (attributes[i])[keys[j]].asBool();
					fwrite(&value, sizeof(bool), 1, file);
				}
				break;
				case 4:
				{
					int value = (attributes[i])[keys[j]].asInt();
					fwrite(&value, sizeof(int), 1, file);
				}
				break;
				case 5:
				{
					std::string value = (attributes[i])[keys[j]].asString();
					unsigned short valueLength = (unsigned short)value.size();
					fwrite(&valueLength, sizeof(unsigned short), 1, file);
					fwrite(value.c_str(), sizeof(char), valueLength, file);
				}
				break;
				case 6:
				{
					float value = (float)(attributes[i])[keys[j]].asDouble();
					fwrite(&value, sizeof(float), 1, file);
				}
				break;
				}
			}
		}

		endMark = 0;
		fwrite(&endMark, sizeof(char), 1, file);

	}

	fclose(file);

	return true;
}

void readOctreeByDummy(FILE* f)
{
	int dummyInt;
	fread(&dummyInt, sizeof(int), 1, f);

	if (dummyInt == 0)
	{
		float dummyFloat;

		fread(&dummyFloat, sizeof(float), 1, f);
		fread(&dummyFloat, sizeof(float), 1, f);
		fread(&dummyFloat, sizeof(float), 1, f);
		fread(&dummyFloat, sizeof(float), 1, f);
		fread(&dummyFloat, sizeof(float), 1, f);
		fread(&dummyFloat, sizeof(float), 1, f);
	}

	//int subOctreesCount = this->m_vec_suboctrees.size(); // this must be 8.***
	unsigned char dummyUChar;
	fread(&dummyUChar, sizeof(unsigned char), 1, f);

	fread(&dummyInt, sizeof(int), 1, f);

	for (unsigned char i = 0; i < dummyUChar; i++)
	{
		readOctreeByDummy(f);
	}
}

bool SmartTile::addData(
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
	Json::Value& attribute)
{
	// check if already-existing data before processing
	size_t alreadyExistingDataMarker;
	bool bAlreadyExist = false;
	for (size_t i = 0; i < dataIds.size(); i++)
	{
		if (dataIds[i] == dataId)
		{
			alreadyExistingDataMarker = i;
			bAlreadyExist = true;
			break;
		}
	}

	// header file
	//// meta data block
	std::string metaDataFullPath = dataPath + std::string("/HeaderAsimetric.hed");
	FILE* file = NULL;
	file = fopen(metaDataFullPath.c_str(), "rb");
	if (file == NULL)
	{
		return false;
	}

	fseek(file, 0, SEEK_END);
	unsigned int metaFileSize = (unsigned int)((unsigned long)ftell(file));
	rewind(file);

	unsigned char* metaDataBuffer = new unsigned char[metaFileSize];
	memset(metaDataBuffer, 0x00, sizeof(unsigned char)*metaFileSize);

	fread(metaDataBuffer, sizeof(unsigned char), metaFileSize, file);

	fclose(file);

	//// re-open header file for detailed meta info
	file = fopen(metaDataFullPath.c_str(), "rb");

	//// basic info
	std::string version;
	char versionArray[6];
	memset(versionArray, 0x00, sizeof(char) * 6);
	fread(versionArray, sizeof(char), 5, file);
	version = std::string(versionArray);

	int dummyInt;
	fread(&dummyInt, sizeof(int), 1, file);
	char dummyArray[StringBufferSize];
	memset(dummyArray, 0x00, StringBufferSize);
	fread(dummyArray, sizeof(char), dummyInt, file);

	double dummyDouble;
	float dummyFloat;
	fread(&dummyDouble, sizeof(double), 1, file);
	fread(&dummyDouble, sizeof(double), 1, file);
	fread(&dummyFloat, sizeof(float), 1, file);

	double minx, miny, minz, maxx, maxy, maxz;
	fread(&minx, sizeof(double), 1, file);
	fread(&miny, sizeof(double), 1, file);
	fread(&minz, sizeof(double), 1, file);
	fread(&maxx, sizeof(double), 1, file);
	fread(&maxy, sizeof(double), 1, file);
	fread(&maxz, sizeof(double), 1, file);

	unsigned short dataType;
	double offsetX, offsetY, offsetZ;
	if (version == std::string("0.0.2"))
	{
		fread(&dataType, sizeof(unsigned short), 1, file);

		fread(&offsetX, sizeof(double), 1, file);
		fread(&offsetY, sizeof(double), 1, file);
		fread(&offsetZ, sizeof(double), 1, file);
	}

	//// octree info
	readOctreeByDummy(file);

	//// material list
	unsigned int materialCount;
	fread(&materialCount, sizeof(unsigned int), 1, file);
	for (unsigned int i = 0; i < materialCount; i++)
	{
		fread(&dummyInt, sizeof(int), 1, file);
		memset(dummyArray, 0x00, StringBufferSize);
		fread(dummyArray, sizeof(char), dummyInt, file);

		fread(&dummyInt, sizeof(int), 1, file);
		memset(dummyArray, 0x00, StringBufferSize);
		fread(dummyArray, sizeof(char), dummyInt, file);
	}

	//// LOD building data
	//// find out names of each lod data with meta data
	std::string lodMeshName(""), lodTextureName("");
	unsigned char buildingDataCount;
	fread(&buildingDataCount, sizeof(unsigned char), 1, file);
	for (unsigned char i = 0; i < buildingDataCount; i++)
	{
		unsigned char lod;
		fread(&lod, sizeof(unsigned char), 1, file);
		bool bModelReference;
		fread(&bModelReference, sizeof(bool), 1, file);

		if (lod == 2)
		{
			unsigned char dummyUChar;
			fread(&dummyUChar, sizeof(unsigned char), 1, file);
			memset(dummyArray, 0x00, StringBufferSize);
			fread(dummyArray, sizeof(char), dummyUChar, file);
		}

		if (!bModelReference)
		{
			unsigned char stringLength;
			char stringArray[StringBufferSize];

			fread(&stringLength, sizeof(unsigned char), 1, file);
			memset(stringArray, 0x00, sizeof(char)*StringBufferSize);
			fread(stringArray, sizeof(char), stringLength, file);

			if(lod == tileLod)
				lodMeshName = std::string(stringArray);

			fread(&stringLength, sizeof(unsigned char), 1, file);
			memset(stringArray, 0x00, sizeof(char)*StringBufferSize);
			fread(stringArray, sizeof(char), stringLength, file);

			if (lod == tileLod)
				lodTextureName = std::string(stringArray);
		}
		else
		{
			if (lod == tileLod)
				lodMeshName = std::string("lod") + std::to_string((unsigned int)lod);
		}
	}

	fclose(file);

	// load mesh data of matched lod
	std::string meshFilePath = dataPath + std::string("/") + lodMeshName;
	file = NULL;
	file = fopen(meshFilePath.c_str(), "rb");
	if (file == NULL)
	{
		delete[] metaDataBuffer;
		return false;
	}

	fseek(file, 0, SEEK_END);
	unsigned int meshFileSize = (unsigned int)((unsigned long)ftell(file));
	rewind(file);

	unsigned char* meshDataBuffer = new unsigned char[meshFileSize];
	memset(meshDataBuffer, 0x00, sizeof(unsigned char)*meshFileSize);
	fread(meshDataBuffer, sizeof(unsigned char), meshFileSize, file);
	fclose(file);

	// load texture data of matched lod mesh
	//std::string textureFilePath = dataPath + std::string("/mosaicTextureLod") + std::to_string((unsigned int)tileLod) + std::string(".jpg");
	std::string textureFilePath = dataPath + std::string("/") + lodTextureName;
	file = NULL;
	file = fopen(textureFilePath.c_str(), "rb");
	if (file == NULL)
	{
		delete[] metaDataBuffer;
		delete[] meshDataBuffer;
		return false;
	}

	fseek(file, 0, SEEK_END);
	unsigned int textureFileSize = (unsigned int)((unsigned long)ftell(file));
	rewind(file);

	unsigned char* textureDataBuffer = new unsigned char[textureFileSize];
	memset(textureDataBuffer, 0x00, sizeof(unsigned char)*textureFileSize);
	fread(textureDataBuffer, sizeof(unsigned char), textureFileSize, file);
	fclose(file);


	// final push
	if (bAlreadyExist)
	{
		dataIds[alreadyExistingDataMarker] = dataId;
		dataGroupIds[alreadyExistingDataMarker] = dataGroupId;
		layerIds[alreadyExistingDataMarker] = layerId;
		buildingIds[alreadyExistingDataMarker] = dataKey;
		dataNames[alreadyExistingDataMarker] = dataName;
		metaDataSizes[alreadyExistingDataMarker] = metaFileSize;
		delete[] metaData[alreadyExistingDataMarker];
		metaData[alreadyExistingDataMarker] = metaDataBuffer;
		lodMeshNames[alreadyExistingDataMarker] = lodMeshName;
		meshDataSizes[alreadyExistingDataMarker] = meshFileSize;
		delete[] meshData[alreadyExistingDataMarker];
		meshData[alreadyExistingDataMarker] = meshDataBuffer;
		imageDataSizes[alreadyExistingDataMarker] = textureFileSize;
		delete[] imageData[alreadyExistingDataMarker];
		imageData[alreadyExistingDataMarker] = textureDataBuffer;
		longitudes[alreadyExistingDataMarker] = longitude;
		latitudes[alreadyExistingDataMarker] = latitude;
		altitudes[alreadyExistingDataMarker] = altitude;
		pitches[alreadyExistingDataMarker] = (float)pitch;
		rolls[alreadyExistingDataMarker] = (float)roll;
		headings[alreadyExistingDataMarker] = (float)heading;
		attributes[alreadyExistingDataMarker] = attribute;
	}
	else
	{
		dataIds.push_back(dataId);
		dataGroupIds.push_back(dataGroupId);
		layerIds.push_back(layerId);
		buildingIds.push_back(dataKey);
		dataNames.push_back(dataName);
		metaDataSizes.push_back(metaFileSize);
		metaData.push_back(metaDataBuffer);
		lodMeshNames.push_back(lodMeshName);
		meshDataSizes.push_back(meshFileSize);
		meshData.push_back(meshDataBuffer);
		imageDataSizes.push_back(textureFileSize);
		imageData.push_back(textureDataBuffer);
		longitudes.push_back(longitude);
		latitudes.push_back(latitude);
		altitudes.push_back(altitude);
		pitches.push_back((float)pitch);
		rolls.push_back((float)roll);
		headings.push_back((float)heading);
		attributes.push_back(attribute);
	}

	return true;
}