#ifndef METAPARSE_H
#define METAPARSE_H

#include "token.h"


typedef enum {
	INVALID,
	TYPEDEF_ANON,
	TYPEDEF_NAMED,
	CPP_STYLE
} StructDefineType;

typedef struct {
	Token attrName;
	TokenVector attrParams;
} AttributeUse;

DEFINE_VECTOR(AttributeUse)

#define NOT_AN_ARRAY -1

typedef struct {
	AttributeUseVector attrs;
	Token typeName;
	Token fieldName;
	int pointerLevel;
	int arrayCount;
} FieldDef;

DEFINE_VECTOR(FieldDef)

typedef struct {
	AttributeUseVector attrs;
	Token name;
	Token parentType;
	FieldDefVector fields;
}StructDef;

DEFINE_VECTOR(StructDef)

typedef struct {
	AttributeUseVector attrs;
	Token enumName;
} EnumEntry;

DEFINE_VECTOR(EnumEntry)

typedef struct {
	AttributeUseVector attrs;
	EnumEntryVector entries;
	Token name;
} EnumDef;

DEFINE_VECTOR(EnumDef)

#endif