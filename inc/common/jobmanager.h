#pragma once

#include <vector>
#include "renderer.h"

struct Job;

struct JobManager{
	static JobManager& getInstance();

	//Return value returned from XML file parser
	enum class xmlParseError {
		JOB_NO_ERROR,
		JOB_RESOURCE_UNAVAILABLE,
        JOB_SYNTAX_ERROR,
        JOB_NO_JOB,
        JOB_NO_METADATA,
        JOB_NO_VERSION,
        JOB_VERSION_MISMATCH,
        JOB_NO_REFERRER,
        JOB_REFERRER_NOT_VALID,
        JOB_NO_AUTH_CODE,
        JOB_AUTH_CODE_INVALID,
        JOB_NO_GUID,
        JOB_GUID_INVALID,
        JOB_NO_STYLE,
        JOB_NO_LAYERS,
        JOB_NO_AREAS
	};

	std::vector<Job*> jobs;

	struct ThreadParameters{
		Renderer* renderer;
		Job::Area area;
	};

	//Parses an XML Job description, puts it int the jobs
	JobManager::xmlParseError addJob(const char* filePath);

	//If system has resources, takes te next job out of the vector
	//and processes it in a seperate thread
	static void* startNextJob(void* threadParameters);

private:
	static JobManager* instance;
	JobManager();
	~JobManager();
	JobManager(JobManager const&)		= delete;
	void operator=(JobManager const&)	= delete;
};
