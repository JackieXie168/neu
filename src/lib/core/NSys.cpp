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

Neu can be used freely for commercial purposes. If you find Neu
useful, please consider helping to support our work and the evolution
of Neu by making a PayPal donation to: neu@andrometa.net

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

#include <neu/NSys.h>

#include <fstream>
#include <cmath>
#include <cstdlib>
#include <functional>

#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/termios.h>
#include <signal.h>
#include <sys/time.h>

#ifdef __APPLE__

#include <sys/types.h>
#include <sys/sysctl.h>

#else

#include <sys/sysinfo.h>

#endif

#include <neu/NError.h>
#include <neu/NMutex.h>
#include <neu/global.h>
#include <neu/NProgram.h>
#include <neu/NThread.h>
#include <neu/NProgram.h>

using namespace std;
using namespace neu;

nvar NSys::sysInfo(){
  nvar info;
  
#ifdef __APPLE__
  
  int mib1[2] = {CTL_KERN, KERN_BOOTTIME};
  struct timeval boottime;
  size_t size1 = sizeof(boottime);
  
  if(sysctl(mib1, 2, &boottime, &size1, 0, 0) != -1){
    info("uptime") = boottime.tv_sec + boottime.tv_usec/10e6;
  }
  else{
    info("uptime") = 0.0;
  }
  
  int mib2[2] = {CTL_HW, HW_PHYSMEM};
  size_t physmem;
  size_t size2 = sizeof(physmem);
  
  if(sysctl(mib2, 2, &physmem, &size2, 0, 0) != -1){
    info("totalram") = physmem;
  }
  else{
    info("totalram") = -1;
  }
  
  int mib3[2] = {CTL_HW, HW_USERMEM};
  size_t usermem;
  size_t size3 = sizeof(usermem);
  
  if(sysctl(mib3, 2, &usermem, &size3, NULL, 0) != -1){
    info("sharedram") = usermem;
  }
  else{
    info("sharedram") = -1;
  }
  
  int mib4[2] = {CTL_VM, VM_LOADAVG};
  struct loadavg la;
  size_t size4 = sizeof(la);
  
  if(sysctl(mib4, 2, &la, &size4, NULL, 0) != -1){
    info("load1") = la.ldavg[0]/float(la.fscale);
    info("load5") = la.ldavg[1]/float(la.fscale);
    info("load15") = la.ldavg[2]/float(la.fscale);
  }
  else{
    info("load1") = -1;
    info("load5") = -1;
    info("load15") = -1;
  }
  
  info("freeram") = -1;
  info("bufferram") = -1;
  info("totalswap") = -1;
  info("freeswap") = -1;
  info("procs") = -1;
  info("totalhigh") = -1;
  info("freehigh") = -1;
  info("mem_unit") = -1;
  
  return info;
  
#else
  
  struct sysinfo i;
  if(sysinfo(&i) == 0){
    info("uptime") = i.uptime;
    info("load1") = i.loads[0]/65536.0;
    info("load5") = i.loads[1]/65536.0;
    info("load15") = i.loads[2]/65536.0;
    info("totalram") = i.totalram;
    info("freeram") = i.freeram;
    info("sharedram") = i.sharedram;
    info("bufferram") = i.bufferram;
    info("totalswap") = i.totalswap;
    info("freeswap") = i.freeswap;
    info("procs") = i.procs;
    info("totalhigh") = i.totalhigh;
    info("freehigh") = i.freehigh;
    info("mem_unit") = i.mem_unit;
    
    return info;
  }
  
#endif
  NERROR("failed to get system info");
}

nstr NSys::hostname(){
  char buf[256];
  if(::gethostname(buf, 256) != 0){
    return "";
  }
  
  return buf;
}

nstr NSys::basename(const nstr& path){
  nstr ret;
  
  for(int i = path.length() - 1; i >= 0; --i){
    if(path[i] == '/'){
      break;
    }
  
    ret.insert(0, 1, path[i]);
  }
  
  return ret;
}

nstr NSys::parentDir(const nstr& path){
  size_t len = path.length();
  
  if(len == 0){
    return "";
  }
  
  size_t i = len - 1;
  
  while(i > 0){
    if(path[i] == '/'){
      return path.substr(0, i);
    }
    --i;
  }
  
  if(path[0] == '/'){
    return "/";
  }
  
  return "";
}

bool NSys::makeDir(const nstr& path){
  return mkdir(path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == 0;
}

nstr NSys::tempPath(){
  return _tempPath;
}

nstr NSys::tempFilePath(const nstr& extension){
  NProgram::use();
  
  nstr p = _tempPath;
  
  nstr h = nstr::toStr(hash<NThread::id>()(NThread::thisThreadId()));
  
  double t = now();
  p += "/" + nstr::toStr(processId()) + "." + h + "." + nstr::toStr(t, false);
  
  if(!extension.empty()){
    p += "." + extension;
  }
  
  return p;
}

long NSys::processId(){
  return getpid();
}

bool NSys::exists(const nstr& path){
  ifstream f(path.c_str());
  if(f.fail()){
    return false;
  }
  else{
    return true;
  }
}

nstr NSys::currentDir(){
  char cwd[4096];
  getcwd(cwd, 4096);
  
  return cwd;
}

bool NSys::getEnv(const nstr& key, nstr& value){
  const char* v = getenv(key.c_str());
  
  if(!v){
    return false;
  }
  value = v;
  
  return true;
}


bool NSys::setEnv(const nstr& key, const nstr& value, bool redef){
  int r = setenv(key.c_str(), value.c_str(), redef);
  
  if(r == 0){
    return true;
  }
  else{
    return false;
  }
}

void NSys::setTimeZone(const nstr& zone){
  setenv("TZ", zone.c_str(), true);
  tzset();
}

bool NSys::rename(const nstr& sourcePath, const nstr& destPath){
  int status = ::rename(sourcePath.c_str(), destPath.c_str());
  
  return status == 0;
}

nstr NSys::fileExtension(const nstr& path){
  size_t len = path.length();
  
  if(len == 0){
    return "";
  }
  
  size_t i = len - 1;
  
  while(i > 0){
    if(path[i] == '.'){
      return path.substr(i + 1, len - i);
    }
    else if(path[i] == '/'){
      return "";
    }
    
    --i;
  }
  
  return "";
}

nstr NSys::fileName(const nstr& filePath){
  nstr bn = basename(filePath);

  size_t pos = bn.find(".");
  if(pos == nstr::npos){
    return bn;
  }

  return bn.substr(0, pos);
}

nstr NSys::normalizePath(const nstr& path){
  nstr ret = path;
  ret.findReplace(" ", "\\ ");
  
  return ret;
}

nstr NSys::popPath(const nstr& path){
  size_t offset = 0;
  
  for(size_t i = path.length() - 1; i > 0; --i){
    if(path[i] == '/'){
      ++offset;
    }
    else{
      break;
    }
  }
  
  return path.substr(0, path.length() - offset);
}

bool NSys::dirFiles(const nstr& dirPath, nvec& files){
  DIR* dir = opendir(dirPath.c_str());
  
  if(!dir){
    return false;
  }
  
  dirent* de;
  
  while((de = readdir(dir))){
    nstr p = de->d_name;
    
    if(p != "." && p != ".."){
      files.push_back(de->d_name);
    }
  }
  
  closedir(dir);
  
  return true;
}

nstr NSys::fileToStr(const nstr& path){
  FILE* file = fopen(path.c_str(), "rb");
  
  if(!file){
    return "";
  }
  
  fseek(file, 0, SEEK_END);
  long size = ftell(file);
  rewind(file);
  
  char* buf = (char*)malloc(sizeof(char)*size);
  fread(buf, 1, size, file);
  
  fclose(file);
  
  nstr ret;
  ret.append(buf, size);
  
  free(buf);
  
  return ret;
}

double NSys::now(){
  timeval tv;
  gettimeofday(&tv, 0);
  
  return tv.tv_sec + tv.tv_usec/1e6;
}

void NSys::sleep(double dt){
  double sec = floor(dt);
  double fsec = dt - sec;
  
  timespec ts;
  ts.tv_sec = sec;
  ts.tv_nsec = fsec*1e9;
  
  nanosleep(&ts, 0);
}

void NSys::replaceEnvs(nstr& s){
  size_t len = s.length();

  for(size_t i = 0; i < len; ++i){
    if(s[i] == '$' &&
       (i == 0 || s[i - 1] != '\\') &&
       (i < len && s[i + 1] == '(')){
      
      size_t begin = i;
      i += 2;
      
      nstr env;
    
      bool fin = false;
      for(; i < len; ++i){
        if(s[i] == ')'){
          fin = true;
          break;
        }

        env += s[i];
      }

      if(!fin){
        NERROR("unterminated environment variable");
      }
      
      nstr v;
      if(!getEnv(env, v)){
        NERROR("invalid environment variable: " + env);
      }
      
      s.replace(begin, i - begin + 1, v);
      i = begin + v.length();
      len = s.length();
    }
  }
}
