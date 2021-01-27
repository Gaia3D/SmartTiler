/**
 * LogWriter Header
 */
#ifndef _LOGWRITER_H_
#define _LOGWRITER_H_
#pragma once

#include <string>
#include <vector>

#define ERROR_FLAG "[ERROR]"
#define WARNING_FLAG "[WARNING]"
#define NO_PATH "[Path not exists]"
#define NO_GEOLOCATION_FILE "[No geolocation file exists]"
#define INVALID_GEOLOCATION_FILE "[Unable to open geolocation file]"
#define FAILURE_READING_GEOLOCATION "[Failed to read geolocation file]"
#define NO_GEOLOCATION_INFO "[No geolocation info]"
#define NO_INPUT_DATA "[No data to be tiled]"
#define DUPLICATED_DATA_ID "[Duplicated data id]"
#define CANNOT_OPEN_FILE "[Unable to open a file]"
#define CANNOT_CREATE_DIRECTORY "[Unable to create a directory]"

class LogWriter
{
private:
	LogWriter();

public:
	unsigned int numberOfJobsToBeDone;

	unsigned int numberOfJobsDone;

private:
	static LogWriter logWriter;

	std::string startTime;

	std::string endTime;

	std::string fullPath;

	bool isSuccess;

public:
	virtual ~LogWriter();

	enum JOB_STATUS
	{
		success,
		warning,
		failure
	};

	static LogWriter *getLogWriter() { return &logWriter; }

	void setFullPath(std::string &path);

	void createNewJobLog(std::string jobName, std::string fullPath);

	void changeCurrentJobStatus(JOB_STATUS jobStatus);

	void addDescriptionToCurrentJobLog(std::string content);

	void closeCurrentJobLog();

	template <typename T>
	void setAdditionalInfoToJop(std::string key, T value);

	template <typename T>
	void setAdditionalInfoToJop(std::string key, std::vector<T> values);

	void setStatus(bool bSuccess, std::string message = std::string());

	void addMessageToLog(std::string content);

	void save();

	void start();

	void finish();

	bool isStarted();

private:
	std::string getCurrentTimeString();
};

#endif // _LOGWRITER_H_
