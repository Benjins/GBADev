#include "ParserStream.h"

#define MAX_DIALOG_OPTIONS 12

typedef struct{
	const char* option;
	int goToIndex;
} DialogOption;

typedef struct{
	const char* name;
	const char* passage;
	DialogOption options[MAX_DIALOG_OPTIONS];
	int optionCount;
} DialogNode;

typedef struct{
	DialogNode* nodes;
	int nodeCount;
	int startingIndex;
}DialogTree;

void AddDialogNode(DialogTree* tree, DialogNode node){
	DialogNode* newNodes = malloc(sizeof(DialogNode) * (tree->nodeCount+1));
	_memcpy(newNodes, tree->nodes, sizeof(DialogNode) * tree->nodeCount);
	newNodes[tree->nodeCount] = node;
	free(tree->nodes);
	tree->nodes = newNodes;
	tree->nodeCount++;
}

int main(int argc, char** argv){
	const char* fileName = "IFgame/God_IF.html";
	FILE* ifFile = fopen(fileName, "rb");
	if(ifFile == NULL){
		printf("Could not open file: '%s'\n", fileName);
		return 0;
	}
	
	fseek(ifFile, 0, SEEK_END);
	int fileSize = ftell(ifFile);
	fseek(ifFile, 0, SEEK_SET);
	
	char* fileBuffer = (char*)malloc(fileSize+1);
	fread(fileBuffer, 1, fileSize, ifFile);
	fileBuffer[fileSize] = '\0';
	
	fclose(ifFile);
	
	DialogTree tree = {};
	
	ParserStream stream = CreateParserStream(fileBuffer, fileSize);
	AdvanceToString(&stream, "<tw-passagedata");
	while(LengthLeft(&stream) > 0){
		AdvancePast(&stream, "name=\"");
		Token nodeName = AdvanceToString(&stream, "\"");
		
		AdvancePast(&stream, ">");
		Token nodeData = AdvanceToString(&stream, "</tw-passagedata>");
		
		DialogNode node = {};
		node.name = AllocateStringFromToken(nodeName);
		
		ParserStream substream = CreateParserStream(nodeData.start, nodeData.length);
		Token nodePara = AdvanceToString(&substream, "[[");
		node.passage = AllocateStringFromToken(nodePara);
		
		while(LengthLeft(&substream) > 0){
			AdvancePast(&substream, "[[");
			Token option = AdvanceToString(&substream, "]]");
			if(option.length > 0){
				node.options[node.optionCount].option = AllocateStringFromToken(option);
				node.optionCount++;
			}
		}
		
		AddDialogNode(&tree, node);
	}
	
	free(fileBuffer);
	
	for(int i = 0; i < tree.nodeCount; i++){
		for(int j = 0; j < tree.nodes[i].optionCount; j++){
			tree.nodes[i].options[j].goToIndex = -1;
			for(int k = 0; k < tree.nodeCount; k++){
				if(_streq(tree.nodes[i].options[j].option, tree.nodes[k].name)){
					tree.nodes[i].options[j].goToIndex = k;
					break;
				}
			}
		}
	}
	
	for(int i = 0; i < tree.nodeCount; i++){
		printf("Name: '%s'\n", tree.nodes[i].name);
		printf("Passage: '%s'\n", tree.nodes[i].passage);
		for(int j = 0; j < tree.nodes[i].optionCount; j++){
			printf("\t[[%s]] -> %d\n", tree.nodes[i].options[j].option, tree.nodes[i].options[j].goToIndex);
		}
		printf("--------------\n");
	}
	
	return 0;
}