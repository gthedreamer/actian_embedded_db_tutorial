#pragma once

#include <btrapi.h>
#include <btrconst.h>

#define EXIT_WITH_ERROR     1
#define TRUE                1
#define FALSE               0
#define VERSION_OFFSET      0
#define REVISION_OFFSET     2
#define PLATFORM_ID_OFFSET  4
#define MY_THREAD_ID        50
#define FIRST_NAME_LEN      16
#define LAST_NAME_LEN       26
#define PHONE_NUM_LEN       14

#if defined(_MSC_VER) 
#pragma pack(1)
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
Definition of record from 'person.btr'
****************************************************************************/
typedef struct
{
	BTI_LONG  ID;
	BTI_CHAR  FirstName[FIRST_NAME_LEN];
	BTI_CHAR  LastName[LAST_NAME_LEN];
	BTI_CHAR  Phone[PHONE_NUM_LEN];
	BTI_CHAR  Street[31];
	BTI_CHAR  City[31];
	BTI_CHAR  State[3];
	BTI_CHAR  Zip[11];
	BTI_CHAR  Country[21];
} PERSON_STRUCT;

/***************************************************************************
Type definitions for Stat/Create structure
****************************************************************************/
typedef struct
{
	BTI_SINT recLength;
	BTI_SINT pageSize;
	BTI_CHAR indexCount;
	BTI_CHAR version;
	BTI_CHAR reserved[4];
	BTI_SINT flags;
	BTI_BYTE dupPointers;
	BTI_BYTE notUsed;
	BTI_SINT allocations;
} FILE_SPECS;

typedef struct
{
	BTI_SINT position;
	BTI_SINT length;
	BTI_SINT flags;
	BTI_CHAR reserved[4];
	BTI_CHAR type;
	BTI_CHAR null;
	BTI_CHAR notUsed[2];
	BTI_BYTE manualKeyNumber;
	BTI_BYTE acsNumber;
} KEY_SPECS;

typedef struct
{
	FILE_SPECS fileSpecs;
	KEY_SPECS  keySpecs[5];
} FILE_CREATE_BUF;

/***************************************************************************
Structure type definitions for Get Next Extended operation
****************************************************************************/
typedef struct
{
	BTI_SINT    descriptionLen;
	BTI_CHAR    currencyConst[2];
	BTI_SINT    rejectCount;
	BTI_SINT    numberTerms;
} GNE_HEADER;

typedef struct
{
	BTI_CHAR    fieldType;
	BTI_SINT    fieldLen;
	BTI_SINT    fieldOffset;
	BTI_CHAR    comparisonCode;
	BTI_CHAR    connector;
	BTI_CHAR value[3];
} TERM_HEADER;

typedef struct
{
	BTI_SINT    maxRecsToRetrieve;
	BTI_SINT    noFieldsToRetrieve;
} RETRIEVAL_HEADER;

typedef struct
{
	BTI_SINT    fieldLen;
	BTI_SINT    fieldOffset;
} FIELD_RETRIEVAL_HEADER;

typedef struct
{
	GNE_HEADER              gneHeader;
	TERM_HEADER             term1;
	//TERM_HEADER             term2;
	RETRIEVAL_HEADER        retrieval;
	FIELD_RETRIEVAL_HEADER  recordRet;
} PRE_GNE_BUFFER;

typedef struct
{
	BTI_SINT      recLen;
	BTI_LONG      recPos;
	BTI_LONG recID;
} RETURNED_REC;

typedef struct
{
	BTI_SINT      numReturned;
	RETURNED_REC  recs[20];
} POST_GNE_BUFFER;

typedef union
{
	PRE_GNE_BUFFER  preBuf;
	POST_GNE_BUFFER postBuf;
} GNE_BUFFER, BTI_FAR* GNE_BUFFER_PTR;

/* restore structure packing */
#if defined(_MSC_VER) 
#pragma pack()
#endif