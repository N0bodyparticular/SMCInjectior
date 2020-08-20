#include <iostream>
#include <Windows.h>
#include <string>
#include <thread>
#include <libloaderapi.h>


void getProcId(const char* windowTitle, DWORD &processId) {
	GetWindowThreadProcessId(FindWindow(NULL, windowTitle), &processId);
}

void errorDisplay(const char* errorTitle, const char* errorMessage) {
	//MessageBox(NULL, errorMessage, errorTitle, NULL);
	printf("ERROR: %s\n", errorMessage);
	exit(-1);
}

bool fileExists(std::string fileName) {
	struct stat buffer;
	return (stat(fileName.c_str(), &buffer) == 0);
}

int main(int argc, char *argv[])
{
    printf("Hello World!\nWelcome to SMCInjector made by SMC.\n"); 	

	DWORD procId = NULL;   // Process ID
	char dllPath[MAX_PATH];// Full path to DLL
	const char* dll_name = NULL;  // Name of DLL to inject
	const char* window_title = NULL;// Title of window.
	bool usePID = false;
	long pid;


	if (argc < 3) {
		printf("ERROR: Two command line paramaters are expected. \nUsage: %s [DLL Name] [Window Title] or Usage: %s [DLL Name] [Process ID] pid.\n", argv[0]);
		exit(-2);
	}
	else if (argc == 3) {
		dll_name = argv[1];
		window_title = argv[2]; // Load the file and application names from argumnt variables.
		usePID = false;
	}
	else if ((argc == 4) & argv[3] == "pid") {
		dll_name = argv[1];
		pid = atoi(argv[2]);
		usePID = true;
	}

	printf("DLL Name: %s. Application Name: %s. Beginning Injection.\n", argv[1], argv[2]);

	if (usePID == false) {
		printf("Checking for DLL File.\n");
		if (!fileExists(dll_name)) {
			errorDisplay("File not found.", "DLL File not found!");
		}

		printf("Finding full path to DLL.\n");
		if (!GetFullPathName(dll_name, MAX_PATH, dllPath, nullptr)) {
			errorDisplay("File not found!", "Failed to get full path to DLL!");
		}

		printf("Finding process ID.\n");
		getProcId(window_title, procId);

		if (procId == NULL) {
			errorDisplay("Process ID", "Failed to get process ID!");
		}
	}
	else {
		procId = pid;
	}

	printf("Getting a process handle.\n");
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, NULL, procId);

	if (!hProcess) {
		errorDisplay("Process Handle Fail", "Failed to get a process handle!");
	}

	printf("Allocating memory.\n");
	void * allocatedMemory = VirtualAllocEx(hProcess, nullptr, MAX_PATH, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

	if (!allocatedMemory) {
		errorDisplay("Allocated Memory", "Failed to allocate process memory");
	}

	printf("Writing process memory.\n");
	if (!WriteProcessMemory(hProcess, allocatedMemory, dllPath, MAX_PATH, nullptr)) {
		errorDisplay("Process Memory", "Failed to write to process memory!");
	}

	printf("Creating Remote Thread.\n");
	HANDLE hThread = CreateRemoteThread(hProcess, nullptr, NULL, LPTHREAD_START_ROUTINE(LoadLibraryA), allocatedMemory, NULL, nullptr);

	if (!hThread) {
		errorDisplay("Thread Creation", "Failed to create remote thread!");
	}


	printf("Cleaning up.\n");
	CloseHandle(hProcess);
	VirtualFreeEx(hProcess, allocatedMemory, NULL, MEM_RELEASE);
	printf("Done. Have a nice day.");
}

