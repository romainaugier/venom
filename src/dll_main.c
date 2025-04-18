/* SPDX-License-Identifier: BSD-3-Clause */
/* Copyright (c) 2025 - Present Romain Augier */
/* All rights reserved. */

#include "venom/lexer.h"

#include "libromano/logger.h"

#include <stdio.h>

#if defined(VENOM_WIN)
#include <Windows.h>
#endif // defined(VENOM_WIN)

/* 
   In this source file we execute all functions that need to be executed at runtime to check and
   set some global variables

   lib_entry is executed on dlopen / LoadLibrary
   lib_exit is executed on dlclose / CloseLibrary
*/

void VENOM_LIB_ENTRY lib_entry(void)
{
#if VENOM_DEBUG
    printf("libvenom entry\n");
#endif // VENOM_DEBUG
    logger_init();

    lexer_maps_init();
}

void VENOM_LIB_EXIT lib_exit(void)
{
#if VENOM_DEBUG
    printf("libvenom exit\n");
#endif // VENOM_DEBUG
    lexer_maps_release();
}

#if defined(VENOM_WIN)
BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) 
{
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            /* Code to run when the DLL is loaded */
            lib_entry();
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            /* Code to run when the DLL is unloaded */
            lib_exit();
            break;
    }

    return TRUE;
}
#endif // defined(VENOM_WIN)