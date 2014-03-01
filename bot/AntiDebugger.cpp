// AntiDebugger.cpp : 定義主控台應用程式的進入點。
//

#include "stdafx.h"
#include <stdio.h>
#include <windows.h>
#include <tlhelp32.h> 

BOOL AD_IsDebuggerPresent()
{
	return IsDebuggerPresent();
}

BOOL AD_PEB_IsDebugged()
{
	__asm {
		xor eax, eax
		mov ebx, fs:[30h]
		mov al, byte ptr [ebx+2]
	}
}

BOOL AD_PEB_NtGlobalFlags()
{
	__asm {
		mov eax, fs:[30h]
		mov eax, [eax+68h]
		and eax, 0x70
	}
}

BOOL AD_PEB_HeapFlags()
{
	// ****** YOUR CODE HERE ********* //
	__asm {
		cmp  al, 6
		cmc
		sbb  ebx, ebx
		and  ebx, 34h
		mov  eax, fs:[30h] ;Process Environment Block
		mov  eax, [eax+18h] ;get process heap base
		mov eax, [eax+ebx+0ch] ;Flags
		;neither HEAP_CREATE_ALIGN_16
		;nor HEAP_SKIP_VALIDATION_CHECKS
		and  eax, 0effeffffh
		;HEAP_GROWABLE
		;+ HEAP_TAIL_CHECKING_ENABLED
		;+ HEAP_FREE_CHECKING_ENABLED
		;+ HEAP_VALIDATE_PARAMETERS_ENABLED
		cmp  eax, 40000062h
		je  choke_true
		jmp choke_false
	}

choke_true:
	return true;

choke_false:
	return false;
}


BOOL AD_CheckRemoteDebuggerPresent()
{
	// ****** YOUR CODE HERE ********* //
	FARPROC Func_addr ;
	HMODULE hModule = GetModuleHandle("kernel32.dll");
	
	if (hModule==INVALID_HANDLE_VALUE)
		return false;
	
	(FARPROC&) Func_addr =GetProcAddress(hModule, "CheckRemoteDebuggerPresent");

	if (Func_addr != NULL) {
		__asm {
			push  eax;
			push  esp;
			push  0xffffffff;
			call  Func_addr;
			test  eax,eax;
			je    choke_false;
			pop    eax;
			test  eax,eax
			je    choke_false;
			jmp    choke_true;
		}
	}

choke_true:
	return true;

choke_false:
	return false;
}


LONG WINAPI ad_excp_handler( struct _EXCEPTION_POINTERS *ExceptionInfo ) 
{
	// ****** YOUR CODE HERE ********* //
	return true;
}

BOOL AD_UnhandleException()
{
	// ****** YOUR CODE HERE ********* //
	return true;
}

BOOL AD_CloseHandle()
{
	// ****** YOUR CODE HERE ********* //
	return true;
}

BOOL AD_PageGuard()
{
	// ****** YOUR CODE HERE ********* //
	return IsDebuggerPresent();
	return true;
}

BOOL AD_INT()
{
	// ****** YOUR CODE HERE ********* //
	__asm {
		push   offset exception_handler; set exception handler
		push  dword ptr fs:[0h]
		mov    dword ptr fs:[0h],esp  
		xor   eax,eax;reset EAX invoke int3
		int    3h
		pop    dword ptr fs:[0h];restore exception handler
		add   esp,4

		test   eax,eax; check the flag 
		je    rt_choke
		jmp    rf_choke

	}

	__asm {
exception_handler:
	mov   eax,dword ptr [esp+0xc];EAX = ContextRecord
	mov    dword ptr [eax+0xb0],0xffffffff;set flag (ContextRecord.EAX)
	inc   dword ptr [eax+0xb8];set ContextRecord.EIP
	xor   eax,eax
	retn

rt_choke:
	xor eax,eax
	inc eax
	mov esp,ebp
	pop ebp
	retn

rf_choke:
	xor eax,eax
	mov esp,ebp
	pop ebp
	retn
	}
}

BOOL AD_ICE()
{
	// ****** YOUR CODE HERE ********* //
	return true;
}

BOOL AD_EnumProcess()
{
	PROCESSENTRY32 pe32;
	// ****** YOUR CODE HERE ********* //
	if(strcmp(pe32.szExeFile,"OLLYICE.EXE")==0)
        return true;
	if(strcmp(pe32.szExeFile,"IDAG.EXE")==0)
        return true;
    if(strcmp(pe32.szExeFile,"OLLYDBG.EXE")==0)
        return true;
    if(strcmp(pe32.szExeFile,"PEID.EXE")==0)
        return true;
    if(strcmp(pe32.szExeFile,"SOFTICE.EXE")==0)
        return true;
    if(strcmp(pe32.szExeFile,"LORDPE.EXE")==0)
        return true;
    if(strcmp(pe32.szExeFile,"IMPORTREC.EXE")==0)
        return true;
    if(strcmp(pe32.szExeFile,"W32DSM89.EXE")==0)
        return true;
    if(strcmp(pe32.szExeFile,"WINDBG.EXE")==0)
        return true;
	return false;
	// TODO: enumerate process to do string matching
	return true;
}

BOOL AD_FindWindow()
{
	// ****** YOUR CODE HERE ********* //
	HWND hWnd;
	hWnd=FindWindow(_T("ollydbg"),NULL);
	if (hWnd!=NULL){
		return true;
	}

	return false;
}



BOOL AntiDebug()
{
	BOOL debugger = FALSE;
	printf("Check debugger\n");
	if(AD_IsDebuggerPresent())
	{
		printf("Debugger Present\n");
		debugger =  TRUE;
	}
	if(AD_PEB_IsDebugged())
	{
		printf("Is Debugged\n");
		debugger = TRUE;
	}
	if(AD_PEB_NtGlobalFlags())
	{
		printf("Is PEB_NtGlobalFlags Detect\n");
		debugger = TRUE;
	}
	// ****** YOUR CODE HERE ********* //
	// Use your debugger-checking functions here!

	if(AD_PEB_HeapFlags())
	{
		printf("Is PEB_HeapFlags Detected\n");
		debugger = TRUE;
	}
	if(AD_CheckRemoteDebuggerPresent())
	{
		printf("Is CheckRemoteDebuggerPresent Detected\n");
		debugger = TRUE;
	}
	if(AD_EnumProcess())
	{
		printf("Is CheckRemoteDebuggerPresent Detected\n");
		debugger = TRUE;
	}
	if(AD_FindWindow())
	{
		printf("Is FindWindow Detected\n");
		debugger = TRUE;
	}
	if(AD_INT())
	{
		printf("Is Interrupt Detected\n");
		debugger = TRUE;
	}
	if(!debugger)
		printf("Debugger Not Found\n");
	return debugger;
}

