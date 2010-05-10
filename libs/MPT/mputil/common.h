/*
 * common.h
 *
 * Copyright (c) 2003 Machine Perception Laboratory
 * University of California San Diego.
 * Please read the disclaimer and notes about redistribution
 * at the end of this file.
 *
 * Authors: Josh Susskind, Ian Fasel, Bret Fortenberry
 */
#ifndef __MPLABCOMMON_H__
#define __MPLABCOMMON_H__

#ifdef WIN32
#include <windows.h>
#else
#include <iostream.h>
#include <sys/time.h>
#include <pthread.h>
#include "errno.h"
#endif
#include <iostream>
#include <algorithm>

#ifndef BYTE
#define BYTE unsigned char
#endif
#ifndef PIXEL_TYPE
#define PIXEL_TYPE BYTE
#endif

#undef BOOL
typedef int BOOL;

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#undef PIXEL_TYPE
#define PIXEL_TYPE BYTE

typedef struct sMUTEX {
	int activation;
#ifdef WIN32
	HANDLE mutex;
#else
	pthread_mutex_t mutex;
#endif
} MUTEX_TYPE;


// ================================================================ 

#ifdef WIN32
#define MUTEX_LOCK_RETURN_TYPE DWORD
#define THREAD_RETURN_TYPE void
#define THREAD_TYPE HANDLE
#define THREAD_COND HANDLE

#define TIMETYPE DWORD
#define IMAGE_FLIPPED true

// ================================================================

#else   //if not WIN32  

class  nRGBTRIPLE{
public:
  PIXEL_TYPE rgbtRed;
  PIXEL_TYPE rgbtGreen;
  PIXEL_TYPE rgbtBlue;

  // Default & Copy Constructors and = operator
  nRGBTRIPLE() : rgbtRed(0), rgbtGreen(0), rgbtBlue(0){};
  nRGBTRIPLE(PIXEL_TYPE r, PIXEL_TYPE g, PIXEL_TYPE b) :
    rgbtRed(r), rgbtGreen(g), rgbtBlue(b){};
  template<class T> 
	inline nRGBTRIPLE(const T &v){
	  rgbtRed = static_cast<PIXEL_TYPE>(v);
	  rgbtGreen = static_cast<PIXEL_TYPE>(v);
	  rgbtBlue = static_cast<PIXEL_TYPE>(v);
  }
  nRGBTRIPLE(const nRGBTRIPLE &v){
	rgbtRed = v.rgbtRed;
	rgbtGreen = v.rgbtGreen;
	rgbtBlue = v.rgbtBlue;
  }
  template<class T> 
//	inline nRGBTRIPLE& operator=(const T &v){
	  inline void operator=(const T &v){
		rgbtRed = static_cast<PIXEL_TYPE>(v);
		rgbtGreen = static_cast<PIXEL_TYPE>(v);
		rgbtBlue = static_cast<PIXEL_TYPE>(v);
//		return *this;
  }
//  inline nRGBTRIPLE& operator=(const nRGBTRIPLE &v){
  inline void operator=(const nRGBTRIPLE &v){
		rgbtRed = v.rgbtRed;
		rgbtGreen = v.rgbtGreen;
		rgbtBlue = v.rgbtBlue;
//		return *this;
  }
};

#define RGBTRIPLE nRGBTRIPLE

typedef unsigned long       DWORD;
typedef unsigned short      WORD;
typedef long 								LONG;
typedef long								LONGLONG;


#define MUTEX_LOCK_RETURN_TYPE int
#define WAIT_OBJECT_0 0
#define WAIT_TIMEOUT EBUSY
#define WAIT_ABANDONED EDEADLK
#define WAIT_FAILED EINVAL
#define THREAD_RETURN_TYPE void *
#define THREAD_TYPE pthread_t
#define THREAD_COND pthread_cond_t

#define TIMETYPE struct timeval
#define IMAGE_FLIPPED false

typedef struct tagBITMAPFILEHEADER { // bmfh 
    WORD    bfType; 
    DWORD   bfSize; 
    WORD    bfReserved1; 
    WORD    bfReserved2; 
    DWORD   bfOffBits; 
} BITMAPFILEHEADER; 

typedef struct tagBITMAPINFOHEADER{ // bmih 
    DWORD  biSize; 
    LONG   biWidth; 
    LONG   biHeight; 
    WORD   biPlanes; 
    WORD   biBitCount;
    DWORD  biCompression; 
    DWORD  biSizeImage; 
    LONG   biXPelsPerMeter; 
    LONG   biYPelsPerMeter; 
    DWORD  biClrUsed; 
    DWORD  biClrImportant; 
} BITMAPINFOHEADER;

	
#endif //not WIN32

///////////////////////////////////////////////////////////////////

inline MUTEX_LOCK_RETURN_TYPE LockMutex(MUTEX_TYPE &pMutex){
	MUTEX_LOCK_RETURN_TYPE rtn;
#ifdef WIN32
	rtn =  WaitForSingleObject( pMutex.mutex, INFINITE );
#else
	rtn = pthread_mutex_lock(&pMutex.mutex);
#endif
  if(rtn != 0) cout << "LockMutex: Nonzero return in thread routine" << endl;
  if(pMutex.activation == 1) {
  	std::cout << "error - managed to lock previously locked mutex\n";
  }
  pMutex.activation = 1;
 return rtn;
}

inline MUTEX_LOCK_RETURN_TYPE TryLockMutex(MUTEX_TYPE &pMutex, DWORD msec = 16){
	MUTEX_LOCK_RETURN_TYPE rtn;
#ifdef WIN32
	rtn = WaitForSingleObject(pMutex.mutex, msec );
#else
	rtn = pthread_mutex_trylock(&pMutex.mutex);
#endif
	if (rtn == WAIT_OBJECT_0) pMutex.activation = 1;
	if(rtn != 0) cout << "TryLockMutex: Nonzero return in thread routine" << endl;
	 return rtn;
}

inline MUTEX_LOCK_RETURN_TYPE CreateMutex (MUTEX_TYPE &pMutex) {
	MUTEX_LOCK_RETURN_TYPE rtn;
#ifdef WIN32
	pMutex.mutex = CreateMutex (NULL, FALSE, NULL);
#else
	rtn = (pthread_mutex_init(&pMutex.mutex,NULL));
#endif
  pMutex.activation = 0;
if(rtn != 0) cout << "CreateMutex: Nonzero return in thread routine" <<endl;
	 return rtn;
}

inline MUTEX_LOCK_RETURN_TYPE ReleaseMutex(MUTEX_TYPE &pMutex) {
	MUTEX_LOCK_RETURN_TYPE rtn;
	pMutex.activation = 0;
#ifdef WIN32 
	rtn = ReleaseMutex(pMutex.mutex);
#else
	rtn = pthread_mutex_unlock(&pMutex.mutex);
#endif
  if(rtn != 0){
    cout << "ReleaseMutex: Nonzero return in thread routine" <<endl;
    cout << "rtrn: " << rtn << endl;
    cout << "errno: " << errno << endl;
    cout << "EINVAL: " << EINVAL << endl;
    cout << "EPERM: " << EPERM << endl;
  }
  return rtn;
}

inline MUTEX_LOCK_RETURN_TYPE CreateMutexEvent(THREAD_COND &pCond) {
	MUTEX_LOCK_RETURN_TYPE rtn;
#ifdef WIN32
	rtn = CreateEvent(NULL, TRUE, FALSE, NULL); 
#else
	rtn = pthread_cond_init(&pCond,NULL);
#endif
 if(rtn != 0) cout << "CreateMutexEvent:Nonzero return in thread routine" <<endl;
	 return rtn;
}

inline MUTEX_LOCK_RETURN_TYPE MutexCondSignal(THREAD_COND &pCond) {
	MUTEX_LOCK_RETURN_TYPE rtn;
#ifdef WIN32
	rtn = PulseEvent(pCond);
#else
	rtn = pthread_cond_signal(&pCond);
#endif
if(rtn != 0) cout << "MutexCondSignal: Nonzero return in thread routine" <<endl;
	 return rtn;
}

inline MUTEX_LOCK_RETURN_TYPE MutexCondWait(THREAD_COND &pCond, MUTEX_TYPE &pMutex) {
	MUTEX_LOCK_RETURN_TYPE rtn;
#ifdef WIN32
	rtn = WaitForSingleObject( pCond, INFINITE );
#else
	rtn = pthread_cond_wait(&pCond,&pMutex.mutex);
#endif
if(rtn != 0) cout << "MutexCondWait: Nonzero return in thread routine" <<endl;
	 return rtn;
}


///////////////////////////////////////////////////////////////////


#endif // __MPLABCOMMON_H__


/*
 * 
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 * 
 *    1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 *    2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 *    3. The name of the author may not be used to endorse or promote products derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */













