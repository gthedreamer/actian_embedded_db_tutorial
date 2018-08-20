/*************************************************************************
**
**  Copyright 2014 Actian Corporation All Rights Reserved
**
*************************************************************************/
/***************************************************************************
  LOGINAPI.C
    This is a simple sample designed to allow you to confirm your
    ability to compile, link, and execute a Btrieve application for
    your target environment using your compiler tools.

    This program demonstrates how to use new Btrieve login API with secured
    database. Login API available with PSQL 8.50. To run a program
    DefaultDb database should be secured and security mode set to database
    authentication and authorization. Use PCC to set that up. 

    This program does the following operations on the DefaultDb database:
    - logs in to DefaultDb database using new btrieve operation B_LOGIN (78)
    - opens sample.btr
    - gets a record on a known value of Key 0
    - displays the retrieved record
    - closes sample.btr
    - logs out from DefaultDB database

    The second section demonstrates how to use a Database URI in a Btrieve Open call.

    - opens sample.btr using URI string in a key buffer. 
    - gets a record on a known value of Key 0
    - displays the retrieved record
    - closes sample.btr

    IMPORTANT:
    You must specify the complete path to the directory that contains
    the sample Btrieve data file, 'sample.btr'.  See IMPORTANT, below.

    You can compile and run this program on any of the platforms
    supported by the interface modules.  Platforms are indicated by the
    platform switches listed in 'btrapi.h'.  For MS Windows you should
    make an application that allows standard output via printf().  Note
    that most C/C++ compilers support standard I/O Windows applications.
    For MS Windows NT or OS2, you should make a console application.

    See the prologue in 'btrapi.h' for information on how to select
    a target platform for your application.  You must specify a target
    platform.

****************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <btrapi.h>
#include <btrconst.h>

/***************************************************************************
  Constants
****************************************************************************/
/********************************************************************
   IMPORTANT: You should modify the following to specify the
              complete path to 'sample.btr' for your environment.
********************************************************************/
#if defined (BTI_LINUX) || defined (BTI_LINUX_64) || defined (BTI_MACOSX) || defined (BTI_MACOSX_64)
#define FILE_NAME "/usr/local/psql/data/samples/sample.btr"
BTI_CHAR *URI_STRING2 = "btrv://Master@localhost/defaultdb?file=/usr/local/psql/data/samples/sample.btr&pwd=master";
#else
#ifdef BTI_NLM
#define FILE_NAME "sys:\\pvsw\\samples\\sample.btr"
#else
#define FILE_NAME "c:\\ProgramData\\Actian\\PSQL\\samples\\sample.btr" 
BTI_CHAR *URI_STRING2 = "btrv://Master@localhost/defaultdb?file=C:/ProgramData/Actian/PSQL/samples/sample.btr&pwd=master";
#endif
#endif

/* define URI string to open sample.btr file in DefaultDb database*/
/* SQL security is set for DefaultDb with User name "Master" and passord "master" */
BTI_CHAR *URI_STRING1 = "btrv://Master@localhost/defaultdb?pwd=master";

#define EXIT_WITH_ERROR     1
#define TRUE                1
#define FALSE               0
#define MY_THREAD_ID        50

/* Don't pad our structures */
#if defined(__BORLANDC__)
  #pragma option -a-
#else
  #if defined(_MSC_VER) || defined(__WATCOMC__)
    #pragma pack(1)
  #endif
#endif

/***************************************************************************
  Type definitions for Client ID and version Structures
****************************************************************************/
typedef struct
{
  BTI_CHAR networkAndNode[12];
  BTI_CHAR applicationID[2];
  BTI_WORD threadID;
} CLIENT_ID;

typedef struct
{
  BTI_SINT  Version;
  BTI_SINT  Revision;
  BTI_CHAR  MKDEId;
} VERSION_STRUCT;

/***************************************************************************
  Definition of record from 'sample.btr'
****************************************************************************/
typedef struct
{
  BTI_LONG  ID;
  BTI_CHAR  FirstName[16];
  BTI_CHAR  LastName[26];
  BTI_CHAR  Street[31];
  BTI_CHAR  City[31];
  BTI_CHAR  State[3];
  BTI_CHAR  Zip[11];
  BTI_CHAR  Country[21];
  BTI_CHAR  Phone[14];
} PERSON_STRUCT;

/* restore structure packing */
#if defined(__BORLANDC__)
  #pragma option -a.
#else
  #if defined(_MSC_VER) || defined(__WATCOMC__)
    #pragma pack()
  #endif
#endif

/***************************************************************************
  Main
****************************************************************************/
int main(void)
{
  /* Btrieve function parameters */
  BTI_BYTE posBlock[128];
  BTI_BYTE dataBuf[255];
  BTI_WORD dataLen;
  BTI_BYTE keyBuf[255];
  BTI_WORD keyNum = 0;

  BTI_LONG personID;
  BTI_BYTE fileOpen = FALSE;
  BTI_SINT status = 0;

  CLIENT_ID       clientID;
  PERSON_STRUCT   personRecord;

  

  printf("**************** Btrieve C/C++ Interface Demo for Login API ****************\n\n");

  /* set up the Client ID */
  memset(clientID.networkAndNode, 0, sizeof(clientID.networkAndNode));
  memcpy(clientID.applicationID, "MT", 2);  /* must be greater than "AA" */
  clientID.threadID = MY_THREAD_ID;

  /* Login to secure database */
  memset(dataBuf, 0, sizeof(dataBuf));
  strcpy((char *)keyBuf, URI_STRING1);
  keyNum = 0;
  status = BTRVID(
                B_LOGIN,
                posBlock,
                dataBuf,
                &dataLen,
                keyBuf,
                keyNum,
                (BTI_BUFFER_PTR)&clientID);  

   printf("Btrieve B_LOGIN status = %d\n", status);
  
  /* clear buffers */
    memset(dataBuf, 0, sizeof(dataBuf));
    memset(keyBuf , 0, sizeof(keyBuf));

  /* open sample.btr */
    strcpy((BTI_CHAR *)keyBuf, FILE_NAME);

    keyNum  = 0;
    dataLen = 0;

    status = BTRVID(
                B_OPEN,
                posBlock,
                dataBuf,
                &dataLen,
                keyBuf,
                keyNum,
                (BTI_BUFFER_PTR)&clientID);

    printf("Btrieve B_OPEN status (sample.btr) = %d\n", status);
    if (status == B_NO_ERROR)
    {
      fileOpen = TRUE;
    }


  /* get the record with key 0 = 263512477 using B_GET_EQUAL */
  if (status == B_NO_ERROR)
  {
    memset(&personRecord, 0, sizeof(personRecord));
    dataLen = sizeof(personRecord);
    personID = 263512477;    /* this is really a social security number */
    *(BTI_LONG BTI_FAR *)&keyBuf[0] = personID;
    keyNum = 0;

    status = BTRVID(
                B_GET_EQUAL,
                posBlock,
                &personRecord,
                &dataLen,
                keyBuf,
                keyNum,
                (BTI_BUFFER_PTR)&clientID);

    printf("Btrieve B_GET_EQUAL status = %d\n", status);
    if (status == B_NO_ERROR)
    {
      printf("\n");
      printf("The retrieved record is:\n");
#ifdef BTI_MACOSX_64
      printf("ID:      %d\n", personRecord.ID);
#else
      printf("ID:      %ld\n", personRecord.ID);
#endif
      printf("Name:    %s %s\n", personRecord.FirstName,
                                 personRecord.LastName);
      printf("Street:  %s\n", personRecord.Street);
      printf("City:    %s\n", personRecord.City);
      printf("State:   %s\n", personRecord.State);
      printf("Zip:     %s\n", personRecord.Zip);
      printf("Country: %s\n", personRecord.Country);
      printf("Phone:   %s\n", personRecord.Phone);
      printf("\n");
    }
  }

  /* close open files */
  if (fileOpen)
  {
    dataLen = 0;

    status = BTRVID(
                B_CLOSE,
                posBlock,
                dataBuf,
                &dataLen,
                keyBuf,
                keyNum,
                (BTI_BUFFER_PTR)&clientID);

    printf("Btrieve B_CLOSE status (sample.btr) = %d\n", status);
  }


  /* Logout from secure database */
  
  memset(dataBuf, 0, sizeof(dataBuf));
#ifdef BTI_MACOSX_64
  strcpy((char *)keyBuf, URI_STRING1);
#else
  strcpy(keyBuf, URI_STRING1);
#endif
  keyNum = 1;
  status = BTRVID(
                B_LOGIN,
                posBlock,
                dataBuf,
                &dataLen,
                keyBuf,
                keyNum,
                (BTI_BUFFER_PTR)&clientID);
   
  
  printf("Btrieve logout status = %d\n", status);
  printf("\n\n");
  
  

  /* Following example uses URI string in Btrieve Open call */
 
  printf("******** Following example uses URI string in Btrieve Open call **********\n\n");

  /* set up the Client ID */
  memset(clientID.networkAndNode, 0, sizeof(clientID.networkAndNode));
  memcpy(clientID.applicationID, "MT", 2);  /* must be greater than "AA" */
  clientID.threadID = MY_THREAD_ID;
  
  /* clear buffers */
  memset(dataBuf, 0, sizeof(dataBuf));
  memset(keyBuf , 0, sizeof(keyBuf));

  /* open sample.btr */
  strcpy((BTI_CHAR *)keyBuf, URI_STRING2);

  keyNum  = 0;
  dataLen = 0;

  status = BTRVID(
                B_OPEN,
                posBlock,
                dataBuf,
                &dataLen,
                keyBuf,
                keyNum,
                (BTI_BUFFER_PTR)&clientID);

  printf("Btrieve B_OPEN status (sample.btr) = %d\n", status);
  if (status == B_NO_ERROR)
  {
    fileOpen = TRUE;
  }


  /* get the record with key 0 = 263512477 using B_GET_EQUAL */
  if (status == B_NO_ERROR)
  {
    memset(&personRecord, 0, sizeof(personRecord));
    dataLen = sizeof(personRecord);
    personID = 263512477;    /* this is really a social security number */
    *(BTI_LONG BTI_FAR *)&keyBuf[0] = personID;
    keyNum = 0;

    status = BTRVID(
                B_GET_EQUAL,
                posBlock,
                &personRecord,
                &dataLen,
                keyBuf,
                keyNum,
                (BTI_BUFFER_PTR)&clientID);

    printf("Btrieve B_GET_EQUAL status = %d\n", status);
    if (status == B_NO_ERROR)
    {
      printf("\n");
      printf("The retrieved record is:\n");
#ifdef BTI_MACOSX_64
      printf("ID:      %d\n", personRecord.ID);
#else
      printf("ID:      %ld\n", personRecord.ID);
#endif
      printf("Name:    %s %s\n", personRecord.FirstName,
                                 personRecord.LastName);
      printf("Street:  %s\n", personRecord.Street);
      printf("City:    %s\n", personRecord.City);
      printf("State:   %s\n", personRecord.State);
      printf("Zip:     %s\n", personRecord.Zip);
      printf("Country: %s\n", personRecord.Country);
      printf("Phone:   %s\n", personRecord.Phone);
      printf("\n");
    }
  }

  /* close open files */
  if (fileOpen)
  {
    dataLen = 0;

    status = BTRVID(
                B_CLOSE,
                posBlock,
                dataBuf,
                &dataLen,
                keyBuf,
                keyNum,
                (BTI_BUFFER_PTR)&clientID);

    printf("Btrieve B_CLOSE status (sample.btr) = %d\n", status);
  }


  return(status);
}

