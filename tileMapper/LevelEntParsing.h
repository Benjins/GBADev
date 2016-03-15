#ifndef LEVEL_ENT_PARSING_H
#define LEVEL_ENT_PARSING_H

#include "../toolsCode/ParserStream.h"
#include "../metaGenTool/metaParse.h"

enum FieldType {
	MetaInt,
	MetaFloat,
	MetaString,
	MetaOther
};

typedef struct{
	Token name;
	FieldType type;
	union {
		int intVal;
		float floatVal;
		char* strVal;
	};
} FieldValue;

DEFINE_VECTOR(FieldValue)

typedef struct {
	int position[2];
	FieldValueVector otherVals;
} GameEntityInstance;

DEFINE_VECTOR(GameEntityInstance)

typedef struct {
	Token name;
	BitmapData icon;
	GameEntityInstanceVector instances;
	int maxCount;
} LevelEntityInstances;

DEFINE_VECTOR(LevelEntityInstances)

/*
monsters:
    [{x:2,y:5}, {x:10,y:64}]
objects:
    [{x:17,y:29,whatSay:"Who am i"}, {x:55,y:123,whatSay:"This is maddd"}]
*/

LevelEntityInstancesVector ReadLevelEntsFromFile(const char* fileName, StructDefVector structDefs, StructDef* levelStruct) {

	int fileLength = 0;
	char* fileBuffer = ReadTextFile(fileName, &fileLength);

	static const char* whitespace = "\n\t\r ";
	static const char* colonAndWhitespace = "\n\t\r :";
	static const char* commaAndWhitespace = "\n\t\r ,";

	static const Token xTok = MAKE_TOKEN(x);
	static const Token yTok = MAKE_TOKEN(y);

	LevelEntityInstancesVector instances = { 0 };

	ParserStream stream = CreateParserStream(fileBuffer, fileLength);

	//TODO: Rename stream to fileStream.
	//This comment probably took more energy than to just do that.
	while (LengthLeft(&stream) > 0) {
		AdvanceUntilNotOneOf(&stream, whitespace);
		Token entFieldName = AdvanceUntilOneOf(&stream, colonAndWhitespace);

		Token entTypeName = { 0 };
		int maxCount = 0;
		for (int i = 0; i < levelStruct->fields.length; i++) {
			if (TokenEqual(levelStruct->fields.vals[i].fieldName, entFieldName)) {
				entTypeName = levelStruct->fields.vals[i].typeName;
				maxCount = levelStruct->fields.vals[i].arrayCount;
				break;
			}
		}

		StructDef* gameEntStruct = NULL;
		for (int i = 0; i < structDefs.length; i++) {
			if (TokenEqual(structDefs.vals[i].name, entTypeName)) {
				gameEntStruct = &structDefs.vals[i];
				break;
			}
		}

		if (gameEntStruct == NULL) {
			printf("\nError in parsing.\n");
			return instances;
		}

		LevelEntityInstances instanceList = { 0 };
		instanceList.name = gameEntStruct->name;
		instanceList.maxCount = maxCount;

		if (*stream.cursor == ':') {
			stream.cursor++;
		}

		AdvancePast(&stream, "[");
		Token objects = AdvanceUntilOneOf(&stream, "]");
		ParserStream objectListStream = CreateParserStream((char*)objects.start, objects.length);

		while (LengthLeft(&objectListStream) > 0) {
			AdvancePast(&objectListStream, "{");
			Token singleObject = AdvanceToString(&objectListStream, "}");
			ParserStream singleObjectStream = CreateParserStream((char*)singleObject.start, singleObject.length);
			
			GameEntityInstance gameEntInst = { 0 };
			
			while (LengthLeft(&singleObjectStream) > 0) {
				AdvanceUntilNotOneOf(&singleObjectStream, whitespace);
				Token fieldName = AdvanceUntilOneOf(&singleObjectStream, colonAndWhitespace);

				AdvanceUntilNotOneOf(&singleObjectStream, colonAndWhitespace);

				Token fieldValue = { 0 };
				
				if (*singleObjectStream.cursor == '"') {
					singleObjectStream.cursor++;
					fieldValue = AdvanceToString(&singleObjectStream, "\"");
					singleObjectStream.cursor++;
				}
				else {
					fieldValue = AdvanceUntilOneOf(&singleObjectStream, commaAndWhitespace);
				}

				char intBuffer[256] = { 0 };
				_memcpy(intBuffer, fieldValue.start, fieldValue.length);
				intBuffer[fieldValue.length] = '\0';
				
				if (TokenEqual(fieldName, xTok)) {
					gameEntInst.position[0] = _atoi(intBuffer);
				}
				else if (TokenEqual(fieldName, yTok)) {
					gameEntInst.position[1] = _atoi(intBuffer);
				}
				else {
					FieldValue fieldVal = { 0 };
					fieldVal.type = MetaOther;
					fieldVal.name = fieldName;

					int fieldIndex = -1;
					for (int i = 0; i < gameEntStruct->fields.length; i++) {
						if (TokenEqual(gameEntStruct->fields.vals[i].fieldName, fieldName)) {
							fieldIndex = i;
							break;
						}
					}

					if (fieldIndex >= 0) {
						FieldDef fieldType = gameEntStruct->fields.vals[fieldIndex];

						if (TOKEN_IS(fieldType.typeName, "char") && fieldType.pointerLevel == 1) {
							fieldVal.type = MetaString;
							fieldVal.strVal = (char*)malloc(fieldValue.length + 1);
							_memcpy(fieldVal.strVal, fieldValue.start, fieldValue.length);
							fieldVal.strVal[fieldValue.length] = '\0';
						}
						else if (TOKEN_IS(fieldType.typeName, "int") || TOKEN_IS(fieldType.typeName, "int32") || TOKEN_IS(fieldType.typeName, "int16")) {
							fieldVal.type = MetaInt;
						}

						if (fieldVal.type != MetaOther) {
							VectorAddFieldValue(&gameEntInst.otherVals, fieldVal);
							fieldVal.intVal = _atoi(intBuffer);
						}
					}
					
				}

				AdvancePast(&singleObjectStream, ",");
			}

			AdvanceToString(&objectListStream, "{");

			VectorAddGameEntityInstance(&instanceList.instances, gameEntInst);
		}

		AdvancePast(&stream, "]");

		VectorAddLevelEntityInstances(&instances, instanceList);
	}

	return instances;
}

void WriteLevelEntsToFile(LevelEntityInstancesVector data, const char* fileName) {
	FILE* entFile = fopen(fileName, "wb");

	if (entFile == NULL) {
		printf("\nError: Could not open file '%s' for writing.\n", fileName);
		return;
	}

	for (int i = 0; i < data.length; i++) {
		fprintf(entFile, "%.*s:\n", data.vals[i].name.length, data.vals[i].name.start);
		fprintf(entFile, "\t[", data.vals[i].name);

		for (int j = 0; j < data.vals[i].instances.length; j++) {
			fprintf(entFile, "%s {", (j != 0 ? "," : ""));
			GameEntityInstance inst = data.vals[i].instances.vals[j];
			fprintf(entFile, "x: %d, y: %d", inst.position[0], inst.position[1]);
			for (int k = 0; k < inst.otherVals.length; k++) {
				FieldValue fieldVal = inst.otherVals.vals[k];
				if (fieldVal.type == MetaInt) {
					fprintf(entFile, ", %.*s: %d", fieldVal.name.length, fieldVal.name.start, fieldVal.intVal);
				}
				else if (fieldVal.type == MetaFloat) {
					fprintf(entFile, ", %.*s: %f", fieldVal.name.length, fieldVal.name.start, fieldVal.floatVal);
				}
				else if (fieldVal.type == MetaString) {
					fprintf(entFile, ", %.*s: %s", fieldVal.name.length, fieldVal.name.start, fieldVal.strVal);
				}
			}
			fprintf(entFile, " }");
		}

		fprintf(entFile, "]\n", data.vals[i].name);
	}
}

#endif