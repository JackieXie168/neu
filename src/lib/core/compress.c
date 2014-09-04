/*

      ___           ___           ___
     /\__\         /\  \         /\__\
    /::|  |       /::\  \       /:/  /
   /:|:|  |      /:/\:\  \     /:/  /
  /:/|:|  |__   /::\~\:\  \   /:/  /  ___
 /:/ |:| /\__\ /:/\:\ \:\__\ /:/__/  /\__\
 \/__|:|/:/  / \:\~\:\ \/__/ \:\  \ /:/  /
     |:/:/  /   \:\ \:\__\    \:\  /:/  /
     |::/  /     \:\ \/__/     \:\/:/  /
     /:/  /       \:\__\        \::/  /
     \/__/         \/__/         \/__/
 
 
The Neu Framework, Copyright (c) 2013-2014, Andrometa LLC
All rights reserved.

neu@andrometa.net
http://neu.andrometa.net

Neu can be used freely for commercial purposes. We hope you will find
Neu powerful, useful, and fun! If so, please consider making a
donation via PayPal to: neu@andrometa.net

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:
 
1. Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
 
2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.
 
3. Neither the name of the copyright holder nor the names of its
contributors may be used to endorse or promote products derived from
this software without specific prior written permission.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 
*/

#include <neu/compress.h>

#include <stdio.h>
#include <stdlib.h>
#include <zlib.h>
#include <assert.h>

int zlib_compress_(const char* in,
                   char* out,
                   unsigned int inSize,
                   unsigned int outSize){
  int ret;
  z_stream strm;
  
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  
  ret = deflateInit(&strm, Z_DEFAULT_COMPRESSION);
  assert(ret == Z_OK);
  
  strm.avail_in = inSize;
  strm.next_in = (unsigned char*)in;
  
  strm.avail_out = outSize;
  strm.next_out = (unsigned char*)out;
  
  ret = deflate(&strm, Z_FINISH);
  
  assert(ret != Z_STREAM_ERROR);
  
  deflateEnd(&strm);
  
  return outSize - strm.avail_out;
}

char* zlib_decompress_(const char* in,
                       unsigned int inSize,
                       char* out,
                       unsigned int* outSize,
                       int resize){
  int ret;
  z_stream strm;
  
  strm.zalloc = Z_NULL;
  strm.zfree = Z_NULL;
  strm.opaque = Z_NULL;
  strm.avail_in = 0;
  strm.next_in = Z_NULL;
  
  if(inflateInit(&strm) != Z_OK){
    return 0;
  }
  
  unsigned int pos = 0;
  unsigned int chunk = *outSize;
  
  strm.avail_in = inSize;
  strm.next_in = (unsigned char*)in;
  strm.avail_out = chunk;
  strm.next_out = (unsigned char*)out;
  
  for(;;){
    ret = inflate(&strm, Z_NO_FLUSH);
    
    switch(ret){
      case Z_ERRNO:
      case Z_STREAM_ERROR:
      case Z_DATA_ERROR:
      case Z_MEM_ERROR:
      case Z_BUF_ERROR:
      case Z_VERSION_ERROR:
        return 0;
    }
    
    pos += chunk - strm.avail_out;
    
    if(ret == Z_STREAM_END){
      break;
    }
    
    if(strm.avail_out > 0 || strm.avail_in == 0){
      break;
    }
    
    if(!resize){
      return 0;
    }
    
    (*outSize) *= 2;
    
    out = (char*)realloc(out, sizeof(char)*(*outSize));
    if(!out){
      return 0;
    }
    
    strm.avail_out = (*outSize)/2;
    strm.next_out = (unsigned char*)(out + pos);
    chunk = (*outSize)/2;
  }
  
  inflateEnd(&strm);
  *outSize = pos + 1;
  
  return out;
}
