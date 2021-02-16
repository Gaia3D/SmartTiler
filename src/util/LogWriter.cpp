/**
 * Implementation of the LogWriter class
 */
#include "LogWriter.h"

#include <filesystem>
#include <fstream>
#include <chrono>
#include <ctime>
#include <cstring>

#include <json/json.h>

LogWriter LogWriter::logWriter;
Json::Value logObject(Json::objectValue);
Json::Value currentJob(Json::objectValue);
bool isJobGoing = false;

LogWriter::LogWriter()
{
	numberOfJobsToBeDone = 0;

	numberOfJobsDone = 0;

	isSuccess = true;
}

LogWriter::~LogWriter()
{
}

///< Set full path of the logfile
void LogWriter::setFullPath(std::string path)
{
	namespace fs = std::filesystem;
	fs::path fullPath(path);
	fullPath = fullPath.lexically_normal();

	if (fs::is_directory(fullPath))
	{
		logFilePath = fullPath.string();
		logFileName = "";
	}
	else
	{
		logFilePath = fullPath.parent_path().string();
		logFileName = fullPath.filename().string();
	}
}

///< Set path where log is to be written
void LogWriter::setPath(std::string path)
{
	logFilePath = path;
}

///< Set log file name
void LogWriter::setFileName(std::string name)
{
	logFileName = name;
}

///< Set log file name
void LogWriter::setFileNameByPrefix(std::string name)
{
	logFileName = logFileNamePrefix + name + std::string(".txt");
}

///< Set log file name prefix
void LogWriter::setFileNamePrefix(std::string prefix)
{
	logFileNamePrefix = prefix;
}

bool LogWriter::isEmptyFileName()
{
	return logFileName.empty();
}

///< save the result log
void LogWriter::save()
{
	Json::StyledWriter writer;
	std::string documentContent = writer.write(logObject);
	FILE *file = NULL;
	if (logFileName.empty())
	{
		logFileName = "log_smartTile.txt";
	}

	std::string fullPath = logFilePath + std::string("/") + logFileName;
	file = fopen(fullPath.c_str(), "wt");

	if (file == NULL)
	{
		printf("[Error] Can't create log file.");
		logObject.clear();
		currentJob.clear();
		return;
	}

	fprintf(file, "%s", documentContent.c_str());
	fclose(file);

	logObject.clear();
	currentJob.clear();
}

///< Record status about running this application
void LogWriter::setStatus(bool bSuccess, std::string message)
{
	isSuccess = bSuccess;
	logObject["isSuccess"] = isSuccess;
	if (!bSuccess)
	{
		if (logObject["log"].asString().empty())
			logObject["log"] = message;
		else
			logObject["log"] = logObject["log"].asString() + std::string(" | ") + message;
	}
}

void LogWriter::addMessageToLog(std::string content)
{
	if (logObject["log"].asString().empty())
		logObject["log"] = content;
	else
		logObject["log"] = logObject["log"].asString() + std::string(" | ") + content;
}

///< Record the start time of all jobs
void LogWriter::start()
{
	startTime = getCurrentTimeString();
	logObject["startTime"] = startTime;
	logObject["numberOfJobsToBeDone"] = 0;
	logObject["numberOfJobsDone"] = 0;
	logObject["isSuccess"] = true;
	logObject["jobResult"] = Json::Value(Json::arrayValue);
	logObject["log"] = std::string("");
}

///< Record the finish time of all jobs
void LogWriter::finish()
{
	endTime = getCurrentTimeString();
	logObject["endTime"] = endTime;
	logObject["numberOfJobsToBeDone"] = numberOfJobsToBeDone;
	logObject["numberOfJobsDone"] = numberOfJobsDone;
}

///< Check application is started or not
bool LogWriter::isStarted()
{
	return !(startTime.empty());
}

///< Get current system time
std::string LogWriter::getCurrentTimeString()
{
	std::chrono::time_point<std::chrono::system_clock> nowTime = std::chrono::system_clock::now();
	std::time_t currentTime = std::chrono::system_clock::to_time_t(nowTime);

	char timeStringLine[256];
	memset(timeStringLine, 0x00, 256);
	std::strftime(timeStringLine, 256, "%Y-%m-%d %H:%M:%S", std::localtime(&currentTime));

	std::string timeString(timeStringLine);

	return timeString;
}

///< Create a new job
void LogWriter::createNewJobLog(std::string jobName, std::string fullPath)
{
	if (isJobGoing)
		return;

	currentJob["jobName"] = jobName;
	currentJob["fullPath"] = fullPath;
	currentJob["startTime"] = getCurrentTimeString();
	currentJob["resultStatus"] = std::string("success");
	currentJob["message"] = std::string("");

	numberOfJobsToBeDone++;

	isJobGoing = true;
}

///< Change result stauts of current job
void LogWriter::changeCurrentJobStatus(JOB_STATUS jobStatus)
{
	if (!isJobGoing)
		return;

	switch (jobStatus)
	{
	case success:
		currentJob["resultStatus"] = std::string("success");
		break;
	case warning:
		currentJob["resultStatus"] = std::string("warning");
		break;
	case failure:
		currentJob["resultStatus"] = std::string("failure");
		break;
	}
}

///< add any message to current job
void LogWriter::addDescriptionToCurrentJobLog(std::string content)
{
	if (!isJobGoing)
		return;

	if (currentJob["message"].asString().empty())
		currentJob["message"] = content;
	else
		currentJob["message"] = currentJob["message"].asString() + std::string(" | ") + content;
}

///< close current job
void LogWriter::closeCurrentJobLog()
{
	if (!isJobGoing)
		return;

	currentJob["endTime"] = getCurrentTimeString();
	logObject["jobResult"].append(currentJob);

	if (currentJob["resultStatus"] != std::string("failure"))
		numberOfJobsDone++;

	currentJob.clear();

	isJobGoing = false;
}
