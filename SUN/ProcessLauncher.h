#ifndef SUN_INCLUDE__UNKNOWNSTRUCT_H
#define SUN_INCLUDE__UNKNOWNSTRUCT_H

class ProcessLauncher 
{
	public:
		static const UINT BUFFER_SIZE = 256;

		ProcessLauncher();
		void Create();
		bool Watch_Process(LPDWORD ExitCode);

		char FirstArgument[BUFFER_SIZE];
		char SecondArgument[BUFFER_SIZE];
		char ThirdArgument[BUFFER_SIZE];
		HANDLE Process;
		HANDLE ProcessThread;
};

#endif