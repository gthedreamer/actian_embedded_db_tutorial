/*************************************************************************
**
**  Copyright 2014 Actian Corporation All Rights Reserved
**
*************************************************************************/
/***************************************************************************
  PersonOperations.cpp
    This is a simple console based Person data Management application helps
	to understand and hands on with Btrieve api

    This program demonstrates the C/C++ interface for Btrieve for Windows

    This program does the following operations on the person database:
	1.CREATE DATA FILE
	2.OPEN DATA FILE
	3.INSERT
	4.DISPLAY ALL
	5.FIND BY ID
	6.FIND BY First Name
	7.DELETE BY ID
	8.UPDATE BY ID
	9.FIND BY Last Name
	10.FIND BY Phone number
	11.DELETE ALL
	12.CLOSE DATA FILE
	0.EXIT AND CLOSE DATA FILE
****************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <windows.h>
#include <btrapitypes.h>
#include <string>
#include <iostream>

using namespace std;

BTI_BYTE posBlock1[128];
BTI_BYTE dataBuf[255];
BTI_WORD dataLen;
BTI_BYTE keyBuf1[255];
BTI_WORD keyNum = 0;

BTI_BYTE btrieveLoaded = FALSE;
BTI_LONG personID;
BTI_BYTE file1Open = FALSE;
BTI_SINT status;
BTI_SINT getStatus = -1;
BTI_SINT i;
BTI_SINT posCtr;

CLIENT_ID       clientID;
VERSION_STRUCT  versionBuffer[3];
FILE_CREATE_BUF fileCreateBuf;
GNE_BUFFER_PTR  gneBuffer;
PERSON_STRUCT   personRecord;

void getRecordFromUser(PERSON_STRUCT *record)
{
	printf("\nID : ");
	scanf("%ld",&(record->ID));
	printf("\nFirstName : ");
	scanf("%s", (record->FirstName));
	printf("\nLastName : ");
	scanf("%s",(record->LastName));
	printf("\nPhone number : ");
	scanf("%s",record->Phone);
	printf("\nStreet : ");
	scanf("%s", record->Street);
	printf("\nCity : ");
	scanf("%s", record->City);
	printf("\nState : ");
	scanf("%s", record->State);
	printf("\nZip : ");
	scanf("%s", record->Zip);
	printf("\nCountry : ");
	scanf("%s", record->Country);
}

void printRecordUtil(PERSON_STRUCT *recordToPrint)
{
	if (recordToPrint != NULL)
	{
		printf("\n***************************************\n");
		printf("ID:      %ld\n", recordToPrint->ID);
		printf("Name:    %s %s\n", recordToPrint->FirstName,
			recordToPrint->LastName);
		printf("Street:  %s\n", recordToPrint->Street);
		printf("City:    %s\n", recordToPrint->City);
		printf("State:   %s\n", recordToPrint->State);
		printf("Zip:     %s\n", recordToPrint->Zip);
		printf("Country: %s\n", recordToPrint->Country);
		printf("Phone:   %s\n", recordToPrint->Phone);
		printf("***************************************\n");
	}
}

/******************************************************************************
Get the data fileName string 
- $ProjectDir\data\person.btr
*******************************************************************************/
void getDataFileName(char* _dataFileName)
{
	char buff[MAX_PATH];
	memset(buff, 0, MAX_PATH);
	GetModuleFileName(NULL, buff, MAX_PATH);
	std::string::size_type pos = string(buff).find_last_of("\\/");
	std::string exePath = string(buff).substr(0, pos);
	std::string dataFileName = exePath + "\\..\\data\\person.btr";
	strcpy(_dataFileName, dataFileName.c_str());
	printf("\nDATA FILE : %s\n", dataFileName.c_str());
}

/******************************************************************************
Creates a data file 
- Before you call any btrieve operation a file must be created
- If the file is already created then it overwrites all 
  the data and makes it empty
*******************************************************************************/
void createDataFile()
{
	/* set up the Client ID */
	memset(clientID.networkAndNode, 0, sizeof(clientID.networkAndNode));
	memcpy(clientID.applicationID, "MT", 2);  /* must be greater than "AA" */
	clientID.threadID = MY_THREAD_ID;

	memset(&fileCreateBuf, 0, sizeof(fileCreateBuf));
	dataLen = sizeof(fileCreateBuf);
	fileCreateBuf.fileSpecs.recLength = sizeof(PERSON_STRUCT);
	fileCreateBuf.fileSpecs.indexCount = 4;
	fileCreateBuf.fileSpecs.pageSize = 16384;
	fileCreateBuf.fileSpecs.version = 0x00;

	//ID index
	fileCreateBuf.keySpecs[0].position = 1;
	fileCreateBuf.keySpecs[0].length = sizeof(BTI_LONG);
	fileCreateBuf.keySpecs[0].flags = EXTTYPE_KEY + MOD;
	fileCreateBuf.keySpecs[0].type = INTEGER_TYPE;

	//First name index
	fileCreateBuf.keySpecs[1].position = sizeof(BTI_LONG)+1;
	fileCreateBuf.keySpecs[1].length = sizeof(personRecord.FirstName);
	fileCreateBuf.keySpecs[1].flags = EXTTYPE_KEY + NOCASE_KEY + DUP + MOD;
	fileCreateBuf.keySpecs[1].type = ZSTRING_TYPE;

	//Last name index
	fileCreateBuf.keySpecs[2].position = sizeof(BTI_LONG) + sizeof(personRecord.FirstName)+1;
	fileCreateBuf.keySpecs[2].length = sizeof(personRecord.LastName);
	fileCreateBuf.keySpecs[2].flags = EXTTYPE_KEY + NOCASE_KEY + DUP + MOD;
	fileCreateBuf.keySpecs[2].type = ZSTRING_TYPE;

	//Phone number index
	fileCreateBuf.keySpecs[3].position = sizeof(BTI_LONG) + sizeof(personRecord.FirstName) + sizeof(personRecord.LastName) + 1;
	fileCreateBuf.keySpecs[3].length = sizeof(personRecord.Phone);
	fileCreateBuf.keySpecs[3].flags = EXTTYPE_KEY + NOCASE_KEY + MOD;
	fileCreateBuf.keySpecs[3].type = ZSTRING_TYPE;

	char dataFileNameWithPath[MAX_PATH];
	getDataFileName(dataFileNameWithPath);
	strcpy((BTI_CHAR *)keyBuf1, dataFileNameWithPath);
	keyNum = 0;

	status = BTRVID(B_CREATE,
		posBlock1,
		&fileCreateBuf,
		&dataLen,
		keyBuf1,
		keyNum,
		(BTI_BUFFER_PTR)&clientID);

	printf("Btrieve B_CREATE status = %d\n", status);
}

/******************************************************************************
Opens a data file for other data operations
- Make sure the data file has been created before opening the file
- File should be opened before any insert/delete/update operation
*******************************************************************************/
void openDataFile()
{
	char dataFileNameWithPath[MAX_PATH];
	getDataFileName(dataFileNameWithPath);
	strcpy((BTI_CHAR *)keyBuf1, dataFileNameWithPath);

	keyNum = 0;
	dataLen = 0;

	status = BTRVID(
		B_OPEN,
		posBlock1,
		dataBuf,
		&dataLen,
		keyBuf1,
		keyNum,
		(BTI_BUFFER_PTR)&clientID);

	printf("Btrieve B_OPEN status (person.btr) = %d\n", status);
	if (status == B_NO_ERROR)
	{
		file1Open = TRUE;
	}
}

/******************************************************************************
Close a data file
- Data file must be closed before exit for proper releasing of lock 
*******************************************************************************/
void closeDataFile()
{
	/* close open files */
	if (file1Open)
	{
		dataLen = 0;

		status = BTRVID(
			B_CLOSE,
			posBlock1,
			dataBuf,
			&dataLen,
			keyBuf1,
			keyNum,
			(BTI_BUFFER_PTR)&clientID);

		printf("Btrieve B_CLOSE status (sample.btr) = %d\n", status);
		file1Open = FALSE;
	}
}

/******************************************************************************
Insert a Person record into the data file based on posBlock1 position
*******************************************************************************/
void insert(PERSON_STRUCT *personData)
{
	dataLen = sizeof(PERSON_STRUCT);
	memcpy(dataBuf, personData, dataLen);

	status = BTRVID(
		B_INSERT,
		posBlock1,
		dataBuf,
		&dataLen,
		keyBuf1,
		-1,   /* no currency change */
		(BTI_BUFFER_PTR)&clientID);

	printf("Btrieve B_INSERT = %d\n", status);
}

/******************************************************************************
Get the first record from ID index
*******************************************************************************/
bool getFirst(bool disp)
{
	keyNum = 0; /* ID index */
	memset(&personRecord, 0, sizeof(personRecord));
	dataLen = sizeof(personRecord);

	getStatus = BTRVID(
		B_GET_FIRST,
		posBlock1,
		&personRecord,
		&dataLen,
		keyBuf1,
		keyNum,
		(BTI_BUFFER_PTR)&clientID);

	printf("Btrieve B_GET_FIRST status (sample.btr) = %d\n\n", getStatus);
	if (getStatus != B_NO_ERROR)
		return false;
	if(disp)
		printRecordUtil(&personRecord);

	return true;
}

/******************************************************************************
Get the next record for the given KeyNumber(index)
*******************************************************************************/
bool getNextUtil(int keyNumber,bool disp)
{
	memset(&personRecord, 0, sizeof(personRecord));
	dataLen = sizeof(personRecord);

	getStatus = BTRVID(
		B_GET_NEXT,
		posBlock1,
		&personRecord,
		&dataLen,
		keyBuf1, // Must have filled by engine in prev call
		keyNumber, // Index 
		(BTI_BUFFER_PTR)&clientID);

	printf("Btrieve B_GET_NEXT status = %d\n\n", getStatus);
	if (getStatus != B_NO_ERROR)
		return false;

	if (disp)
	{
		printRecordUtil(&personRecord);
	}

	return true;
}

/******************************************************************************
Get the first record which matches the data in keyBuffer for the given key(index)
- keyBuffer contains the data to compare for ex: Id in case of Id based search
*******************************************************************************/
bool getEqualUtil(BTI_BYTE *keyBuffer, BTI_WORD keyNumber, bool disp)
{
	memset(&personRecord, 0, sizeof(personRecord));
	dataLen = sizeof(personRecord);

	status = BTRVID(
		B_GET_EQUAL,
		posBlock1,
		&personRecord,
		&dataLen,
		keyBuffer,
		keyNumber,
		(BTI_BUFFER_PTR)&clientID);

	printf("Btrieve B_GET_EQUAL status = %d\n", status);

	if (status == B_NO_ERROR)
	{
		if(disp)
			printRecordUtil(&personRecord);
		return true;
	}

	return false;
}

/******************************************************************************
Get all records which maches the firstNameToSearch
- Search is case insensitive
- Uses firstName index
*******************************************************************************/
void findByFirstName(BTI_CHAR *firstNameToSearch)
{
	keyNum = 1; //First name index
	memset(&keyBuf1, 0, sizeof(keyBuf1));
	strcpy((BTI_CHAR *)keyBuf1, firstNameToSearch);
	
	bool readStatus = getEqualUtil(keyBuf1,keyNum,false);
	
	while (readStatus && strcmpi(firstNameToSearch, personRecord.FirstName) == 0)
	{
		printRecordUtil(&personRecord);
		readStatus = getNextUtil(keyNum,false);
	};
}

/******************************************************************************
Get all records which maches the lastNameToSearch
- Search is case insensitive
- Uses lastName index
*******************************************************************************/
void findByLastName(BTI_CHAR *lastNameToSearch)
{
	keyNum = 2; //Last name index
	memset(&keyBuf1, 0, sizeof(keyBuf1));
	strcpy((BTI_CHAR *)keyBuf1, lastNameToSearch);

	bool readStatus = getEqualUtil(keyBuf1, keyNum, false);

	while (readStatus && strcmpi(lastNameToSearch, personRecord.LastName) == 0)
	{
		printRecordUtil(&personRecord);
		readStatus = getNextUtil(keyNum, false);
	};
}

/******************************************************************************
Get the record which matches idToFind
- result is set to global variable personRecord
- since ID is unique for Person we get only one record
*******************************************************************************/
bool findByID(BTI_LONG idToFind,bool disp)
{
	memset(&keyBuf1, 0, sizeof(keyBuf1));
	*(BTI_LONG BTI_FAR *)&keyBuf1[0] = idToFind;
	keyNum = 0;

	return getEqualUtil(keyBuf1,keyNum,true);
}

/******************************************************************************
Get record which matches phoneNoToSearch
- Uses phoneNumber index
- result is set to global variable personRecord
- since ID is unique for Person we get only one record
*******************************************************************************/
bool findByPhoneNumber(BTI_CHAR_PTR phoneNoToSearch)
{
	memset(&keyBuf1, 0, sizeof(keyBuf1));
	strcpy((BTI_CHAR *)keyBuf1, phoneNoToSearch);
	keyNum = 3;//Phone number index

	return getEqualUtil(keyBuf1, keyNum, true);
}

/******************************************************************************
Displays all record in the file
- Uses ID index
*******************************************************************************/
void showAll()
{
	bool returnStatus = getFirst(true);

	while (returnStatus)
	{
		returnStatus = getNextUtil(0, true);
	}
}

void insertMultipleRecords(int count)
{
	PERSON_STRUCT person;
	for (int i = 0; i < count; i++)
	{
		memset(&person, 0, sizeof(person));
		getRecordFromUser(&person);
		insert(&person);
	}
}


/******************************************************************************
Deletes the record from file which matches idToDelete
- Uses ID index to find the record
*******************************************************************************/
void deleteByID(BTI_LONG idToDelete)
{
	if (findByID(idToDelete,false)) // This sets the position block in correct position
	{
		status = BTRVID(
			B_DELETE,
			posBlock1,
			dataBuf,
			&dataLen,
			keyBuf1,
			keyNum,
			(BTI_BUFFER_PTR)&clientID);

		printf("Btrieve B_DELETE status = %d\n", status);

		if (status == B_NO_ERROR)
		{
			printf("Record with id %ld deleted successfully\n", idToDelete);
		}
	}
}


/******************************************************************************
Updates the record which matches idToUpdate
- Uses ID index
*******************************************************************************/
void updateByID(BTI_LONG idToUpdate)
{
	if (findByID(idToUpdate, false)) // This sets the position block in correct position
	{
		memset(&personRecord,0,sizeof(personRecord));
		dataLen = sizeof(PERSON_STRUCT);
		
		printf("\nEnter new data to update\n");
		getRecordFromUser(&personRecord);
		memcpy(dataBuf, &personRecord, dataLen);
		keyNum = 0;// ID index

		status = BTRVID(
			B_UPDATE,
			posBlock1,
			dataBuf,
			&dataLen,
			keyBuf1,
			keyNum,
			(BTI_BUFFER_PTR)&clientID);

		printf("Btrieve B_UPDATE status = %d\n", status);

		if (status == B_NO_ERROR)
		{
			printf("Record with id %ld updated successfully\n", idToUpdate);
		}
	}
}

/******************************************************************************
Delete all the records in the file by iterating over ID index.It makes
the file empty
- Uses ID index
*******************************************************************************/
void deleteAll()
{
	bool returnStatus = getFirst(false);

	while (returnStatus)
	{
		deleteByID(personRecord.ID);
		returnStatus = getNextUtil(0, false);
	}
}

void handleMenuSelected(int option)
{
	int noOfRecordsToInsert = 0;
	BTI_LONG id;
	BTI_CHAR firstNameToSearch[FIRST_NAME_LEN];
	BTI_CHAR lastNameToSearch[LAST_NAME_LEN];
	BTI_CHAR phoneNumToSearch[PHONE_NUM_LEN];

	//Validate data file is opened
	if (!file1Open && option != 2 && option != 1)
	{
		printf("\n\n\t\t !!!File is NOT OPENED yet;Pleasee select option 2 from Menu\t\t !!!\n\n");
		return;
	}

	switch (option)
	{
	case 0:
		printf("Closing data file.....\n");
		closeDataFile();
		break;
	case 1:
		createDataFile();
		break;
	case 2:
		openDataFile();
		break;
	case 3:
		printf("\nEnter number of records you want to insert : ");
		scanf("%d", &noOfRecordsToInsert);
		insertMultipleRecords(noOfRecordsToInsert);
		break;
	case 4:
		showAll();
		break;
	case 5:
		printf("\nEnter ID of the person to find : ");
		scanf("%ld", &id);
		findByID(id,true);
		break;
	case 6:
		printf("\nEnter First name of the person to find : ");
		scanf("%s", firstNameToSearch);
		findByFirstName(firstNameToSearch);
		break;
	case 7:
		printf("\nEnter ID of the person to delete : ");
		scanf("%ld", &id);
		deleteByID(id);
		break;
	case 8:
		printf("\nEnter ID of the person to update : ");
		scanf("%ld", &id);
		updateByID(id);
		break;
	case 9:
		printf("\nEnter Last name of the person to find : ");
		scanf("%s", lastNameToSearch);
		findByLastName(lastNameToSearch);
		break;
	case 10:
		printf("\nEnter phone number of the person to find : ");
		scanf("%s", phoneNumToSearch);
		findByPhoneNumber(phoneNumToSearch);
		break;
	case 11:
		deleteAll();
		break;
	case 12:
		closeDataFile();
		break;
	default:
		printf("Invalid option please retry\n");
		break;
	}
}

/***************************************************************************
  Main
****************************************************************************/
int main(void)
{
	/* Btrieve function parameters */
	int noOfRecs = 0;
	int menuOptionSelected = 0;
	char dataFileName[MAX_PATH];
	getDataFileName(dataFileName);

	printf("****************  Person data management using Btrieve C/C++ Interface Demo ****************\n\n");

	do
	{
		  printf("\n*************   Person data operations  ****************\n");
		  printf("1.CREATE DATA FILE\n");
		  printf("2.OPEN DATA FILE\n");
		  printf("3.INSERT\n");
		  printf("4.DISPLAY ALL\n");
		  printf("5.FIND BY ID\n");
		  printf("6.FIND BY First Name\n");
		  printf("7.DELETE BY ID\n");
		  printf("8.UPDATE BY ID\n");
		  printf("9.FIND BY Last Name\n");
		  printf("10.FIND BY Phone number\n");
		  printf("11.DELETE ALL\n");
		  printf("12.CLOSE DATA FILE\n");
		  printf("0.EXIT AND CLOSE DATA FILE\n");

		  printf("\nEnter any of the menu option number : ");
		  scanf("%d", &menuOptionSelected);
		  handleMenuSelected(menuOptionSelected);
	} while (menuOptionSelected != 0);

	char ch;
	scanf("%c", &ch);

	return(0);
}

