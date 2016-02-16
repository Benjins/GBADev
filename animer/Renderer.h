#ifndef RENDERER_H
#define RENDERER_H

#pragma once

struct BitmapData{
	int* data;
	int width;
	int height;
};

inline int RGBA(int r, int g, int b, int a) {
	return (r << 24) | (g << 16) | (b << 8) | (a);
}

void DrawBox(BitmapData bitmap, int x, int y, int w, int h, int col);

void DrawBitmap(BitmapData bitmap, int x, int y, int w, int h, BitmapData sprite);

//If the text goes outside the box, it'll be truncated
void DrawText(BitmapData bitmap, char* text, int x, int y, int width, int height);

void Render(BitmapData frameBuffer);

void InitText(char* fileName, int size);

bool Button(BitmapData frameBuffer, int x, int y, int w, int h, int offCol, int hoverCol, int pressCol, char* label);

BitmapData LoadBMPFile(char* fileName);

void WriteBMPFile(char* fileName, BitmapData* data);

#endif