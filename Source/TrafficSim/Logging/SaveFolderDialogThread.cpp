#include "SaveFolderDialogThread.h"

#include "Engine.h"
//Needed for Win32 Save dialog
#ifdef _WIN32
#include <Windows.h>
#include <ShlObj.h>
#endif

//Needed for Unreal to compile
FSaveFolderDialogThread* FSaveFolderDialogThread::currentInstance = nullptr;

FSaveFolderDialogThread::FSaveFolderDialogThread() {
	//Create the thread
	runningThread = FRunnableThread::Create(this, TEXT("FSaveFolderDialogRunnable"), 0, TPri_BelowNormal);
}

FSaveFolderDialogThread::~FSaveFolderDialogThread() {
	//Clear the running thread
	delete runningThread;
	runningThread = nullptr;
}

//The following must be defined for the FRunnable interface
bool FSaveFolderDialogThread::Init() {
	return true;
}

uint32 FSaveFolderDialogThread::Run() {
	showDialog();
	Stop();

	return 0;
}

void FSaveFolderDialogThread::Stop() {
	stopTaskCounter.Increment();
	bIsFinished = true;
}

//Ensures that the thread has properly stopped
void FSaveFolderDialogThread::EnsureFinished() {
	Stop();
	runningThread->WaitForCompletion();
}

//Starts the thread. Static so we can easilly access it, and that only one dialog exists at a time.
FSaveFolderDialogThread* FSaveFolderDialogThread::startThread() {
	//Create a new instance of the thread if it doesn't exist.
	if (currentInstance == nullptr || currentInstance->IsFinished()) {
		currentInstance = new FSaveFolderDialogThread();
	}

	return currentInstance;
}

//Stop the thread. Static so it can easily be called
void FSaveFolderDialogThread::stopThread() {
	if (currentInstance != nullptr) {
		currentInstance->EnsureFinished();
		delete currentInstance;
		currentInstance = nullptr;
	}
}

//Check if the thread is finished from any other thread
bool FSaveFolderDialogThread::isThreadFinished() {
	if (currentInstance != nullptr) {
		return currentInstance->IsFinished();
	} else {
		return true;
	}
}

#ifdef _WIN32
//Callback for Win32 API folder picker dialog
static int CALLBACK FolderPickerCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData) {
	if (uMsg == BFFM_INITIALIZED) {
		SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
	}

	return 0;
}
#endif

//Shows the actual save dialog
void FSaveFolderDialogThread::showDialog() {
	bool fileOpened = false;

#ifdef _WIN32
	TCHAR path[MAX_PATH];

	const char * path_param = "";

	BROWSEINFO bi = { 0 };
	bi.lpszTitle = (LPCWSTR) L"Select a folder to log data to...";
	bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
	bi.lpfn = FolderPickerCallbackProc;
	bi.lParam = (LPARAM)path_param;

	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

	if (pidl != 0) {
		//get the name of the folder and put it in path
		SHGetPathFromIDList(pidl, path);

		//free memory used
		IMalloc * imalloc = 0;
		if (SUCCEEDED(SHGetMalloc(&imalloc))) {
			imalloc->Free(pidl);
			imalloc->Release();
		}

		fileOpened = true;
		selectedFolder = FString(path);
	}
#endif

	if (fileOpened) {
		//Do stuff with file here!
		
	}
}

FString FSaveFolderDialogThread::getSelectedFolder() {
	FString toReturn;
	if (currentInstance != nullptr && currentInstance->IsFinished()) {
		toReturn = currentInstance->selectedFolder;
	}

	return toReturn;
}
