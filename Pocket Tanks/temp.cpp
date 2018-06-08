#include "temp.h"

void UpdateRecords(Player players[])
{
	RecordRow records[NUMBER_OF_RECORD_ROWS];
	LoadRecords(records);
	int recordsIndex;

	for (int i = 0; i < 2; i++)
	{
		recordsIndex = -1;

		for (int j = 0; j < NUMBER_OF_RECORD_ROWS; j++)
			if (players[i].score >= records[j].score)
			{
				recordsIndex = j;
				break;
			}

		if (recordsIndex > -1)
		{
			for (int j = NUMBER_OF_RECORD_ROWS - 2; j >= recordsIndex; j--)
				records[j + 1] = records[j];

			strcpy_s(records[recordsIndex].name, NAME_LENGTH, players[i].name);
			records[recordsIndex].score = players[i].score;
		}
	}

	//for (int i = 0; i < NUMBER_OF_RECORD_ROWS; i++)
	//	if (players[0].score >= records[i].score)
	//	{
	//		recordsIndex = i;
	//		break;
	//	}

	//if (recordsIndex > -1)
	//{
	//	for (int i = recordsIndex; i < NUMBER_OF_RECORD_ROWS - 1; i++)
	//		records[i + 1] = records[i];

	//	records[recordsIndex].name = players[0].name;
	//	records[recordsIndex].score = players[0].score;
	//}

	//recordsIndex = -1;

	//for (int i = 0; i < NUMBER_OF_RECORD_ROWS; i++)
	//	if (players[1].score >= records[i].score)
	//	{
	//		recordsIndex = i;
	//		break;
	//	}

	//if (recordsIndex > -1)
	//{
	//	for (int i = recordsIndex; i < NUMBER_OF_RECORD_ROWS - 1; i++)
	//		records[i + 1] = records[i];

	//	records[recordsIndex].name = players[1].name;
	//	records[recordsIndex].score = players[1].score;
	//}

	FILE *recordsFile = NULL;
	fopen_s(&recordsFile, "records.pt", "wb");
	fwrite(records, sizeof(RecordRow), NUMBER_OF_RECORD_ROWS, recordsFile);
	fclose(recordsFile);
	recordsFile = NULL;
}