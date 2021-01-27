/**
 * PreDefinition Header
 */
#ifndef _PREDEFINITION_H_
#define _PREDEFINITION_H_
#pragma once

///< basic data prefix
#define BasicDataPrefix "F4D_"

///< meta file name of data
#define MetaFileName "HeaderAsimetric.hed"

///< intial tile depth
#define InitialTileDepth 12

///< thresholds of data bbox max length for matching this data to appropriate tile depth 
#define lengthThresholdsForTileDepth {200.0, 100.0, 50.0, 30.0, 10.0, 0.5}

///< threshold count
#define ThresholdCount 6

///< rouphest LOD
#define LodMax 5

///< most detailed LOD
#define LodMin 3

#endif // _PREDEFINITION_H_