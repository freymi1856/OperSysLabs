#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <queue>
#include <stdexcept>
#include <memory>

#include "json-lib/json/single_include/nlohmann/json.hpp"

using json = nlohmann::json;

struct Job {
    std::string name;
    std::vector<std::string> dependencies;
    bool completed = false;
    bool failed = false;
};

class DAGExecutor {
public:
    DAGExecutor(const std::string& filename) {/////
        parseConfig(filename);
        if (!validateDAG()) {
            throw std::runtime_error("Cycle detected in DAG.");
        }
        if (!isConnected()) {
            throw std::runtime_error("DAG is not connected.");
        }
        if (!hasStartAndEndJobs()) {
            throw std::runtime_error("No start or end job found.");
        }
    }

    void execute() {
        std::vector<std::thread> threads;
        std::queue<std::string> readyQueue;

        // Initialize ready queue with jobs that have no dependencies
        for (auto& job : jobs) {
            if (job->dependencies.empty()) {
                readyQueue.push(job->name);
            }
        }

        // Start threads for all jobs
        for (auto& job : jobs) {
            threads.emplace_back(&DAGExecutor::runJob, this, job->name);
        }

        // Main loop to process ready jobs
        while (!readyQueue.empty()) {
            std::string jobName = readyQueue.front();
            readyQueue.pop();

            // Mark job as completed
            {
                std::lock_guard<std::mutex> lock(mtx);
                jobsByName[jobName]->completed = true;
                std::cout << "Job " << jobName << " completed successfully." << std::endl;
            }

            // Signal dependent jobs
            for (auto& dependent : adjList[jobName]) {
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    if (--inDegree[dependent] == 0) {
                        readyQueue.push(dependent);
                    }
                }
            }
        }

        // Wait for all threads to finish
        for (auto& thread : threads) {
            if (thread.joinable()) {
                thread.join();
            }
        }

        // Check if any job failed
        if (failureFlag) {
            std::cerr << "DAG execution failed due to job failure." << std::endl;
            exit(1);
        } else {
            std::cout << "DAG execution completed successfully." << std::endl;
        }
    }

private:
    std::vector<std::unique_ptr<Job>> jobs;
    std::unordered_map<std::string, std::vector<std::string>> adjList;
    std::unordered_map<std::string, int> inDegree;
    std::unordered_map<std::string, Job*> jobsByName;
    std::mutex mtx;
    std::atomic<bool> failureFlag{false};

    void parseConfig(const std::string& filename) {/////
        std::ifstream file(filename);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open file: " + filename);
        }
        json j;
        file >> j;

        // Parse jobs and dependencies
        for (auto& elem : j["jobs"]) {
            auto job = std::make_unique<Job>();
            job->name = elem["name"];
            job->dependencies = elem["dependencies"];
            jobs.push_back(std::move(job));
        }

        // Build adjacency list and in-degree map
        for (auto& job : jobs) {
            adjList[job->name] = {};
            inDegree[job->name] = 0;
        }

        for (auto& job : jobs) {
            for (auto& dep : job->dependencies) {
                adjList[dep].push_back(job->name);
                inDegree[job->name]++;
            }
        }

        // Populate jobsByName
        for (auto& job : jobs) {
            jobsByName[job->name] = job.get();
        }
    }

    
    bool validateDAG() {/////////
        std::queue<std::string> queue;
        std::unordered_map<std::string, int> localInDegree = inDegree;

        // Initialize queue with jobs that have no dependencies
        for (auto& job : jobs) {
            if (localInDegree[job->name] == 0) {
                queue.push(job->name);
            }
        }

        int count = 0;
        while (!queue.empty()) {
            std::string u = queue.front();
            queue.pop();
            count++;

            for (auto& v : adjList[u]) {
                if (--localInDegree[v] == 0) {
                    queue.push(v);
                }
            }
        }

        // If count != jobs.size(), то есть цикл
        return count == jobs.size();
    }

    bool isConnected() {//////////
        std::unordered_set<std::string> visited;
        std::string startJob = "";

        // Find a start job (no dependencies)
        for (auto& job : jobs) {
            if (job->dependencies.empty()) {
                startJob = job->name;
                break;
            }
        }

        if (startJob.empty()) {
            return false; // No start job found
        }

        dfs(startJob, visited);
        return visited.size() == jobs.size();
    }

    void dfs(const std::string& u, std::unordered_set<std::string>& visited) {//////////
        visited.insert(u);
        for (auto& v : adjList[u]) {
            if (visited.find(v) == visited.end()) {
                dfs(v, visited);
            }
        }
    }

    bool hasStartAndEndJobs() {///////
        int startCount = 0;
        int endCount = 0;

        // Check for start jobs (no dependencies)
        for (auto& job : jobs) {
            if (job->dependencies.empty()) {
                startCount++;
                std::cout << "Start job: " << job->name << std::endl;
            }
        }

        // Check for end jobs (no dependents)
        for (auto& job : jobs) {
            if (adjList[job->name].empty()) {
                endCount++;
                std::cout << "End job: " << job->name << std::endl;
            }
        }

        return startCount > 0 && endCount > 0;
    }

    void runJob(const std::string& name) {////////////////////
        try {
            // Simulate job execution
            std::this_thread::sleep_for(std::chrono::seconds(1));
            std::lock_guard<std::mutex> lock(mtx);
            jobsByName[name]->completed = true;
            std::cout << "Job " << name << " completed successfully." << std::endl;
        } catch (...) {
            std::lock_guard<std::mutex> lock(mtx);
            std::cerr << "Job " << name << " failed." << std::endl;
            jobsByName[name]->failed = true;
            failureFlag = true;
        }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file.json>" << std::endl;
        return 1;
    }

    try {
        DAGExecutor executor(argv[1]);
        executor.execute();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}