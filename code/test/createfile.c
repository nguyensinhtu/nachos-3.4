#include "syscall.h"
#include "copyright.h"
#define maxlen 32
int
main()
{
	/*int len;
	char filename[maxlen +1];
	//Create("text.txt");
	//Create a file
	if (Create("hello.txt") == -1)
	{
		PrintString("failed to create new file\n");
	}
	else
	{
		PrintString("create file success\n");	
	}*/
	int isCreate = Create("hello");
	char mess[255];
	OpenFileID open; 
	if (isCreate == -1)
	{
		PrintString("Can not create file \'hello.txt\'\n");
	} 
	else
	{
		PrintString("Successfully create file\n");
		open = OpenFile("hello", 0);
		if (open==-1) PrintString("Can not open file\n");
		WriteFile("123", 3, open);
		//SeekFile(0, open);
		//ReadFile(mess, 3, open);
	  PrintString(mess);
		//CloseFile(open);
	}
	
	return 0;
	Halt();
}

