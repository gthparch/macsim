#include <cstdlib>
#include <iostream>
#include <sys/time.h>
#include "annotations.h"

#define ITER   20000
#define MAX_N 128*1024*1024
#define MB    (1024*1024)
#define KB    1024
#define BYTE  8

// LLC Parameters assumed
#define START_SIZE 1*MB
#define STOP_SIZE  32*MB


using namespace std;

char array[MAX_N];



/////////////////////////////////////////////////////////
// Provides elapsed Time between t1 and t2 in milli sec
/////////////////////////////////////////////////////////

double elapsedTime(timeval t1, timeval t2){
  double delta;
  delta = (t2.tv_sec - t1.tv_sec) * 1000.0;      // sec to ms
  delta += (t2.tv_usec - t1.tv_usec) / 1000.0;   // us to ms
  return delta;
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

double DummyTest(void)
{
  timeval t1, t2;
  int ii, iterid;

  // start timer
  gettimeofday(&t1, NULL);

  for(iterid=0;iterid<ITER;iterid++){
    for(ii=0; ii< MAX_N; ii++){
      array[ii] ++;
    }
  }

  // stop timer
  gettimeofday(&t2, NULL);

  return elapsedTime(t1,t2);
}



/////////////////////////////////////////////////////////
// Change this, including input parameters
/////////////////////////////////////////////////////////

double CacheNumLevelsTest(int inputsize, int stride)
{
  double retval;
  int ii, iterid;
  double counter = 0;
  timeval t1, t2;
  char dummy;
  int steps = 256 * MB;
  int mask = inputsize - 1;
  // start timer
  //int multiplier = 4 * MB / inputsize + 1;
  gettimeofday(&t1, NULL);

  //for(iterid=0;iterid<ITER * multiplier;iterid++){
    for(ii=0; ii< steps; ii ++){
      array[(ii * stride) & mask] ++;
      //dummy = array[ii];
      //counter ++;
    }
  //}

  gettimeofday(&t2, NULL);
  retval = elapsedTime(t1,t2);
  retval /= steps;
  return retval;
}


/////////////////////////////////////////////////////////
// Change this, including input parameters
/////////////////////////////////////////////////////////

double CacheSizeTest(void)
{
  double retval;
  // same as Cache Level test so empty here.
  return retval;
}


/////////////////////////////////////////////////////////
// Change this, including input parameters
/////////////////////////////////////////////////////////

double LineSizeTest(int inputsize, int stride)
{
  double retval;
  int ii, iterid;
  double counter = 0;
  timeval t1, t2;
  char dummy;
  int steps = 64 * KB;
  int mask = inputsize - 1;
  // start timer
  //int multiplier = 4 * MB / inputsize + 1;
  gettimeofday(&t1, NULL);

  //for(iterid=0;iterid<ITER * multiplier;iterid++){
    for(ii=0; ii< steps; ii ++){
      array[(ii * stride) & mask] ++;
      //dummy = array[ii];
      //counter ++;
    }
  //}

  gettimeofday(&t2, NULL);
  retval = elapsedTime(t1,t2);
  retval /= steps;
  return retval;
}


/////////////////////////////////////////////////////////
// Change this, including input parameters
/////////////////////////////////////////////////////////

double MemoryTimingTest(int inputsize, int stride)
{
  double retval;
  int ii, iterid;
  double counter = 0;
  timeval t1, t2;
  //char dummy;
  int steps = 1000 * inputsize/stride;
  int mask = inputsize - 1;
  // start timer

  gettimeofday(&t1, NULL);


    for(ii=0; ii< steps; ii ++){
      array[(ii * stride +1) & mask] ++; //reason to add one here is to avoid regular pattern
      //dummy = array[ii];

    }


  gettimeofday(&t2, NULL);
  retval = elapsedTime(t1,t2);
  retval /= steps;
  return retval;
}



/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////

int main(){

  //cout << "\nStarting DummyTest:" << "\n";
  //cout << "Dummy Test took " << DummyTest()/1000.0 << " seconds\n";
  cout << "\nStarting LineSizeTest:" << "\n";
  cout << "Testing strides from 1KB to 128KB" << "\n";
  /*
  for(int b = 1; b <= 128; b*=2) {
        cout << "LineSizeTest of " << double(32* KB)/KB <<  " KB Array using " << b << " Byte stride took " << LineSizeTest(32* KB, b) * 1000000 << " ns\n";
  }

  */

  int b = 64 *2; //use 128 byte as stride to speed up the testing and also avoiding next line prefetcher.
  cout << "\nStarting CacheLevelTest:" << "\n";
  cout << "Testing from 1KB to 32MB" << "\n";
  int ArrSize[] = {1*KB, 2*KB, 4*KB, 8*KB, 16*KB, 32*KB, 64*KB, 128*KB, 256*KB, 512*KB, 1*MB, 2*MB, 4*MB, 8*MB, 16*MB};
  //cout << "Warmup. Lvl#Test of " << double(KB)/KB <<  " KB Array using " << 256 << " Byte stride took " << CacheNumLevelsTest(KB, 256) * 1000000 << " ns\n";
  //cout << "Warmup. Lvl#Test of " << double(2*KB)/KB <<  " KB Array using " << 256 << " Byte stride took " << CacheNumLevelsTest(2*KB, 256) * 1000000 << " ns\n";
  //cout << "Warmup. Lvl#Test of " << double(4*KB)/KB <<  " KB Array using " << 256 << " Byte stride took " << CacheNumLevelsTest(4*KB, 256) * 1000000 << " ns\n";
  //cout << "Warmup. Lvl#Test of " << double(8*KB)/KB <<  " KB Array using " << 256 << " Byte stride took " << CacheNumLevelsTest(8*KB, 256) * 1000000 << " ns\n";
  //no need to warm up for millions of time
  SIM_BEGIN(1); 
  for(int a = 4; a < 5; a++) {
        cout << "Lvl#Test of " << double(ArrSize[a])/KB <<  " KB Array using " << b << " Byte stride took " << CacheNumLevelsTest(ArrSize[a], b) * 1000000 << " ns\n";
  }
  SIM_END(1); 
  for(int a = 5; a < 7; a++) {
        cout << "Lvl#Test of " << double(ArrSize[a])/KB <<  " KB Array using " << b << " Byte stride took " << CacheNumLevelsTest(ArrSize[a], b) * 1000000 << " ns\n";
  }
  

  
  
/*
  cout << "\nStarting MemoryTimingTest" << "\n";
  cout << "Warmup. " << double(128*MB)/KB <<  " KB Array using " << 256 << " Byte stride took " << MemoryTimingTest(128*MB, 256) * 1000000 << " ns\n";
  
  cout << "MemoryTimingTest of " << double(128*MB)/KB <<  " KB Array using " << 3 * MB + (256+32) * KB<< " Byte stride took " << MemoryTimingTest(128* MB, 3 * MB + (256+32)* KB) * 1000000 << " ns\n";
*/

  // Add your code here, and comment above

  cout << "\n";
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
