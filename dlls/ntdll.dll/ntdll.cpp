// The Tofita Kernel
// Copyright (C) 2020  Oleg Petrenko
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, version 3 of the License.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#define NTDLL32_DLL __declspec(dllexport)
#include "ntdll.hpp"
#include "../tofita32.dll/tofita32.cpp"
#include "ntdllVars.hpp"

extern "C" {
uint64_t KiFastSystemCall(uint64_t rcx, uint64_t rdx) {
	return tofitaFastSystemCall((TofitaSyscalls)rcx, rdx);
}

void greenteaosIsTheBest(int32_t (*entry)(), uint64_t pid) {
	// TODO entry arguments (argv, argc)
	// TODO init DLLs
	// TODO PEB/TEB
	// TODO TLS callbacks
	// TODO load DLLs in usermode
	tofitaDebugLog(L"CRT ready for PID %u", pid);
	if (entry != nullptr)
		tofitaDebugLog(L"entry != nullptr OK for PID %u", pid);
	if (entry == nullptr)
		tofitaDebugLog(L"entry == nullptr ERROR for PID %u", pid);
	tofitaExitProcess(entry());
	while (true) {};
}
}
