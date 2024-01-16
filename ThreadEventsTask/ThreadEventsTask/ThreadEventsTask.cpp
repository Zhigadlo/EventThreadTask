#include <iostream>
#include <chrono>
#include <ctime>
#include <random>
#include <thread>
#include <string.h>
#include <fstream>
#include <map>

int idCounter = 0;
std::string command;
const char* loggingFilePath = "../ThreadEventsTask/log.txt";

struct Event {
	int id;
	std::string date;
	std::string time;
	int firstVal;
	int secondVal;
	int thirdVal;
};

class Logger {
protected:
	std::ofstream file;
	Logger(const char* fileName) {
		file.open(fileName, std::ios::app);
		if (!file.is_open()) {
			std::cerr << "Fail to open " << fileName << std::endl;
		}
	}
	~Logger() {
		if (file.is_open()) {
			file.close();
		}
	}

public:
	virtual void Write(Event event) = 0;
};

class Level0Logger : public Logger {
public:
	Level0Logger(const char* fileName) : Logger(fileName) {}

	void Write(Event event) override {
		file << "level 0" << std::endl;
		file << "id: " << event.id << std::endl;
		file << "time: " << event.time << std::endl;
		file << std::endl;
	}
};
class Level1Logger : public Logger {
public:
	Level1Logger(const char* fileName) : Logger(fileName) {}
	
	void Write(Event event) override {
		file << "level 1" << std::endl;
		file << "id: " << event.id << std::endl;
		file << "date: " << event.date << std::endl;
		file << "time: " << event.time << std::endl;
		file << "first value: " << event.firstVal << std::endl;
		file << std::endl;
	}
};
class Level2Logger : public Logger {
public:
	Level2Logger(const char* fileName) : Logger(fileName) {}

	void Write(Event event) override {
		file << "level 2" << std::endl;
		file << "id: " << event.id << std::endl;
		file << "date: " << event.date << std::endl;
		file << "time: " << event.time << std::endl;
		file << "first value: " << event.firstVal << std::endl;
		file << "second value: " << event.secondVal << std::endl;
		file << "third value: " << event.thirdVal << std::endl;
		file << std::endl;
	}
};

Event* CreateEvent()
{
	std::time_t rawtime;
	std::time(&rawtime);
	std::tm timeinfo;
	localtime_s(&timeinfo, &rawtime);

	char date[11];
	strftime(date, sizeof(date), "%d.%m.%Y", &timeinfo);

	char time[9];
	strftime(time, sizeof(time), "%H:%M:%S", &timeinfo);

	// Создание генератора случайных чисел
	std::random_device rd;
	std::mt19937 gen(rd());

	// Создание равномерного распределения от -50 до 50
	std::uniform_int_distribution<> distr(-50, 50);

	int firstVal = distr(gen);
	int secondVal = distr(gen);
	int thirdVal = distr(gen);

	return new Event{ idCounter++, date, time, firstVal, secondVal, thirdVal };
}

std::atomic<Logger*> logger = new Level2Logger(loggingFilePath);
std::atomic<Event*> event = CreateEvent();

int secondsBetweenEvents = 5; 

typedef void (*Routine)();
bool isEnd = false;
bool isGeneratingPaused = false;

std::map<std::string, Routine> commandMap;

void exit() {
	isEnd = true;
}
void timeOutput() {
	std::cout << (*event).time << std::endl;
}
void dateOutput() {
	std::cout << (*event).date << std::endl;
}
void faster() {
	if (secondsBetweenEvents - 1 <= 0) {
		std::cerr << "Time between events can't be less than zero";
		return;
	}

	secondsBetweenEvents -= 1;
	std::cout << "Now time between events equals " << secondsBetweenEvents << std::endl;
}
void slower() {
	secondsBetweenEvents += 1;
	std::cout << "Now time between events equals " << secondsBetweenEvents << std::endl;
}
void pause() {
	isGeneratingPaused = true;
	std::cout << "Events generating is stopped" << std::endl;
}
void resume() {
	isGeneratingPaused = false;	
	std::cout << "Events generating is resumed" << std::endl;
}
void level0() {
	logger.exchange(new Level0Logger(loggingFilePath));
	std::cout << "Loggin level 0" << std::endl;
}
void level1() {
	logger.exchange(new Level1Logger(loggingFilePath));
	std::cout << "Loggin level 1" << std::endl;
}
void level2() {
	logger.exchange(new Level2Logger(loggingFilePath));
	std::cout << "Loggin level 2" << std::endl;
}
void stat() {
	std::cout << "Generated events " << (*event).id + 1 << std::endl;
}

void FillMap() {
	commandMap["date"] = dateOutput;
	commandMap["time"] = timeOutput;
	commandMap["exit"] = exit;
	commandMap["faster"] = faster;
	commandMap["slower"] = slower;
	commandMap["pause"] = pause;
	commandMap["resume"] = resume;
	commandMap["level0"] = level0;
	commandMap["level1"] = level1;
	commandMap["level2"] = level2;
	commandMap["stat"] = stat;
}

void InputHandle() {
	std::string command;
	while (!isEnd) {
		std::cin >> command;

		if (commandMap.find(command) != commandMap.end()) {
			commandMap[command]();
			continue;
		}

		std::cerr << "There is no such command..." << std::endl;
	}
}

void GenerateEvents() {
	while (!isEnd) {
		if (!isGeneratingPaused) {
			Event* newEvent = CreateEvent();
			event.exchange(newEvent);
			std::this_thread::sleep_for(std::chrono::seconds(secondsBetweenEvents));
		}
	}
}

void LogEvents() {
	int previousId = 0;
	while (!isEnd) {
		Event e = *event;
		//проверка на запись одного и того же события
		if (e.id != previousId) {
			Logger* l = logger;
			l->Write(*event);
			//std::cout << e.id << std::endl;
			previousId = (*event).id;
		}
	}
}

int main()
{
	FillMap();

	std::thread userInputThread(InputHandle);
	std::thread eventGenerationThread(GenerateEvents);
	std::thread eventLoggingThread(LogEvents);

	userInputThread.join();
	eventGenerationThread.join();
	eventLoggingThread.join();

	std::cout << "Press any key to close the programm" << std::endl;
	std::getchar();
	return 0;
}