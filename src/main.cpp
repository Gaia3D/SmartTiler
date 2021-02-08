#ifdef _WIN32
#define APIENTRY __stdcall
#endif

#include <filesystem>

#include "argdefinition.h"
#include "util/LogWriter.h"
#include "tiler/TilingProcessor.h"

bool extractArguments(int argc, char **argv, std::map<std::string, std::string> &arguments)
{
	std::vector<std::string> tokens;

	arguments[ProgramPath] = std::string(argv[0]);
	for (int i = 1; i < argc; i++)
	{
		tokens.push_back(std::string(argv[i]));
	}

	size_t tokenCount = tokens.size();
	if (tokenCount % 2 != 0)
	{
		printf("[ERROR][Invalid Arguments] Mismatched Key-Value pair\n");
		return false;
	}

	for (size_t i = 0; i < tokenCount; i++)
	{
		if (tokens[i] == std::string(InputFile))
		{
			arguments[InputFile] = tokens[i + 1];
			i++;
			continue;
		}

		if (tokens[i] == std::string(InputFolder))
		{
			arguments[InputFolder] = tokens[i + 1];
			i++;
			continue;
		}

		if (tokens[i] == std::string(OutputFolder))
		{
			arguments[OutputFolder] = tokens[i + 1];
			i++;
			continue;
		}

		if (tokens[i] == std::string(LogFilePath))
		{
			arguments[LogFilePath] = tokens[i + 1];
			i++;
			continue;
		}

		if (tokens[i] == std::string(GeolocationPath))
		{
			arguments[GeolocationPath] = tokens[i + 1];
			i++;
			continue;
		}

		if (tokens[i] == std::string(CreateIndex))
		{
			arguments[CreateIndex] = tokens[i + 1];
			i++;
			continue;
		}

		if (tokens[i] == std::string(DataGroupKey))
		{
			arguments[DataGroupKey] = tokens[i + 1];
			i++;
			continue;
		}
	}

	if (arguments.find(OutputFolder) == arguments.end())
	{
		printf("[ERROR][Invalid Arguments] -outputFolder MUST be entered.\n");
		return false;
	}

	if (arguments.find(InputFile) == arguments.end())
	{
		if (arguments.find(InputFolder) == arguments.end() && arguments.find(CreateIndex) == arguments.end())
		{
			printf("[ERROR][Invalid Arguments] -inputFolder or -indexing or both MUST be entered.\n");
			return false;
		}

		if (arguments.find(InputFolder) != arguments.end() && arguments.find(GeolocationPath) == arguments.end())
		{
			printf("[ERROR][Invalid Arguments] -geolocationPath MUST be entered when -inputFolder is used.\n");
			return false;
		}

		if (arguments.find(InputFolder) != arguments.end())
		{
			if (arguments.find(DataGroupKey) == arguments.end())
			{
				printf("[ERROR][Invalid Arguments] -dataGroupKey MUST be entered when -inputFolder is used.\n");
				return false;
			}
			else
			{
				if (arguments[DataGroupKey].size() == 0)
				{
					printf("[ERROR][Invalid Arguments] value of -dataGroupKey is zero.\n");
					return false;
				}
			}
		}
	}

	if (arguments.find(CreateIndex) != arguments.end())
	{
		if (arguments[CreateIndex] == std::string("Y") ||
			arguments[CreateIndex] == std::string("y") ||
			arguments[CreateIndex] == std::string("N") ||
			arguments[CreateIndex] == std::string("n"))
		{
		}
		else
		{
			printf("[ERROR][Invalid Arguments] Value of -indexing MUST be one of [Y, y, N, n].\n");
			return false;
		}
	}

	if (arguments.find(LogFilePath) == arguments.end())
	{
		printf("[ERROR][Invalid Arguments] -log MUST be entered.\n");
		return false;
	}

	return true;
}

int main(int argc, char *argv[])
{
	///< basic argument validation
	if (argc < 2)
	{
		printf("[Error]No argument.\n");
		return -1;
	}

	///< extract arguments
	std::map<std::string, std::string> arguments;
	if (!extractArguments(argc, argv, arguments))
	{
		printf("[Error]Invalid Arguments.\n");
		return -2;
	}

	///< arguments screen log
	printf("[Info]Arguments are following.\n");
	std::map<std::string, std::string>::iterator iter = arguments.begin();
	for (; iter != arguments.end(); iter++)
	{
		printf("%s : %s\n", iter->first.c_str(), iter->second.c_str());
	}

	///< start log writer if needed
	if (arguments.find(LogFilePath) != arguments.end())
	{
		namespace fs = std::filesystem;

		std::string logFile = arguments[LogFilePath];
		
		if (!fs::exists(fs::path(logFile)))
		{
			printf("[Error]The Path for log file does NOT exist.\n");
			return -3;
		}

		LogWriter::getLogWriter()->start();
		LogWriter::getLogWriter()->setFullPath(logFile);
		LogWriter::getLogWriter()->setFileNamePrefix(std::string("log_smartTile_"));
	}

	///< process
	if (TilingProcessor::getProcessor()->initialize(arguments))
	{
		TilingProcessor::getProcessor()->process();
	}

	///< finish and save log if log writer started
	if (LogWriter::getLogWriter()->isStarted())
	{
		LogWriter::getLogWriter()->finish();
		LogWriter::getLogWriter()->save();
	}

	return 0;
}