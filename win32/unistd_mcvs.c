/*
 *      This file is part of the KoraOS project.
 *  Copyright (C) 2015-2021  <Fabien Bavent>
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as
 *  published by the Free Software Foundation, either version 3 of the
 *  License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *   - - - - - - - - - - - - - - -
 */

#include <windows.h>
#include <stdio.h>
#include <fcntl.h>

FILE *_fhopen(HANDLE hFile)
{
    int nHandle = _open_osfhandle((long)hFile, _O_RDONLY);
    if (nHandle == -1) {
        CloseHandle(hFile);
        return NULL;
    }
    FILE *fp = _fdopen(nHandle, "rb");
    if (!fp)
        close(nHandle);
    return fp;
}

int pipe2(int fds[2], int flags)
{
    HANDLE hRd, hWr;
    if (CreatePipe(&hRd, &hWr, NULL, 65536) == FALSE)
        return -1;
    fds[0] = _open_osfhandle((long)hRd, _O_RDONLY | flags);
    fds[1] = _open_osfhandle((long)hWr, _O_WRONLY | flags);
    return 0;
}

int pipe(int fds[2])
{
    return pipe2(fds, 0);
}

int __exec(char *name, const char **argv, const char **env, int fds[3])
{
    int i;
    char cmdline[4096];

    int argc = 0;
    // TODO -- on PATH !?
    strncpy(cmdline, name, 4096);
    for (i = 0; i < argc; ++i) {
        strncat(cmdline, " ", 4096);
        strncat(cmdline, argv[i], 4096);
    }

    PROCESS_INFORMATION piProcInfo;
    STARTUPINFO siStartInfo;

    SECURITY_ATTRIBUTES saAttr;
    saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    saAttr.bInheritHandle = TRUE;
    saAttr.lpSecurityDescriptor = NULL;

    HANDLE hInRd, hInWr, hOutRd, hOutWr;
    if (CreatePipe(&hInRd, &hInWr, &saAttr, 65536) == FALSE)
        return -1;
    if (CreatePipe(&hOutRd, &hOutWr, &saAttr, 65536) == FALSE) {
        CloseHandle(hInRd);
        CloseHandle(hInWr);
        return -1;
    }

    SetHandleInformation(hOutRd, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hInWr, HANDLE_FLAG_INHERIT, 0);

    ZeroMemory(&piProcInfo, sizeof(PROCESS_INFORMATION));

    ZeroMemory(&siStartInfo, sizeof(STARTUPINFO));
    siStartInfo.cb = sizeof(STARTUPINFO);
    siStartInfo.hStdError = hOutWr;
    siStartInfo.hStdOutput = hOutWr;
    siStartInfo.hStdInput = hInRd;
    siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

    if (CreateProcess(NULL,
                      (TCHAR *)cmdline,
                      NULL,
                      NULL,
                      TRUE,
                      0,
                      NULL,
                      NULL,
                      &siStartInfo,
                      &piProcInfo)
        == FALSE) {
        CloseHandle(hInRd);
        CloseHandle(hInWr);
        CloseHandle(hOutRd);
        CloseHandle(hOutWr);
        return -1;
    }

    fds[0] = _open_osfhandle(hInWr, _O_WRONLY);
    fds[1] = _open_osfhandle(hOutRd, _O_RDONLY);
    if (fds[0] == 0)
        fds[0] = -1;
    if (fds[1] == 0)
        fds[1] = -1;
    fds[2] = fds[1];

    CloseHandle(piProcInfo.hProcess);
    CloseHandle(piProcInfo.hThread);

    CloseHandle(hInRd);
    CloseHandle(hOutWr);
    return piProcInfo.dwProcessId;
}
