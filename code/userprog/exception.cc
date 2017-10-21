// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"



// Input: - User space address (int)
//- Limit of buffer (int)
// Output:- Buffer (char*)
// Purpose: Copy buffer from User memory space to System memory space
char* User2System(int virtAddr,int limit)
{
	int i;// index
	int oneChar;
	char* kernelBuf = NULL;
	kernelBuf = new char[limit +1];//need for terminal string
	if (kernelBuf == NULL)
		return kernelBuf;
	memset(kernelBuf,0,limit+1);
	//printf("\n Filename u2s:");
	for (i = 0 ; i < limit ;i++)
	{
		machine->ReadMem(virtAddr+i,1,&oneChar);
		kernelBuf[i] = (char)oneChar;
		//printf("%c",kernelBuf[i]);
		if (oneChar == 0)
		break;
	}
return kernelBuf;
}


// Input: - User space address (int)
//- Limit of buffer (int)
//- Buffer (char[])
// Output:- Number of bytes copied (int)
// Purpose: Copy buffer from System memory space to User memory space
int System2User(int virtAddr,int len,char* buffer)
{
	if (len < 0) return -1;
	if (len == 0)return len;
	int i = 0;
	int oneChar = 0 ;
	do{
		oneChar= (int) buffer[i];
		machine->WriteMem(virtAddr+i,1,oneChar);
		i ++;
	}while(i < len && oneChar != 0);
	return i;
}


//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------





void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

   // Input: reg4 -filename (string)
// Output: reg2 -1: error and 0: success
// Purpose: process the event SC_Create of System call
// mã system call sẽ được đưa vào thanh ghi r2 (có thể xem lại phần xử lý cho
// system call Halt trong tập tin start.s ở trên)
// tham số thứ 1 sẽ được đưa vào thanh ghi r4
// tham số thứ 2 sẽ được đưa vào thanh ghi r5
// tham số thứ 3 sẽ được đưa vào thanh ghi r6
// tham số thứ 4 sẽ được đưa vào thanh ghi r7
// kết quả thực hiện của system call sẽ được đưa vào thanh ghi r2
switch (which) {
case NoException:
return;
case SyscallException:
switch (type){
	case SC_Halt:
		DEBUG('a', "\n Shutdown, initiated by user program.");
		printf ("\n\n Shutdown, initiated by user program.");
		interrupt->Halt();
		break;
	case SC_Create:
	{
		int virtAddr;
		char* filename;
		DEBUG('a',"\n SC_Create call ...");
		DEBUG('a',"\n Reading virtual address of filename");
		// Lấy tham số tên tập tin từ thanh ghi r4
		virtAddr = machine->ReadRegister(4);
		DEBUG ('a',"\n Reading filename.");
		int MaxFileLength = 32;
		filename = User2System(virtAddr,MaxFileLength+1);
		if (filename == NULL)
		{
		printf("\n Not enough memory in system");
		DEBUG('a',"\n Not enough memory in system");
		machine->WriteRegister(2,-1); // trả về lỗi cho chương
		// trình người dùng
		delete filename;
		return;
		}
		DEBUG('a',"\n Finish reading filename.");
		//DEBUG('a',"\n File name : '"<<filename<<"'");
		// Create file with size = 0
		// Dùng đối tượng fileSystem của lớp OpenFile để tạo file,
		// việc tạo file này là sử dụng các thủ tục tạo file của hệ điều
		// hành Linux, chúng ta không quản ly trực tiếp các block trên
		// đĩa cứng cấp phát cho file, việc quản ly các block của file
		// trên ổ đĩa là một đồ án khác
		if (!fileSystem->Create(filename,0))
		{
			printf("\n Error create file '%s'",filename);
			machine->WriteRegister(2,-1);
			delete filename;
			return;
		}
		machine->WriteRegister(2,0); // trả về cho chương trình
		// người dùng thành công
		delete filename;
		break;
	}
	case SC_PrintString : {
		char *buff = new char[9999];
		int buffaddr = machine->ReadRegister(4);
		buff = User2System(buffaddr, 10000);
		//printf("\n %s", buff);
		gSynchConsole->Write(buff, strlen(buff));
		//printf("\n sao ko vao day?");
		DEBUG('a', "\nFinish reading string.");
		delete buff;
		break;		
	}
	
	case SC_ReadFile:
	{
		int bufAddr = machine->ReadRegister(4);
		int sz = machine->ReadRegister(5);
		int index = machine->ReadRegister(6);
		int oldp, newp;
		char *buff = new char[sz];
		if (index < 0 || index > 10){
			machine->WriteRegister(2, -1);
			delete[] buff;
			break;		
		}

		if (fileSystem->openf[index] == NULL){
			machine->WriteRegister(2, -1);
			delete[] buff;
			break;	
		}
		oldp = fileSystem->openf[index]->GetCurrentPos();
		buff = User2System(bufAddr, sz);
		if (fileSystem->openf[index]->type == 2)
		{
			int szByte = gSynchConsole->Read(buff, sz);
			System2User(bufAddr, szByte, buff);
			machine->WriteRegister(2, sz);
			break; 
		}
		
		if ((fileSystem->openf[index]->Read(buff, sz)) > 0){
			newp = fileSystem->openf[index]->GetCurrentPos();
			System2User(bufAddr, newp - oldp + 1, buff);
			machine->WriteRegister(2, newp - oldp + 1);
		}else{
			machine->WriteRegister(2, -1);
			delete[] buff;
			break;
		}
		
		if (buff != NULL)
			delete buff;
		break;
	}
	case SC_WriteFile : {
		int bufAddr = machine->ReadRegister(4);
		int sz = machine->ReadRegister(5);
		int index = machine->ReadRegister(6);
		int oldp, newp;
		char *buff = new char[sz];	

		if (index < 0 || index > 10){
			machine->WriteRegister(2, -1);
			delete[] buff;
			break;		
		}

		if (fileSystem->openf[index] == NULL){
			machine->WriteRegister(2, -1);
			delete[] buff;
			break;	
		}

		oldp = fileSystem->openf[index]->GetCurrentPos();
		buff = User2System(bufAddr, sz);
		if (fileSystem->openf[index]->type  == 0 || fileSystem->openf[index]->type == 3);
			if (fileSystem->openf[index]->type == 1)
			{
				machine->WriteRegister(2,-1);
				delete[] buff;
				break; 
			}else if ((fileSystem->openf[index]->Write(buff, sz)) > 0){
				printf("%s\n", buff);
				newp = fileSystem->openf[index]->GetCurrentPos();
				machine->WriteRegister(2, newp - oldp + 1); 
			}
		
		
		if ((fileSystem->openf[index]->Read(buff, sz)) > 0){
			newp = fileSystem->openf[index]->GetCurrentPos();
			System2User(bufAddr, newp - oldp + 1, buff);
			machine->WriteRegister(2, newp - oldp + 1);
		}else{
			machine->WriteRegister(2, -1);
			delete[] buff;
			break;
		}
		// print data to console
		if (fileSystem->openf[index]->type == 3)
		{
			int i = 0;
			while (buff[i] != 0 && buff[i] != '\n')
			{
				gSynchConsole->Write(buff+i, 1);
					i++;
			}
			buff[i] = '\n';
			gSynchConsole->Write(buff+i,1);
			machine->WriteRegister(2, i-1);
		}		
		if (buff != NULL)
			delete buff;
		break;
	}

	case SC_OpenFileID : {
		int bufAddr = machine->ReadRegister(4); // read string pointer from user
		int type = machine->ReadRegister(5);
		char *buf = new char[255];
		if (fileSystem->index > 10)
		{
			machine->WriteRegister(2, -1);
			delete[] buf;
			break;
		}
		buf = User2System(bufAddr, 255);
		printf("%s\n", buf);
		/*if (strcmp(buf,"stdin") == 0)
		{
			printf("stdin mode\n");
			machine->WriteRegister(2, 0);
			break;
		}
		if (strcmp(buf,"stdout") == 0)
		{
			printf("stdout mode\n");
			machine->WriteRegister(2, 1);
			break;
		}*/
		if ((fileSystem->openf[fileSystem->index] = fileSystem->Open(buf, type)) != NULL)
		{
			DEBUG('f',"open file successfully");
			machine->WriteRegister(2, fileSystem->index-1);
		} else 
		{
			DEBUG('f',"can not open file");
			machine->WriteRegister(2, -1);
		};
		delete [] buf;
		break;		
	}	
	default:
		printf("\n Unexpected user mode exception (%d %d)", which,
		type);
		interrupt->Halt();
	}
	
	// Advance program counters.
    	machine->registers[PrevPCReg] = machine->registers[PCReg];
    	machine->registers[PCReg] = machine->registers[NextPCReg];
    	machine->registers[NextPCReg] += 4;
}

}
